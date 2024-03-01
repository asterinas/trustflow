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

FROM secretflow/occlum:0.29.5-ubuntu20.04

LABEL maintainer="secretflow-contact@service.alipay.com"

# change dash to bash as default shell
RUN ln -sf /usr/bin/bash /bin/sh

# change source list (just for speeding up in install)
RUN mv /etc/apt/sources.list /etc/apt/sources.list.bak
COPY sources.list.ubuntu20.04 /etc/apt/sources.list
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
RUN npm config set registry http://registry.npm.taobao.org \
  && npm install -g @bazel/bazelisk

# install rust
RUN curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | bash -s -- -y

# upgrade gcc and g++ to 11
RUN apt update \
    && apt upgrade -y \
    && apt install -y software-properties-common \
    && add-apt-repository -y ppa:ubuntu-toolchain-r/test \
    # change ppa source (just for speeding up in install) \
    && sed -i "s/ppa.launchpad.net/launchpad.proxy.ustclug.org/g" /etc/apt/sources.list.d/*.list \
    && sed -ri 's#(.*http)(://launchpad.proxy.ustclug.org.*)#\1s\2#g' /etc/apt/sources.list /etc/apt/sources.list.d/*.list \
    ################################################## \
    && apt update \
    && apt purge -y gcc-9 g++-9 \
    && apt install -y gcc-11 g++-11 lld-11 libasan6 \
    git wget curl unzip autoconf make nasm \
    cmake ninja-build nasm vim-common libgl1 libglib2.0-0 \
    && apt clean \
    && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-11 100 \
    && update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-11 100 \
    && update-alternatives --install /usr/bin/ld.lld ld.lld /usr/bin/ld.lld-11 100

# install dcap lib in ubuntu 20.04
RUN echo "ca_directory=/etc/ssl/certs" >> /etc/wgetrc \
  && echo 'deb [arch=amd64] https://download.01.org/intel-sgx/sgx_repo/ubuntu focal main' | tee /etc/apt/sources.list.d/intel-sgx.list \
  && wget -qO - https://download.01.org/intel-sgx/sgx_repo/ubuntu/intel-sgx-deb.key --no-check-certificate | apt-key add - \
  && apt update && apt install -y libsgx-epid libsgx-quote-ex libsgx-dcap-ql && apt clean \
  && rm -f intel-sgx-deb.key

# install sgx sdk in ubuntu 20.04
RUN wget https://download.01.org/intel-sgx/latest/linux-latest/distro/ubuntu20.04-server/sgx_linux_x64_sdk_2.22.100.3.bin \
  && chmod +x sgx_linux_x64_sdk_2.22.100.3.bin \
  && ./sgx_linux_x64_sdk_2.22.100.3.bin --prefix /opt/intel \
  && source /opt/intel/sgxsdk/environment \
  && rm -f sgx_linux_x64_sdk_2.22.100.3.bin
