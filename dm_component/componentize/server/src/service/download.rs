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


use crate::error::Error;
use hyper::{Body, Request, Response};
use serde::{Deserialize, Serialize};
use std::{path::Path, sync::Arc};
use tokio::fs::File;
use tokio_util::codec::{BytesCodec, FramedRead};

use crate::{
    client::data_mesh_client::DataMeshClient,
    service::{not_found, DmInputUriQueryParams},
};

#[derive(Deserialize, Serialize)]
struct RequestParams {
    pub uri: String,
}

pub(crate) async fn handle_download(
    req: Request<Body>,
    dm_client: Arc<DataMeshClient>,
) -> Result<Response<Body>, Error> {
    log::info!("download");
    let body_bytes = hyper::body::to_bytes(req).await?;
    let request_params: RequestParams = serde_json::from_slice(&body_bytes)?;
    let url = url::Url::parse(&request_params.uri)?;
    println!("{url}");
    match url.scheme() {
        "dm" => {
            let qs = url.query().ok_or("lack query")?;
            let params: DmInputUriQueryParams = serde_qs::from_str(qs)?;
            let domain_data = dm_client.get_domain_data(&params.id).await?;
            let relative_uri = domain_data.relative_uri;

            let domain_data_source = dm_client
                .get_domain_datasource(&domain_data.datasource_id)
                .await?;
            let info = domain_data_source.info.ok_or("lack datasource info")?;
            let info_localfs = info.localfs.ok_or("lack localfs config")?;

            let filename = Path::new(&info_localfs.path).join(relative_uri);
            if let Ok(file) = File::open(filename).await {
                let stream = FramedRead::new(file, BytesCodec::new());
                let body = Body::wrap_stream(stream);
                return Ok(Response::new(body));
            }
        }
        "file" => {
            if let Ok(file) = File::open(url.path()).await {
                let stream = FramedRead::new(file, BytesCodec::new());
                let body = Body::wrap_stream(stream);
                return Ok(Response::new(body));
            }
        }
        _ => return Err(Error::from("Unsupport scheme")),
    }

    Ok(not_found())
}
