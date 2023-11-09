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


pub mod custom_serde;
// Include the `items` module, which is generated from items.proto.
// It is important to maintain the same structure as in the proto.
pub mod api {
    include!(concat!(env!("OUT_DIR"), "/kuscia.proto.api.v1alpha1.rs"));
    pub mod datamesh {
        include!(concat!(env!("OUT_DIR"), "/kuscia.proto.api.v1alpha1.datamesh.rs"));
    }
}