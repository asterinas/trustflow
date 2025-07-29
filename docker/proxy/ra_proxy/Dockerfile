# Copyright 2024 Ant Group Co., Ltd.
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#   http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

FROM secretflow/trustflow-dev-ubuntu22.04:latest AS builder

ARG TEE_PLAT="sim"
RUN echo "TEE_PLAT: ${TEE_PLAT}"

WORKDIR /home/admin/dev

COPY .bazelrc .bazelversion BUILD.bazel WORKSPACE ./
COPY bazel ./bazel
COPY trustflow ./trustflow

RUN bazel --output_base=target build -c opt --repository_cache=/tmp/bazel_repo_cache \
  --copt=-DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG --define tee_type=${TEE_PLAT} \
  //trustflow/proxy/ra_proxy:main


FROM secretflow/trustflow-release-ubuntu22.04:latest

COPY --from=builder /home/admin/dev/bazel-bin/trustflow/proxy/ra_proxy/main /home/trustflow/proxy/ra_proxy/main

WORKDIR /home/trustflow/proxy/ra_proxy