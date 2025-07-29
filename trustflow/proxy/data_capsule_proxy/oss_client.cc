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

#include "trustflow/proxy/data_capsule_proxy/oss_client.h"

#include <filesystem>

#include "spdlog/spdlog.h"
#include "yacl/base/exception.h"

namespace trustflow {
namespace proxy {
namespace data_capsule_proxy {

namespace {
AlibabaCloud::OSS::OssClient GetOssClient(const std::string& endpoint,
                                          const std::string& ak,
                                          const std::string& sk,
                                          const std::string& sts_token) {
  AlibabaCloud::OSS::ClientConfiguration conf;
  if (sts_token.empty()) {
    return AlibabaCloud::OSS::OssClient(endpoint, ak, sk, conf);
  } else {
    return AlibabaCloud::OSS::OssClient(endpoint, ak, sk, sts_token, conf);
  }
}
}  // namespace

// Download from oss
// Support single file or a directory
void DownloadFromOss(const std::string& endpoint, const std::string& bucket,
                     const std::string& src_path, const std::string& dest_path,
                     const std::string& ak, const std::string& sk,
                     const std::string& sts_token) {
  auto oss_client = GetOssClient(endpoint, ak, sk, sts_token);

  // list src_path
  AlibabaCloud::OSS::ListObjectsRequest list_request(bucket);
  list_request.setPrefix(src_path);

  auto list_res = oss_client.ListObjects(list_request);
  YACL_ENFORCE(list_res.isSuccess(), "oss list object failed, error {}: {}",
               list_res.error().Code(), list_res.error().Message());
  YACL_ENFORCE(list_res.result().ObjectSummarys().size() > 0,
               "no object found in {}", src_path);

  for (const auto& object : list_res.result().ObjectSummarys()) {
    const std::filesystem::path object_path(object.Key());
    const std::filesystem::path relative_path =
        std::filesystem::relative(object_path, src_path);
    std::filesystem::path dest_object_path;
    if (relative_path == ".") {
      // use object's filename instead of .
      dest_object_path =
          std::filesystem::path(dest_path / object_path.filename());
    } else {
      dest_object_path = std::filesystem::path(dest_path / relative_path);
    }

    if (!std::filesystem::exists(dest_object_path.parent_path())) {
      std::filesystem::create_directories(dest_object_path.parent_path());
    }

    AlibabaCloud::OSS::DownloadObjectRequest download_request(
        bucket, object.Key(), dest_object_path.string());

    SPDLOG_INFO("Downloading {} to {}", object.Key(),
                dest_object_path.string());
    const auto download_res =
        oss_client.ResumableDownloadObject(download_request);
    YACL_ENFORCE(download_res.isSuccess(),
                 "oss download object failed, error {}: {}",
                 download_res.error().Code(), download_res.error().Message());
    SPDLOG_INFO("Download {} to {} success", object.Key(),
                dest_object_path.string());
  }
}

// Upload to oss
// Support single file or a directory
void UploadToOss(const std::string& endpoint, const std::string& bucket,
                 const std::string& src_path, const std::string& dest_path,
                 const std::string& ak, const std::string& sk,
                 const std::string& sts_token) {
  YACL_ENFORCE(std::filesystem::exists(src_path), "src_path {} not exists",
               src_path);

  const auto oss_client = GetOssClient(endpoint, ak, sk, sts_token);

  std::filesystem::path dest_object_path;

  if (std::filesystem::is_regular_file(src_path)) {
    dest_object_path = std::filesystem::path(dest_path) /
                       std::filesystem::path(src_path).filename();

    SPDLOG_INFO("Uploading {} to {}", src_path, dest_object_path.string());
    AlibabaCloud::OSS::UploadObjectRequest upload_request(
        bucket, dest_object_path, src_path);
    const auto upload_res = oss_client.ResumableUploadObject(upload_request);
    YACL_ENFORCE(upload_res.isSuccess(),
                 "oss upload object failed, error {}: {}",
                 upload_res.error().Code(), upload_res.error().Message());
    SPDLOG_INFO("Upload {} to {} success", src_path, dest_object_path.string());
  } else if (std::filesystem::is_directory(src_path)) {
    // for directory, upload all files in the directory
    // and its subdirectories
    for (const auto& src_item :
         std::filesystem::recursive_directory_iterator(src_path)) {
      if (std::filesystem::is_regular_file(src_item.path())) {
        const std::filesystem::path relative_path =
            std::filesystem::relative(src_item.path(), src_path);

        dest_object_path = std::filesystem::path(dest_path) / relative_path;

        SPDLOG_INFO("Uploading {} to {}", src_item.path().string(),
                    dest_object_path.string());
        AlibabaCloud::OSS::UploadObjectRequest upload_request(
            bucket, dest_object_path, src_item.path().string());
        const auto upload_res =
            oss_client.ResumableUploadObject(upload_request);
        YACL_ENFORCE(upload_res.isSuccess(),
                     "oss upload object failed, error {}: {}",
                     upload_res.error().Code(), upload_res.error().Message());
        SPDLOG_INFO("Upload {} to {} success", src_item.path().string(),
                    dest_object_path.string());
      }
    }
  } else {
    YACL_THROW("src_path is not a file or directory");
  }
}

}  // namespace data_capsule_proxy
}  // namespace proxy
}  // namespace trustflow