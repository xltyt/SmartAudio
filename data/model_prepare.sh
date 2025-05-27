#!/bin/bash

CUR_FILE=`readlink -f $0`
CUR_DIR=`dirname $CUR_FILE`
cd $CUR_DIR


mkdir -p model
if [ ! -f model/campplus.onnx ]; then
  cp model_ori/campplus.onnx model
fi

if [ ! -f model/speech_tokenizer_v1.onnx ]; then
  cp model_ori/speech_tokenizer_v1.onnx model
fi

# vim: set expandtab ts=4 sw=4 sts=4:
