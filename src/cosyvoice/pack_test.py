#!/usr/bin/python
# -*- coding: utf-8 -*-

import torch
from typing import List, Tuple

@torch.jit.script
def einops_pack(tensors: List[torch.Tensor], pattern: str) -> Tuple[torch.Tensor, List[List[int]]]:
    # pattern: "b * t"
    # b = dim 0 (must match), t = last dim (must match)
    # flatten middle dims and concatenate along axis 1

    reshaped: List[torch.Tensor] = []
    packed_shapes: List[List[int]] = []

    for t in tensors:
        ndim = t.dim()
        # record middle dim sizes (axes 1 .. ndim-2)
        mid: List[int] = []
        for i in range(1, ndim - 1):
            mid.append(t.size(i))
        packed_shapes.append(mid)

        # reshape: (b, h*w*..., t)
        flat = 1
        for i in range(1, ndim - 1):
            flat = flat * t.size(i)
        reshaped.append(t.reshape(t.size(0), flat, t.size(ndim - 1)))

    result = torch.cat(reshaped, dim=1)
    return result, packed_shapes

tensor1 = torch.randn(2, 4, 3)
tensor2 = torch.randn(2, 5, 3)
packed, packed_shapes = einops_pack([tensor1, tensor2], 'b * t')
print(packed.shape)
print(packed_shapes)
print(packed)
print("===================")

from einops import pack
packed, packed_shapes = pack([tensor1, tensor2], 'b * t')
print(packed.shape)
print(packed_shapes)
print(packed)
