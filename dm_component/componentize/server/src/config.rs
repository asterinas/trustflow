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


use std::{env, fs};

use clap::{CommandFactory, FromArgMatches, Parser};

#[derive(Parser)]
#[command(author, version, about = "http server", long_about = None)]
pub(super) struct Config {
    /// port
    #[arg(short, long, default_value_t = 10001, action = clap::ArgAction::Append)]
    pub port: u16,

    /// data mesh endpoint
    #[arg(short, long, default_value_t = String::from("datamesh:8070"), action = clap::ArgAction::Append)]
    pub data_mesh_endpoint: String,

    /// domain id
    #[arg(short, long, default_value_t = String::from(""), action = clap::ArgAction::Append)]
    pub domain_id: String,

    /// log file name
    #[arg(long, default_value_t = String::from("log"), action = clap::ArgAction::Append)]
    pub log_dir: String,

    /// log level
    #[arg(long, default_value_t = String::from("info"), action = clap::ArgAction::Append)]
    pub log_level: String,

    /// enable console logger
    #[arg(long, default_value_t = true, action = clap::ArgAction::Append)]
    pub enable_console_logger: bool,

    /// Sets a custom config file
    #[arg(short, long)]
    config_file_path: Option<String>,
}

impl Config {
    pub fn new() -> Config {
        let mut args: Vec<String> = env::args().collect();
        let mut matches: clap::ArgMatches =
            <Self as CommandFactory>::command().get_matches_from(args.clone());
        if let Some(config_file_path) = matches.get_one::<String>("config_file_path") {
            let config_file_content = fs::read_to_string(config_file_path)
                .expect(format!("file `{config_file_path}` is not exist.").as_str());
            let mut file_args: Vec<String> = config_file_content
                .split_ascii_whitespace()
                .into_iter()
                .map(|x| x.to_string())
                .collect();
            args.append(&mut file_args);
            let mut matches: clap::ArgMatches =
                <Self as CommandFactory>::command().get_matches_from(args.clone());
            let config = <Self as FromArgMatches>::from_arg_matches_mut(&mut matches).unwrap();
            return config;
        }

        let config = <Self as FromArgMatches>::from_arg_matches_mut(&mut matches).unwrap();
        config
    }
}
