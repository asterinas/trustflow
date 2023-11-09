// Copyright 2023 Ant Group Co., Ltd.
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//   http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.


use std::{io::Write, path::Path, sync::Arc};

use crate::error::Error;
use crate::{client::data_mesh_client::DataMeshClient, constants, service::DmOutputUriQueryParams};
use futures::TryStreamExt;
use hyper::Body;
use multipart_async::server::Multipart;
use std::fs;

pub(crate) async fn handle_upload(
    mut multipart: Multipart<Body>,
    dm_client: Arc<DataMeshClient>,
) -> Result<(), Error> {
    log::info!("Receiving request");
    // parse request
    let mut store_path = None;
    let mut file_temp_path = None;
    let mut domain_data = None;
    while let Some(mut field) = multipart.next_field().await? {
        let field_name = field.headers.name;

        match field_name.as_str() {
            "store_path" => {
                let ret = field.data.read_to_string().await?;
                store_path = Some(ret);
            }
            "file" => {
                let dir = Path::new(constants::TEMP_DIR);
                fs::create_dir_all(&dir)?;
                let path = dir.join(field.headers.filename.ok_or(Error::from("lack filename"))?);
                let mut file = match path.exists() {
                    true => fs::OpenOptions::new()
                        .write(true)
                        .truncate(true)
                        .open(&path)?,
                    false => fs::OpenOptions::new()
                        .create_new(true)
                        .write(true)
                        .append(true)
                        .open(&path)?,
                };

                while let Some(chunk) = field.data.try_next().await? {
                    file.write_all(&chunk)?;
                }
                file_temp_path = Some(path);
            }
            "domain_data" => {
                use kuscia_proto::api::datamesh::DomainData;
                let ret = field.data.read_to_string().await?;
                domain_data = Some(serde_json::from_slice::<DomainData>(ret.as_bytes())?);
            }
            // ignore other field
            _ => {}
        }
    }

    // Register to data mesh
    let store_path = store_path.ok_or(Error::from("lack store_path"))?;
    let file_temp_path = file_temp_path.ok_or(Error::from("lack upload"))?;
    let domain_data = domain_data.ok_or(Error::from("locak domain data"))?;

    log::info!("store path: {}", store_path);
    log::info!("file_temp_path: {:?}", file_temp_path);

    log::info!("Register to datamesh");
    let url = url::Url::parse(&store_path)?;
    println!("{store_path}");
    match url.scheme() {
        "dm" => {
            let qs = url.query().ok_or("lack query")?;
            let params: DmOutputUriQueryParams = serde_qs::from_str(qs)?;
            let datasource = dm_client
                .get_domain_datasource(&params.datasource_id)
                .await?;
            let info = datasource.info.ok_or("lack datasource info")?;
            let info_localfs = info.localfs.ok_or("lack localfs config")?;
            let dest_path = Path::new(&info_localfs.path).join(&params.uri);

            let parent = dest_path.parent().ok_or(Error::from("not in dir"))?;
            if !parent.exists() {
                fs::create_dir_all(parent)?;
            }

            let file_temp_size = std::fs::metadata(&file_temp_path)?.len();
            log::info!("Length of {:?} is {}", file_temp_path, file_temp_size);
            log::info!("Coping file from {:?} to {:?}", file_temp_path, dest_path);
            // move file to data source file system
            fs::copy(file_temp_path, &dest_path)?;
            log::info!("Copy file successfully");
            let file_dest_size = std::fs::metadata(&dest_path)?.len();
            log::info!("Length of {:?} is {}", dest_path, file_dest_size);

            // regiseter to data mesh
            dm_client
                .create_domain_data(&params.id, &params.uri, &params.datasource_id, &domain_data)
                .await?;
        }
        _ => return Err(Error::from("Unsupport scheme")),
    }

    Ok(())
}
