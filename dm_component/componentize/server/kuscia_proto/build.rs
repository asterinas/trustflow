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


fn main() -> Result<(), Box<dyn std::error::Error>> {
    // Use this in build.rs
    let mut config = prost_build::Config::new();
    config.type_attribute(".", "#[derive(serde::Deserialize, serde::Serialize)]");
    
    config.field_attribute("file_format", "#[serde(deserialize_with = \"crate::custom_serde::FileFormat::from_str\")]");
    config.field_attribute(".kuscia.proto.api.v1alpha1.datamesh", "#[serde(default)]");
    config.field_attribute(".kuscia.proto.api.v1alpha1.RequestHeader", "#[serde(default)]");
    config.field_attribute(".kuscia.proto.api.v1alpha1.Status", "#[serde(default)]");
    config.field_attribute(".kuscia.proto.api.v1alpha1.Partition", "#[serde(default)]");
    config.field_attribute(".kuscia.proto.api.v1alpha1.DataColumn", "#[serde(default)]");
    config.extern_path(".google.protobuf.Any", "::prost_wkt_types::Any");
    config.compile_protos(
        &[
            "src/datamesh/domaindatasource.proto",
            "src/datamesh/domaindata.proto",
        ],
        &["src/"],
    )?;
    Ok(())
}
