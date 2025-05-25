#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
import sys
import torch
from hyperpyyaml import load_hyperpyyaml
sys.path.append('pkg/third_party/Matcha-TTS')
sys.path.append('pkg/')
import logging
logging.getLogger('torch.jit').setLevel(logging.DEBUG)

device = "cpu"
model_dir = "../../data/model"
target_model_dir = "../../model"
os.system("mkdir -p %s" % target_model_dir)
hyper_yaml_path = 'cosyvoice.yaml'
if not os.path.exists(hyper_yaml_path):
  raise ValueError('{} not found!'.format(hyper_yaml_path))

with torch.no_grad():
  with open(hyper_yaml_path, 'r') as f:
    configs = load_hyperpyyaml(f)
  llm = configs["llm"]
  llm.half()
  llm.load_state_dict(torch.load("%s/llm.pt" % model_dir, map_location=device), strict=True)
  llm.to(device).eval()
  #llm.text_encoder = torch.jit.load("./model/llm.text_encoder.fp16.zip", map_location=device)
  #llm.llm = torch.jit.load("./model/llm.llm.fp16.zip", map_location=device)
  print("Load Finished")
  script = torch.jit.script(llm)
  #script = torch.jit.optimize_for_inference(script)
  script.save("%s/llm.fp16.jit" % target_model_dir)
  print("Save Finished")


with torch.no_grad():
  with open(hyper_yaml_path, 'r') as f:
    configs = load_hyperpyyaml(f)
  llm = configs["llm"]
  llm.load_state_dict(torch.load("%s/llm.pt" % model_dir, map_location=device), strict=True)
  llm.to(device).eval()
  #llm.text_encoder = torch.jit.load("./model/llm.text_encoder.fp32.zip", map_location=device)
  #llm.llm = torch.jit.load("./model/llm.llm.fp32.zip", map_location=device)
  print("Load Finished")
  script = torch.jit.script(llm)
  #script = torch.jit.optimize_for_inference(script)
  script.save("%s/llm.fp32.jit" % target_model_dir)
  print("Save Finished")
