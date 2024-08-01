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

ARG BASE_IMAGE=ubuntu:22.04
FROM ${BASE_IMAGE}

LABEL maintainer="secretflow-contact@service.alipay.com"

# change dash to bash as default shell
RUN ln -sf /usr/bin/bash /bin/sh



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
    vim \
    golang \
    cmake \
    ninja-build  \
    curl \
    ssh \
    llvm-dev libclang-dev clang \
    rsync \
    libfuse2 \
    && rm -f /etc/ssh/ssh_host_* \
    && apt clean 

# instal protoc v3.19.4
RUN curl -LO https://github.com/protocolbuffers/protobuf/releases/download/v3.19.4/protoc-3.19.4-linux-x86_64.zip \
   && unzip protoc-3.19.4-linux-x86_64.zip -d /root/.local && echo 'export PATH="/root/.local/bin:$PATH"' >> /root/.bashrc \
   && rm -f protoc-3.19.4-linux-x86_64.zip


# install conda
RUN wget http://repo.anaconda.com/miniconda/Miniconda3-py310_24.4.0-0-Linux-x86_64.sh \
  && bash Miniconda3-py310_24.4.0-0-Linux-x86_64.sh -b && rm -f Miniconda3-py310_24.4.0-0-Linux-x86_64.sh \
  && ln -sf /root/miniconda3/bin/conda /usr/bin/conda \
  && conda init


# install bazelisk 
RUN npm install -g @bazel/bazelisk

# install emsdk
RUN git clone https://github.com/emscripten-core/emsdk.git /opt/emsdk && cd /opt/emsdk \
    && ./emsdk install latest && ./emsdk activate latest && echo "source /opt/emsdk/emsdk_env.sh" >> /root/.bashrc


# install rust
RUN curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | bash -s -- -y
RUN curl -LsSf https://get.nexte.st/latest/linux | tar zxf - -C ${CARGO_HOME:-~/.cargo}/bin

# install dcap lib in ubuntu 22.04
RUN echo "ca_directory=/etc/ssl/certs" >> /etc/wgetrc \
  && echo 'deb [signed-by=/etc/apt/keyrings/intel-sgx-keyring.asc arch=amd64] https://download.01.org/intel-sgx/sgx_repo/ubuntu jammy main' | tee /etc/apt/sources.list.d/intel-sgx.list \
  && wget https://download.01.org/intel-sgx/sgx_repo/ubuntu/intel-sgx-deb.key \
  && cat intel-sgx-deb.key | tee /etc/apt/keyrings/intel-sgx-keyring.asc > /dev/null \
  && rm -f intel-sgx-deb.key \
  && apt update && apt install -y libsgx-epid libsgx-quote-ex libsgx-dcap-ql libsgx-dcap-quote-verify-dev libsgx-dcap-default-qpl && apt clean \
  && pushd /usr/lib/x86_64-linux-gnu/ && ln -s libdcap_quoteprov.so.1 libdcap_quoteprov.so && popd

# install intel sgx sdk in ubuntu 22.04
RUN wget https://download.01.org/intel-sgx/latest/linux-latest/distro/ubuntu22.04-server/sgx_linux_x64_sdk_2.23.100.2.bin \
  && chmod +x sgx_linux_x64_sdk_2.23.100.2.bin \
  && ./sgx_linux_x64_sdk_2.23.100.2.bin --prefix /opt/intel \
  && source /opt/intel/sgxsdk/environment \
  && rm -f sgx_linux_x64_sdk_2.23.100.2.bin