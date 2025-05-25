#!/usr/bin/python
# -*- coding: utf-8 -*-

import numpy as np
import torch

data = np.load("whisper/mel_filters.npz")
for key in data.keys():
  print(f"Process{key}...")
  np_array = data[key]
  tensor = torch.from_numpy(np_array)
  torch.save(tensor, f"whisper/mel_filters_{key}.pt")
  np_array.tofile(f"whisper/mel_filters_{key}.bin")
