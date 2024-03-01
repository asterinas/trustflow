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

FROM docker.io/ubuntu:22.04

LABEL maintainer="secretflow-contact@service.alipay.com"

# change dash to bash as default shell
RUN ln -sf /usr/bin/bash /bin/sh

# change source list (just for speeding up in install)
RUN mv /etc/apt/sources.list /etc/apt/sources.list.bak
COPY sources.list.ubuntu22.04 /etc/apt/sources.list
###########################################


RUN apt update && DEBIAN_FRONTEND=noninteractive TZ=Etc/UTC apt install -y \
    tzdata \
    build-essential \
    ocaml \
    automake \
    autoconf \
    libtool \
    wget \
    python-is-python3 \
    python3-pip \
    libssl-dev \
    npm \
    git \
    debhelper \
    zip \
    libcurl4-openssl-dev \
    pkgconf \
    libboost-dev \
    libboost-system-dev \
    libboost-thread-dev \
    protobuf-c-compiler \
    libprotobuf-c-dev \
    protobuf-compiler \
    vim \
    golang \
    cmake \
    ninja-build  \
    curl \
    ssh \
    llvm-dev libclang-dev clang \
    && rm -f /etc/ssh/ssh_host_* \
    && apt clean


# install conda
RUN wget http://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-x86_64.sh \
  && bash Miniconda3-latest-Linux-x86_64.sh -b && rm -f Miniconda3-latest-Linux-x86_64.sh \
  && ln -sf /root/miniconda3/bin/conda /usr/bin/conda \
  && conda init
COPY .condarc /root/.condarc

# install python packages
RUN python3 -m pip config set global.index-url https://mirrors.aliyun.com/pypi/simple/ \
    && python3 -m pip install --upgrade pip \
    && python3 -m pip cache purge \
    && pip install pandas xgboost statsmodels scikit-learn sklearn2pmml

# install bazelisk
RUN npm install -g @bazel/bazelisk

# install emsdk
RUN git clone https://github.com/emscripten-core/emsdk.git && cd emsdk \
    && ./emsdk install latest && ./emsdk activate latest && echo 'source $PWD/emsdk_env.sh' >> /root/.bashrc


# install rust
RUN curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | bash -s -- -y

# install dcap lib in ubuntu 22.04
RUN echo "ca_directory=/etc/ssl/certs" >> /etc/wgetrc \
  && echo 'deb [signed-by=/etc/apt/keyrings/intel-sgx-keyring.asc arch=amd64] https://download.01.org/intel-sgx/sgx_repo/ubuntu jammy main' | tee /etc/apt/sources.list.d/intel-sgx.list \
  && wget https://download.01.org/intel-sgx/sgx_repo/ubuntu/intel-sgx-deb.key \
  && cat intel-sgx-deb.key | tee /etc/apt/keyrings/intel-sgx-keyring.asc > /dev/null \
  && rm -f intel-sgx-deb.key \
  && apt update && apt install -y libsgx-epid libsgx-quote-ex libsgx-dcap-ql libsgx-dcap-quote-verify-dev && apt clean

# install intel sgx sdk in ubuntu 22.04
RUN wget https://download.01.org/intel-sgx/latest/linux-latest/distro/ubuntu22.04-server/sgx_linux_x64_sdk_2.23.100.2.bin \
  && chmod +x sgx_linux_x64_sdk_2.23.100.2.bin \
  && ./sgx_linux_x64_sdk_2.23.100.2.bin --prefix /opt/intel \
  && source /opt/intel/sgxsdk/environment \
  && rm -f sgx_linux_x64_sdk_2.23.100.2.bin