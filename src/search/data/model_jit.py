#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
import sys
import torch
from hyperpyyaml import load_hyperpyyaml
sys.path.append('/data/cosyvoice/CosyVoice/third_party/Matcha-TTS')
sys.path.append('/data/cosyvoice/CosyVoice')

device = "cpu"
model_dir = "./model"
hyper_yaml_path = '{}/cosyvoice.yaml'.format(model_dir)
if not os.path.exists(hyper_yaml_path):
  raise ValueError('{} not found!'.format(hyper_yaml_path))

with torch.no_grad():
  with open(hyper_yaml_path, 'r') as f:
    configs = load_hyperpyyaml(f)
  llm = configs["llm"]
  llm.half()
  llm.load_state_dict(torch.load("./model/llm.pt", map_location=device), strict=True)
  llm.to(device).eval()
  #llm.text_encoder = torch.jit.load("./model/llm.text_encoder.fp16.zip", map_location=device)
  #llm.llm = torch.jit.load("./model/llm.llm.fp16.zip", map_location=device)
  print("Load Finished")
  script = torch.jit.script(llm)
  #script = torch.jit.optimize_for_inference(script)
  script.save("./model/llm.jit.fp16.pt")
  print("Save Finished")


with torch.no_grad():
  with open(hyper_yaml_path, 'r') as f:
    configs = load_hyperpyyaml(f)
  llm = configs["llm"]
  llm.load_state_dict(torch.load("./model/llm.pt", map_location=device), strict=True)
  llm.to(device).eval()
  #llm.text_encoder = torch.jit.load("./model/llm.text_encoder.fp32.zip", map_location=device)
  #llm.llm = torch.jit.load("./model/llm.llm.fp32.zip", map_location=device)
  print("Load Finished")
  script = torch.jit.script(llm)
  #script = torch.jit.optimize_for_inference(script)
  script.save("./model/llm.jit.fp32.pt")
  print("Save Finished")
