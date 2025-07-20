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
sys.path.append('/home/work/SmartAudio/src/_ref/cosyvoice/CosyVoice/third_party/Matcha-TTS')
sys.path.append('/home/work/SmartAudio/src/_ref/cosyvoice/CosyVoice')
from cosyvoice.cli.cosyvoice import CosyVoice, CosyVoice2
from cosyvoice.utils.file_utils import load_wav
import torchaudio
import wave

_cosyvoice = CosyVoice2('CosyVoice2-0.5B', load_jit=False, load_trt=False, fp16=True)
#_cosyvoice = CosyVoice('CosyVoice-300M', load_jit=False, load_trt=False, fp16=False)

for step in range(10):
  prompt_speech_16k = load_wav('mda-qmwfy2k746929rxh.wav', 16000)
  _ori_text = "2024年，我们一起走过春夏秋冬，一道经历风雨彩虹，一个个瞬间定格在这不平凡的一年，令人感慨、难以忘怀。"
  text = '主席说我开飞机的水平很高'
  tts_result = _cosyvoice.inference_zero_shot(text, _ori_text, prompt_speech_16k, stream=False)
  rand_key = str(binascii.hexlify(os.urandom(8)), "utf-8")
  for i, j in enumerate(tts_result):
    result_path = '/tmp/test_{}_{}.wav'.format(rand_key, i)
    torchaudio.save(result_path, j['tts_speech'], _cosyvoice.sample_rate)


def parse_arguments():
  parser = argparse.ArgumentParser()
  parser.add_argument('--server_port', type=int, default=18000)
  return parser.parse_args()

class Searcher(RequestHandler):
  def post(self):
    #post_data = self.request.body
    #print(post_data)
    #try:
    tmp_path = "/tmp/wave_" + str(binascii.hexlify(os.urandom(8)), "utf-8")
    wave_path = tmp_path + ".wav"
    ori_text = self.get_argument('ori_text')
    #ori_text = "2024年，我们一起走过春夏秋冬，一道经历风雨彩虹，一个个瞬间定格在这不平凡的一年，令人感慨、难以忘怀。"
    text = self.get_argument('text')
    #text = '主席说我开飞机的水平很高'
    fileinfo = self.request.files['file'][0]
    filename = fileinfo['filename']
    filebody = fileinfo['body']
    print("Ori[%s] Text[%s] FileName[%s]" % (ori_text, text, filename))
    with open(tmp_path, 'wb') as f:
      f.write(filebody)
    os.system("/bin/ffmpeg -i %s -ar 16000 %s" % (tmp_path, wave_path))
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
    #prompt_speech_16k = load_wav("mda-qmwfy2k746929rxh.mp3", 16000)
    rand_key = str(binascii.hexlify(os.urandom(8)), "utf-8")
    tts_result = _cosyvoice.inference_zero_shot(text, ori_text, prompt_speech_16k, stream=False)
    result_name = 'output_{}.wav'.format(rand_key)
    concat_file = open("/tmp/concat.txt", "w")
    for i, j in enumerate(tts_result):
      sub_result_name = 'output_{}_{}.wav'.format(rand_key, i)
      torchaudio.save("/tmp/" + sub_result_name, j['tts_speech'], _cosyvoice.sample_rate)
      concat_file.write("file '/tmp/%s'\n" % sub_result_name)
    concat_file.close()
    cmd = "/bin/ffmpeg -f concat -safe 0 -i /tmp/concat.txt -c copy /tmp/%s" % (result_name)
    print(cmd)
    os.system(cmd)
    resp = "<HTML><A href='/download?filename=%s'>Download</HTML>" % result_name
    print(resp)
    self.write(resp)
    #except Exception as e:
    #  resp = json.dumps({"code":-1, "msg": str(e)}, ensure_ascii = False)
    #  print(resp)
    #  self.write(resp)
    #  return
  
  def get(self):
    body = '''
<form action="/tts" method="post" enctype="multipart/form-data" autocomplete="off">
  <input type="text" name="ori_text" placeholder="Ori Text"><BR/>
  <input type="file" name="file"><BR/>===<BR/>
  <input type="text" name="text" placeholder="Text"><BR/>
  <button type="submit">Upload</button><BR/>
</form>'''
    self.write(body)

class Downloader(RequestHandler):
  def get(self):
    buf_size = 4096
    filename = self.get_argument('filename', None)
    if not filename:
      self.write({"error": "Name Empty"})
      return
    self.set_header('Content-Disposition', 'attachment; filename='+filename)
    self.set_header('Content-Type', 'application/octet-stream')
    with open("/tmp/" + filename, 'rb') as f:
      while True:
        data = f.read(buf_size)
        if not data:
          break
        self.write(data)
    self.finish()

def make_app():
  return Application([
    (r"/tts", Searcher),
    (r"/download", Downloader),
  ])

if __name__ == "__main__":
  args = parse_arguments()
  app = make_app()
  app.listen(args.server_port, reuse_port = True)
  print("server start, port:", args.server_port)
  IOLoop.current().start()

# vim: set expandtab ts=4 sw=4 sts=4:
