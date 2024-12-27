// Copyright 2024 Ant Group Co., Ltd.
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

#include "trustflow/proxy/utils/io_util.h"

#include "yacl/io/stream/file_io.h"

namespace trustflow {
namespace proxy {
namespace utils {

std::string ReadFile(const std::string& file_path) {
  yacl::io::FileInputStream in(file_path);
  std::string content;
  content.resize(in.GetLength());
  in.Read(content.data(), content.size());
  in.Close();
  return content;
}

void WriteFile(const std::string& file_path, yacl::ByteContainerView content) {
  yacl::io::FileOutputStream out(file_path);
  out.Write(content.data(), content.size());
  out.Close();
}

}  // namespace utils
}  // namespace proxy
}  // namespace trustflow