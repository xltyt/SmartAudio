#!/usr/bin/python
# -*- coding: utf-8 -*-

#import torch
#from torch import nn,Tensor
#import torch.nn.functional as F
#from typing import Dict,Tuple,Optional,List
#
#class Test(nn.Module):
#    def __init__(self) -> None:
#        super().__init__()
#    
#    def forward(self, a:Tensor) -> Tuple[Optional[Tensor]]:
#        return (a,a,None)
#
#
#test_model = torch.jit.script(Test())

import torch
from torch import nn,Tensor
import torch.nn.functional as F
from typing import Dict,Tuple,Optional,List

class Test(nn.Module):
    def __init__(self) -> None:
        super().__init__()
    
    def forward(self, a:Tensor) -> Tuple[Optional[Tensor], Optional[Tensor], Optional[Tensor]]:
        return (a,a,None)


test_model = torch.jit.script(Test())
