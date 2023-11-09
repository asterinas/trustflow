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


use crate::error;
use kuscia_proto::api::datamesh::{
    CreateDomainDataRequest, CreateDomainDataResponse, DomainData, DomainDataSource,
    QueryDomainDataRequest, QueryDomainDataResponse, QueryDomainDataSourceRequest,
    QueryDomainDataSourceResponse,
};

#[derive(Debug)]
pub struct DataMeshClient {
    // data mesh endpoint, e.g. datamesh:8070
    endpoint: String,

    // request client
    http_client: reqwest::Client,
}

impl DataMeshClient {
    pub fn new(endpoint: String) -> Result<Self, error::Error> {
        // Read CA, client certificate and client private key from env variables,
        // since accessing the Data Mesh requires the use of m-TLS
        let server_root_ca_cert = reqwest::Certificate::from_pem(
            std::fs::read_to_string(std::env::var("TRUSTED_CA_FILE")?)?.as_bytes(),
        )?;
        let mut client_pem = std::fs::read_to_string(std::env::var("CLIENT_CERT_FILE")?)?;
        client_pem
            .push_str(std::fs::read_to_string(std::env::var("CLIENT_PRIVATE_KEY_FILE")?)?.as_str());
        let client_identity = reqwest::Identity::from_pem(client_pem.as_bytes())?;
        let client = reqwest::Client::builder().use_rustls_tls();
        let http_client = client
            .add_root_certificate(server_root_ca_cert)
            .identity(client_identity)
            .https_only(true)
            .build()?;

        Ok(DataMeshClient {
            endpoint,
            http_client,
        })
    }

    async fn post_data<T: serde::Serialize, U: serde::de::DeserializeOwned>(
        &self,
        path: &str,
        body: &T,
    ) -> Result<U, error::Error> {
        let body_json_string = serde_json::to_string(body)?;
        let body_json: serde_json::Value = serde_json::from_slice(body_json_string.as_bytes())?;
        let client = self.http_client.clone();
        let url = format!("https://{}/{}", self.endpoint, path);
        let response_json = client
            .post(&url)
            .json(&body_json)
            .send()
            .await?
            .bytes()
            .await?;
        let response: U = serde_json::from_slice(&response_json)?;
        Ok(response)
    }

    pub async fn get_domain_data(&self, domaindata_id: &str) -> Result<DomainData, error::Error> {
        let body = QueryDomainDataRequest {
            header: None,
            domaindata_id: domaindata_id.to_owned(),
        };
        let response: QueryDomainDataResponse = self
            .post_data("api/v1/datamesh/domaindata/query", &body)
            .await?;
        let status = response.status.ok_or("status is empty")?;
        if status.code != 0 {
            return Err(
                format!("status code: {}, err msg: {}", status.code, status.message).into(),
            );
        }
        Ok(response.data.ok_or("datasource data is empty")?)
    }

    pub async fn get_domain_datasource(
        &self,
        datasource_id: &str,
    ) -> Result<DomainDataSource, error::Error> {
        let body = QueryDomainDataSourceRequest {
            header: None,
            datasource_id: datasource_id.to_owned(),
        };
        let response: QueryDomainDataSourceResponse = self
            .post_data("api/v1/datamesh/domaindatasource/query", &body)
            .await?;
        let status = response.status.ok_or("status is empty")?;
        if status.code != 0 {
            return Err(
                format!("status code: {}, err msg: {}", status.code, status.message).into(),
            );
        }
        Ok(response.data.ok_or("datasource data is empty")?)
    }

    pub async fn create_domain_data(
        &self,
        domaindata_id: &str,
        relative_uri: &str,
        datasource_id: &str,
        domain_data: &DomainData,
    ) -> Result<String, error::Error> {
        let body = CreateDomainDataRequest {
            header: None,
            domaindata_id: domaindata_id.to_owned(),
            name: domain_data.name.clone(),
            r#type: domain_data.r#type.to_owned(),
            relative_uri: relative_uri.to_owned(),
            datasource_id: datasource_id.to_owned(),
            attributes: domain_data.attributes.clone(),
            partition: domain_data.partition.clone(),
            columns: domain_data.columns.clone(),
            vendor: domain_data.vendor.clone(),
            file_format: domain_data.file_format,
        };
        let response: CreateDomainDataResponse = self
            .post_data("api/v1/datamesh/domaindata/create", &body)
            .await?;
        let status = response.status.ok_or("status is empty")?;
        if status.code != 0 {
            return Err(
                format!("status code: {}, err msg: {}", status.code, status.message).into(),
            );
        }
        let data = response.data.ok_or("data is none")?;
        Ok(data.domaindata_id)
    }
}
