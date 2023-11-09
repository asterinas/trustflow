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


use serde::{Deserializer, Deserialize, de};

pub enum FileFormat {
    Unknown = 0,
    Csv = 1,
}

impl FileFormat {
    pub fn from_str<'de, D>(deserializer: D) -> Result<i32, D::Error>
    where
        D: Deserializer<'de>,
    {
        let s: &str = Deserialize::deserialize(deserializer)?;

        match s.to_lowercase().as_str() {
            "unknown" => Ok(Self::Unknown as i32),
            "csv" => Ok(Self::Csv as i32),
            _ => Err(de::Error::unknown_variant(s, &["UNKNOWN", "CSV"])),
        }
    }
}