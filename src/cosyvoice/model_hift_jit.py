#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
import sys
import torch
from hyperpyyaml import load_hyperpyyaml
sys.path.append('pkg/third_party/Matcha-TTS')
sys.path.append('pkg/')

device = "cpu"
model_dir = "../../data/model_ori"
target_model_dir = "../../data/model"
os.system("mkdir -p %s" % target_model_dir)
hyper_yaml_path = './cosyvoice.yaml'
if not os.path.exists(hyper_yaml_path):
  raise ValueError('{} not found!'.format(hyper_yaml_path))

with torch.no_grad():
  with open(hyper_yaml_path, 'r') as f:
    configs = load_hyperpyyaml(f)
  hift = configs["hift"]
  hift.half()
  hift.load_state_dict(torch.load("%s/hift_no_weight.pt" % target_model_dir, map_location=device), strict=True)
  hift.to(device).eval()
  print("Load Finished")
  #script = torch.jit.optimize_for_inference(script)
  torch.jit.script(hift).save("%s/hift.fp16.jit" % target_model_dir)
  print("Save Finished")

with torch.no_grad():
  with open(hyper_yaml_path, 'r') as f:
    configs = load_hyperpyyaml(f)
  hift = configs["hift"]
  hift.load_state_dict(torch.load("%s/hift_no_weight.pt" % target_model_dir, map_location=device), strict=True)
  hift.to(device).eval()
  print("Load Finished")
  #script = torch.jit.optimize_for_inference(script)
  torch.jit.script(hift).save("%s/hift.fp32.jit" % target_model_dir)
  print("Save Finished")
