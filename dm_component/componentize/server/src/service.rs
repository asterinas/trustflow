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


mod download;
mod get_domain_data;
mod upload;

use hyper::{Body, Method, Request, Response, StatusCode};
use serde::{Deserialize, Serialize};
use std::sync::{Arc, Mutex};

use lazy_static::lazy_static;
use multipart_async::server::Multipart;
use tokio::sync::oneshot::Sender;

use crate::client::data_mesh_client::DataMeshClient;

use crate::error::Error;

static NOTFOUND: &[u8] = b"Not Found";

lazy_static! {
    pub static ref SHUTDOWN_TX: Arc<Mutex<Option<Sender<()>>>> = <_>::default();
}

/// HTTP status code 404
fn not_found() -> Response<Body> {
    Response::builder()
        .status(StatusCode::NOT_FOUND)
        .body(Body::from(NOTFOUND))
        .unwrap()
}

async fn shutdown(_: Request<Body>) -> Result<Response<Body>, Error> {
    // Attempt to send a shutdown signal, if one hasn't already been sent
    if let Ok(mut lock) = SHUTDOWN_TX.lock() {
        if let Some(tx) = lock.take() {
            let _ = tx.send(());
            return Ok(Response::new(Body::from("start to shut down")));
        }
    }
    Err("shutdown failed.".into())
}

// the datamesh input uri format is defined as follows:
//      dm://input?id=alice-table
// this struct is used to parse the query for the input uri
#[derive(Debug, PartialEq, Deserialize, Serialize)]
struct DmInputUriQueryParams {
    // domain data id
    pub id: String,
}

// the datamesh output uri format is defined as follows:
//      dm://output?datasource_id=default-data-source&id=alice-table&uri=alice-table.encrypted
// this struct is used to parse the query for the output uri
#[derive(Debug, PartialEq, Deserialize, Serialize)]
struct DmOutputUriQueryParams {
    // data source id
    pub datasource_id: String,

    // domain data id
    pub id: String,

    // relative file path
    pub uri: String,
}

use crate::service::download::handle_download;
use crate::service::get_domain_data::get_domain_data;
use crate::service::upload::handle_upload;

pub async fn serve_req(
    req: Request<Body>,
    dm_client: Arc<DataMeshClient>,
) -> Result<Response<Body>, Error> {
    match (req.method(), req.uri().path()) {
        (&Method::POST, "/upload") => Ok(match Multipart::try_from_request(req) {
            Ok(multipart) => match handle_upload(multipart, dm_client).await {
                Ok(()) => Response::new(Body::from("successful request!")),
                Err(e) => {
                    log::info!("upload error: {:?}", e);
                    Response::builder()
                        .status(StatusCode::BAD_REQUEST)
                        .body(Body::from(e.to_string()))?
                }
            },
            Err(req) => Response::new(Body::from("expecting multipart/form-data")),
        }),
        (&Method::POST, "/download") => Ok(match handle_download(req, dm_client).await {
            Ok(r) => r,
            Err(e) => {
                log::info!("downlaod error: {:?}", e);
                Response::builder()
                    .status(StatusCode::BAD_REQUEST)
                    .body(Body::from(e.to_string()))?
            }
        }),
        (&Method::POST, "/get_domain_data") => Ok(match get_domain_data(req, dm_client).await {
            Ok(r) => r,
            Err(e) => {
                log::info!("get domain data error: {:?}", e);
                Response::builder()
                    .status(StatusCode::BAD_REQUEST)
                    .body(Body::from(e.to_string()))?
            }
        }),
        (&Method::POST, "/shutdown") => Ok(match shutdown(req).await {
            Ok(r) => r,
            Err(e) => {
                log::info!("shutdown server");
                Response::builder()
                    .status(StatusCode::BAD_REQUEST)
                    .body(Body::from(e.to_string()))?
            }
        }),
        _ => Ok(not_found()),
    }
}

#[cfg(test)]
mod tests {
    use kuscia_proto::api::datamesh::DomainData;
    use kuscia_proto::api::datamesh::{
        QueryDomainDataSourceRequest, QueryDomainDataSourceResponse,
    };

    #[test]
    fn test_deserialization() {
        let json_string: &str  = "{\n  \"domaindata_id\": \"alice-table\",\n  \"name\": \"alice.csv\",\n  \"type\": \"table\",\n  \"relative_uri\": \"alice.csv\",\n  \"datasource_id\": \"default-data-source\",\n  \"attributes\": {\n    \"description\": \"alice demo data\"\n  },\n  \"columns\": [\n    {\n      \"name\": \"id1\",\n      \"type\": \"str\"\n    },\n    {\n      \"name\": \"age\",\n      \"type\": \"float\"\n    },\n    {\n      \"name\": \"education\",\n      \"type\": \"float\"\n    },\n    {\n      \"name\": \"default\",\n      \"type\": \"float\"\n    },\n    {\n      \"name\": \"balance\",\n      \"type\": \"float\"\n    },\n    {\n      \"name\": \"housing\",\n      \"type\": \"float\"\n    },\n    {\n      \"name\": \"loan\",\n      \"type\": \"float\"\n    },\n    {\n      \"name\": \"day\",\n      \"type\": \"float\"\n    },\n    {\n      \"name\": \"duration\",\n      \"type\": \"float\"\n    },\n    {\n      \"name\": \"campaign\",\n      \"type\": \"float\"\n    },\n    {\n      \"name\": \"pdays\",\n      \"type\": \"float\"\n    },\n    {\n      \"name\": \"previous\",\n      \"type\": \"float\"\n    },\n    {\n      \"name\": \"job_blue-collar\",\n      \"type\": \"float\"\n    },\n    {\n      \"name\": \"job_entrepreneur\",\n      \"type\": \"float\"\n    },\n    {\n      \"name\": \"job_housemaid\",\n      \"type\": \"float\"\n    },\n    {\n      \"name\": \"job_management\",\n      \"type\": \"float\"\n    },\n    {\n      \"name\": \"job_retired\",\n      \"type\": \"float\"\n    },\n    {\n      \"name\": \"job_self-employed\",\n      \"type\": \"float\"\n    },\n    {\n      \"name\": \"job_services\",\n      \"type\": \"float\"\n    },\n    {\n      \"name\": \"job_student\",\n      \"type\": \"float\"\n    },\n    {\n      \"name\": \"job_technician\",\n      \"type\": \"float\"\n    },\n    {\n      \"name\": \"job_unemployed\",\n      \"type\": \"float\"\n    },\n    {\n      \"name\": \"marital_divorced\",\n      \"type\": \"float\"\n    },\n    {\n      \"name\": \"marital_married\",\n      \"type\": \"float\"\n    },\n    {\n      \"name\": \"marital_single\",\n      \"type\": \"float\"\n    }\n  ],\n  \"vendor\": \"manual\"\n}";
        let _value = serde_json::from_slice::<DomainData>(json_string.as_bytes()).unwrap();

        let body = QueryDomainDataSourceRequest {
            header: None,
            datasource_id: "datasource".to_owned(),
        };
        serde_json::to_string(&body).unwrap();

        let json_string = "{\"status\":{\"code\":12302, \"message\":\"domaindatasources.kuscia.secretflow \\\"default-data-source\\\" not found\", \"details\":[]}, \"data\":null}";
        let _value =
            serde_json::from_slice::<QueryDomainDataSourceResponse>(json_string.as_bytes())
                .unwrap();
    }
}
