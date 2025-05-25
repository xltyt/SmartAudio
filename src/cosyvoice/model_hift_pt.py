#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
import sys
import torch
from hyperpyyaml import load_hyperpyyaml
sys.path.append('pkg/third_party/Matcha-TTS')
sys.path.append('pkg/')

device = "cpu"
model_dir = "../../data/model"
target_model_dir = "../../model"
os.system("mkdir -p %s" % target_model_dir)
hyper_yaml_path = 'cosyvoice.yaml'
if not os.path.exists(hyper_yaml_path):
  raise ValueError('{} not found!'.format(hyper_yaml_path))

def fuse_weight_norm(model):
  for module in model.modules():
    # 检查该模块是否有 parametrizations 属性
    if hasattr(module, 'parametrizations'):
      # 获取所有被参数化的张量名称（通常是 'weight'）
      for name in list(module.parametrizations.keys()):
        # remove_parametrizations 会：
        # 1. 计算当前的归一化权重 W = g * v / ||v||
        # 2. 将 W 赋值给 module.weight
        # 3. 删除 module.parametrizations 和额外的 g/v 参数
        torch.nn.utils.parametrize.remove_parametrizations(module, name)
  return model

with torch.no_grad():
  # Remove parametrizations
  with open(hyper_yaml_path, 'r') as f:
    configs = load_hyperpyyaml(f)
  hift = configs["hift"]
  hift.half()
  hift.load_state_dict(torch.load("%s/hift.pt" % model_dir, map_location=device), strict=True)
  hift.to(device).eval()
  hift = fuse_weight_norm(hift)
  torch.save(hift.state_dict(), "%s/hift.fp16.pt" % target_model_dir)

with torch.no_grad():
  # Remove parametrizations
  with open(hyper_yaml_path, 'r') as f:
    configs = load_hyperpyyaml(f)
  hift = configs["hift"]
  hift.load_state_dict(torch.load("%s/hift.pt" % model_dir, map_location=device), strict=True)
  hift.to(device).eval()
  hift = fuse_weight_norm(hift)
  torch.save(hift.state_dict(), "%s/hift.fp32.pt" % target_model_dir)

