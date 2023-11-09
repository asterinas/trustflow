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
use std::sync::Arc;

use crate::{client::data_mesh_client::DataMeshClient, service::DmInputUriQueryParams};

#[derive(Deserialize, Serialize)]
struct RequestParams {
    pub uri: String,
}

pub(crate) async fn get_domain_data(
    req: Request<Body>,
    dm_client: Arc<DataMeshClient>,
) -> Result<Response<Body>, Error> {
    let body_bytes = hyper::body::to_bytes(req).await?;
    let request_params: RequestParams = serde_json::from_slice(&body_bytes)?;
    let url = url::Url::parse(&request_params.uri)?;
    println!("{url}");
    match url.scheme() {
        "dm" => {
            let qs = url.query().ok_or("lack query")?;
            let params: DmInputUriQueryParams = serde_qs::from_str(qs)?;
            let domain_data = dm_client.get_domain_data(&params.id).await?;
            let domain_data_json = serde_json::to_vec(&domain_data)?;
            return Ok(Response::new(Body::from(domain_data_json)));
        }
        _ => return Err(Error::from("Unsupport scheme")),
    }
}
