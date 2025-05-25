#!/usr/bin/python
# -*- coding: utf-8 -*-

import time
import json
import os
import sys
import argparse
import base64
import binascii
from io import BytesIO
sys.path.append('pkg/third_party/Matcha-TTS')
sys.path.append('pkg/')
from cosyvoice.cli.cosyvoice import CosyVoice, CosyVoice2
from cosyvoice.utils.file_utils import load_wav
import torch
import torchaudio

# pip install "ruamel.yaml<0.19.0"

#_cosyvoice = CosyVoice2('CosyVoice2-0.5B', load_jit=False, load_trt=False, fp16=True)
#_cosyvoice = CosyVoice('CosyVoice-300M', load_jit=False, load_trt=False, fp16=True)
_cosyvoice = CosyVoice('model', load_jit=False, load_trt=False, fp16=True)
#prompt_speech_16k = load_wav('/data/liuyong1/cosyvoice/female_1_prompt.wav', 16000)
prompt_speech_16k = load_wav('mda-qmwfy2k746929rxh.wav', 16000)
#_ori_text = "希望你以后能够做的比我还好呦。"
_ori_text = "2024年，我们一起走过春夏秋冬，一道经历风雨彩虹，一个个瞬间定格在这不平凡的一年，令人感慨、难以忘怀。"
#text = '家事国事天下事，让人民过上幸福生活是头等大事。家家户户都盼着孩子能有好的教育，老人能有好的养老服务，年轻人能有更多发展机会。这些朴实的愿望，'
#text = '主席表扬我是一个好同志'
text = '主席说我开飞机的水平很高'
tts_result = _cosyvoice.inference_zero_shot(text, _ori_text, prompt_speech_16k, stream=False)
rand_key = str(binascii.hexlify(os.urandom(8)), "utf-8")
for i, j in enumerate(tts_result):
  result_path = '/tmp/test_{}_{}.wav'.format(rand_key, i)
  torchaudio.save(result_path, j['tts_speech'], _cosyvoice.sample_rate)

# vim: set expandtab ts=4 sw=4 sts=4:
