FROM secretflow/occlum:0.29.5-ubuntu20.04

LABEL maintainer="secretflow-contact@service.alipay.com"

# change dash to bash as default shell
RUN ln -sf /usr/bin/bash /bin/sh

# change source list (just for speeding up in install)
RUN mv /etc/apt/sources.list /etc/apt/sources.list.bak
COPY sources.list /etc/apt/sources.list
###########################################

# install gcc g++
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


# install conda
RUN wget https://repo.anaconda.com/miniconda/Miniconda3-py38_23.5.2-0-Linux-x86_64.sh \
    && bash Miniconda3-py38_23.5.2-0-Linux-x86_64.sh -b \
    && rm -f Miniconda3-py38_23.5.2-0-Linux-x86_64.sh \
    && /root/miniconda3/bin/conda update -n base -c defaults conda \
    && /root/miniconda3/bin/conda init \
    && /root/miniconda3/bin/conda update --all -y 
RUN /root/miniconda3/bin/pip list -o | cut -f1 -d' ' | tr " " "\n" | awk '{if(NR>=3)print}' | cut -d' ' -f1 | xargs -n1 /root/miniconda3/bin/pip install -U \
    && /root/miniconda3/bin/pip cache purge

ENV PATH="/root/miniconda3/bin:${PATH}" 

# Update libstdc++
RUN conda install -c conda-forge libstdcxx-ng && conda clean --all -f --yes

# install bazel 
RUN wget https://github.com/bazelbuild/bazelisk/releases/download/v1.18.0/bazelisk-linux-amd64 \
    && chmod 777 bazelisk-linux-amd64 \
    && mv bazelisk-linux-amd64 /usr/bin/bazel \
    && ln -s /usr/bin/bazel /usr/bin/bazelisk   

# install rust
RUN curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | bash -s -- -y

# run as root for now
WORKDIR /home/admin/
