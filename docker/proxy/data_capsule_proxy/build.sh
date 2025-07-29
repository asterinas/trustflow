#!/bin/bash
#
# Copyright 2023 Ant Group Co., Ltd.
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
#

set -e

show_help() {
    echo "Usage: bash build.sh [OPTION]..."
    echo "  -p --plat"
    echo "          the platform to build with. sim/tdx/csv."
    echo "  -v --version"
    echo "          the version to build with."
    echo "  -l --latest"
    echo "          tag this version as latest."
    echo "  -u --upload"
    echo "          upload to docker registry."
}

if [[ "$#" -lt 2 ]]; then
    show_help
    exit
fi

while [[ "$#" -ge 1 ]]; do
    case $1 in
        -p|--plat)
            PLATFORM="$2"
            shift

            if [[ "$#" -eq 0 ]]; then
                echo "Plat shall not be empty."
                echo ""
                show_help
                exit 1
            fi
            shift
        ;;
        -v|--version)
            VERSION="$2"
            shift
            
            if [[ "$#" -eq 0 ]]; then
                echo "Version shall not be empty."
                echo ""
                show_help
                exit 1
            fi
            shift
        ;;
        -l|--latest)
            LATEST=1
            shift
        ;;
        -u|--upload)
            UPLOAD=1
            shift
        ;;
        *)
            echo "Unknown argument passed: $1"
            exit 1
        ;;
    esac
done

if [[ -z ${VERSION} ]]; then
    echo "Please specify the version."
    exit 1
fi


GREEN="\033[32m"
NO_COLOR="\033[0m"

DOCKER_REG="secretflow"
ALIYUN_DOKER_PREFIX="secretflow-registry.cn-hangzhou.cr.aliyuncs.com"

case "$PLATFORM" in
  sim)
    IMAGE_TAG=${DOCKER_REG}/trustflow-data-capsule-proxy-sim-ubuntu22.04:${VERSION}
    LATEST_TAG=${DOCKER_REG}/trustflow-data-capsule-proxy-sim-ubuntu22.04:latest
    echo -e "Building ${GREEN}${IMAGE_TAG}${NO_COLOR}"
    cd ../../.. && docker build . -f docker/proxy/data_capsule_proxy/Dockerfile -t ${IMAGE_TAG} \
      --build-arg TEE_PLAT="sim"
    ;;
  tdx)
    IMAGE_TAG=${DOCKER_REG}/trustflow-data-capsule-proxy-tdx-ubuntu22.04:${VERSION}
    LATEST_TAG=${DOCKER_REG}/trustflow-data-capsule-proxy-tdx-ubuntu22.04:latest
    echo -e "Building ${GREEN}${IMAGE_TAG}${NO_COLOR}"
    cd ../../.. && docker build . -f docker/proxy/data_capsule_proxy/Dockerfile -t ${IMAGE_TAG} \
      --build-arg TEE_PLAT="tdx"
    ;;
  csv)
    IMAGE_TAG=${DOCKER_REG}/trustflow-data-capsule-proxy-csv-ubuntu22.04:${VERSION}
    LATEST_TAG=${DOCKER_REG}/trustflow-data-capsule-proxy-csv-ubuntu22.04:latest
    echo -e "Building ${GREEN}${IMAGE_TAG}${NO_COLOR}"
    cd ../../.. && docker build . -f docker/proxy/data_capsule_proxy/Dockerfile -t ${IMAGE_TAG} \
      --build-arg TEE_PLAT="csv"
    ;;
  *)
    echo -e "PLATFORM does not match any of options(sim/tdx/csv)"
    exit 1
    ;;
esac

echo -e "Finish building ${GREEN}${IMAGE_TAG}${NO_COLOR}"

if [[ UPLOAD -eq 1 ]]; then
    docker push ${IMAGE_TAG}
    docker tag ${IMAGE_TAG} ${ALIYUN_DOKER_PREFIX}/${IMAGE_TAG}
    docker push ${ALIYUN_DOKER_PREFIX}/${IMAGE_TAG}
fi


if [[ LATEST -eq 1 ]]; then
    echo -e "Tag ${GREEN}${LATEST_TAG}${NO_COLOR} ..."
    docker tag ${IMAGE_TAG} ${LATEST_TAG}
    docker tag ${LATEST_TAG} ${ALIYUN_DOKER_PREFIX}/${LATEST_TAG}
    if [[ UPLOAD -eq 1 ]]; then
        echo -e "Push ${GREEN}${LATEST_TAG}${NO_COLOR} ..."
        docker push ${LATEST_TAG}
        docker push ${ALIYUN_DOKER_PREFIX}/${LATEST_TAG}
    fi
fi