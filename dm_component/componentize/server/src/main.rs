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


mod client;
mod config;
mod constants;
mod error;
mod service;
mod utils;

use crate::client::data_mesh_client;
use hyper::server::conn::AddrStream;
use hyper::service::{make_service_fn, service_fn};
use hyper::Server;
use std::fs;
use std::path;
use std::sync::Arc;

type Error = Box<dyn std::error::Error + Send + Sync>;

#[tokio::main]
async fn main() {
    let config = config::Config::new();

    // init log
    utils::log::init_log(
        &config.log_dir,
        &config.log_level,
        config.enable_console_logger,
    );

    let addr = ([0, 0, 0, 0], config.port).into();

    if !path::Path::new(constants::TEMP_DIR).exists() {
        fs::create_dir(constants::TEMP_DIR).unwrap();
    }

    let data_mesh_client = Arc::new(
        data_mesh_client::DataMeshClient::new(config.data_mesh_endpoint)
            .expect("create data mesh client failed."),
    );

    let (tx, rx) = tokio::sync::oneshot::channel::<()>();

    service::SHUTDOWN_TX.lock().unwrap().replace(tx);

    let make_svc = make_service_fn(|socket: &AddrStream| {
        println!("request from: {}\n", socket.remote_addr());
        let data_mesh_client = data_mesh_client.clone();
        async move {
            Ok::<_, Error>(service_fn(move |req| {
                service::serve_req(req, data_mesh_client.clone())
            }))
        }
    });

    // Then bind and serve...
    let server = Server::bind(&addr).serve(make_svc);

    log::info!("server running on {}", server.local_addr());

    let graceful = server.with_graceful_shutdown(async {
        rx.await.ok();
    });

    if let Err(e) = graceful.await {
        eprintln!("an error occurred: {}", e);
    }
}
