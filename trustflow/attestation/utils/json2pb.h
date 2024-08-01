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

#pragma once

#include "google/protobuf/util/json_util.h"
#include "yacl/base/exception.h"

#define PB2JSON(pbmsg, p_jsonstr)                                     \
  do {                                                                \
    ::google::protobuf::util::JsonPrintOptions options;               \
    options.preserve_proto_field_names = false;                       \
    using google::protobuf::util::MessageToJsonString;                \
    (p_jsonstr)->clear();                                             \
    auto status = MessageToJsonString((pbmsg), (p_jsonstr), options); \
    YACL_ENFORCE(status.ok(),                                         \
                 "Parsing pb message to json failed, "                \
                 "error: {}",                                         \
                 status.ToString());                                  \
  } while (0)

#define JSON2PB(jsonstr, p_pbmsg)                                \
  do {                                                           \
    using google::protobuf::util::JsonStringToMessage;           \
    auto status = JsonStringToMessage((jsonstr), (p_pbmsg));     \
    YACL_ENFORCE(status.ok(),                                    \
                 "Parsing json to pb message failed, json: {}, " \
                 "error: {}",                                    \
                 jsonstr, status.ToString());                    \
  } while (0)
