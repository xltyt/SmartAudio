#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
import sys
import torch
from hyperpyyaml import load_hyperpyyaml
sys.path.append('pkg/third_party/Matcha-TTS')
sys.path.append('pkg/')

#torch._C._jit_set_profiling_mode(False)
#torch.jit.set_fusion_strategy([('STATIC', 0), ('DYNAMIC', 0)])
#torch._C._jit_debug_type_inference(True)

device = "cpu"
model_dir = "../../data/model_ori"
target_model_dir = "../../data/model"
os.system("mkdir -p %s" % target_model_dir)
hyper_yaml_path = 'cosyvoice.yaml'
if not os.path.exists(hyper_yaml_path):
  raise ValueError('{} not found!'.format(hyper_yaml_path))

with torch.no_grad():
  with open(hyper_yaml_path, 'r') as f:
    configs = load_hyperpyyaml(f)
  flow = configs["flow"]
  flow.half()
  flow.load_state_dict(torch.load("%s/flow.pt" % model_dir, map_location=device), strict=True)
  flow.to(device).eval()
  print("Load Finished")
  #script = torch.jit.optimize_for_inference(script)
  torch.jit.script(flow.encoder).save("%s/flow.encoder.fp16.jit" % target_model_dir)
  torch.jit.script(flow.length_regulator).save("%s/flow.length_regulator.fp16.jit" % target_model_dir)
  torch.jit.script(flow.decoder.estimator).save("%s/flow.estimator.fp16.jit" % target_model_dir)
  torch.jit.script(flow.decoder).save("%s/flow.decoder.fp16.jit" % target_model_dir)
  torch.jit.script(flow).save("%s/flow.fp16.jit" % target_model_dir)
  print("Save Finished")

with torch.no_grad():
  with open(hyper_yaml_path, 'r') as f:
    configs = load_hyperpyyaml(f)
  flow = configs["flow"]
  flow.load_state_dict(torch.load("%s/flow.pt" % model_dir, map_location=device), strict=True)
  flow.to(device).eval()
  print("Load Finished")
  #script = torch.jit.optimize_for_inference(script)
  torch.jit.script(flow.encoder).save("%s/flow.encoder.fp32.jit" % target_model_dir)
  torch.jit.script(flow.length_regulator).save("%s/flow.length_regulator.fp32.jit" % target_model_dir)
  torch.jit.script(flow.decoder.estimator).save("%s/flow.estimator.fp32.jit" % target_model_dir)
  torch.jit.script(flow.decoder).save("%s/flow.decoder.fp32.jit" % target_model_dir)
  torch.jit.script(flow).save("%s/flow.fp32.jit" % target_model_dir)
  print("Save Finished")
