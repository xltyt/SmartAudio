jit编译说明

LLM模型
代码Patch: llm.patch, 路径: pkg
转换脚本: model_llm_jit.py

Flow模型
依赖的diffusers(版本0.29.0)默认无法支持jit，需要使用diffusers_0.29.0.patch对代码进行修改, 路径: /data/local/conda/envs/audio_ori/lib/python3.10/site-packages/diffusers
matcha.patch，路径: pkg/third_party/Matcha-TTS
代码Patch: flow.patch, 路径: pkg
转换脚本: model_flow_jit.py

Hift模型
先进行权重融合然后再Patch&转换: model_hift_pt.py
代码Patch: hift.patch, 路径: pkg
转换脚本: model_hift_jit.py
