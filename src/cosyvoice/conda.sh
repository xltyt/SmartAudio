#!/bin/bash

source ~/local/conda_env.sh
conda create -n audio python=3.10
conda activate audio
#conda install setuptools
conda install -c conda-forge "setuptools<81.0.0"
sed -i '/openai-whisper/d' docker/requirements.txt
pip install -r docker/requirements.txt -i https://pypi.tuna.tsinghua.edu.cn/simple
pip install openai-whisper==20231117  -i https://mirrors.aliyun.com/pypi/simple/ --no-build-isolation
pip install "ruamel.yaml<0.19.0"

# vim: set expandtab ts=4 sw=4 sts=4:
