#!/usr/bin/python
# -*- coding: utf-8 -*-

import tornado
from tornado.web import RequestHandler, Application
from tornado.ioloop import IOLoop
import time
import json
import os
import sys
import argparse
import base64
import binascii
from io import BytesIO
sys.path.append(os.getcwd() + '/pkg/third_party/Matcha-TTS')
sys.path.append(os.getcwd() + '/pkg/')
from cosyvoice.cli.cosyvoice import CosyVoice, CosyVoice2
from cosyvoice.utils.file_utils import load_wav
import torchaudio
import wave

#_cosyvoice = CosyVoice2('CosyVoice2-0.5B', load_jit=False, load_trt=False, fp16=True)
#_cosyvoice = CosyVoice('CosyVoice-300M', load_jit=False, load_trt=False, fp16=False)
_cosyvoice = CosyVoice('../../data/model_ori/', load_jit=False, load_trt=False, fp16=True)

for step in range(10):
  prompt_speech_16k = load_wav('mda-qmwfy2k746929rxh.wav', 16000)
  _ori_text = "2024年，我们一起走过春夏秋冬，一道经历风雨彩虹，一个个瞬间定格在这不平凡的一年，令人感慨、难以忘怀。"
  text = '主席说我开飞机的水平很高。今天天气不错，出去玩吧，划划船'
  tts_result = _cosyvoice.inference_zero_shot(text, _ori_text, prompt_speech_16k, stream=False)
  rand_key = str(binascii.hexlify(os.urandom(8)), "utf-8")
  for i, j in enumerate(tts_result):
    result_path = '/tmp/test_{}_{}.wav'.format(rand_key, i)
    torchaudio.save(result_path, j['tts_speech'], _cosyvoice.sample_rate)

def parse_arguments():
  parser = argparse.ArgumentParser()
  parser.add_argument('--server_port', type=int, default=28801)
  return parser.parse_args()

class Searcher(RequestHandler):
  def post(self):
    post_data = json.loads(self.request.body)
    text = post_data['text']
    text_speech = post_data['text_speech']
    out_text = post_data['out_text']
    
    tmp_path = "/tmp/wave_" + str(binascii.hexlify(os.urandom(8)), "utf-8")
    wave_path = tmp_path + ".wav"
    print("Ori[%s] OutText[%s] FileName[%s]" % (text, out_text, tmp_path))
    with open(tmp_path, 'wb') as f:
      f.write(base64.b64decode(text_speech.encode("utf-8")))
    os.system("ffmpeg -i %s -ar 16000 %s" % (tmp_path, wave_path))
    wf = wave.open(wave_path, 'rb')
    nchannels = wf.getnchannels()
    sampwidth = wf.getsampwidth()
    framerate = wf.getframerate()
    nframes = wf.getnframes()
    duration = nframes / framerate
    wf.close()
    if 16000 != framerate:
      raise ValueError("Invalid File Fs")
    prompt_speech_16k = load_wav(wave_path, 16000)
    rand_key = str(binascii.hexlify(os.urandom(8)), "utf-8")
    tts_result = _cosyvoice.inference_zero_shot(out_text, text, prompt_speech_16k, stream=False)
    result_name = 'output_{}.wav'.format(rand_key)
    concat_file = open("/tmp/concat.txt", "w")
    sub_files = []
    for i, j in enumerate(tts_result):
      sub_result_name = 'output_{}_{}.wav'.format(rand_key, i)
      torchaudio.save("/tmp/" + sub_result_name, j['tts_speech'], _cosyvoice.sample_rate)
      concat_file.write("file '/tmp/%s'\n" % sub_result_name)
      sub_files.append("/tmp/%s" % sub_result_name)
    concat_file.close()
    cmd = "ffmpeg -f concat -safe 0 -i /tmp/concat.txt -c copy /tmp/%s" % (result_name)
    print(cmd)
    os.system(cmd)
    for sub_file in sub_files:
      os.unlink(sub_file)
    f = open("/tmp/" + result_name, "rb")
    obj_result = {}
    obj_result["status"] = 0
    obj_result["data"] = base64.b64encode(f.read()).decode('utf-8')
    resp = json.dumps(obj_result, ensure_ascii = False)
    print(resp)
    self.write(resp)
    return

def make_app():
  return Application([
    (r"/audio", Searcher),
  ])

if __name__ == "__main__":
  args = parse_arguments()
  app = make_app()
  app.listen(args.server_port, reuse_port = True)
  print("server start, port:", args.server_port)
  IOLoop.current().start()

# vim: set expandtab ts=4 sw=4 sts=4:
