/****************************************************\
 *
 * Copyright (C) 2025. All Rights Reserved
 * Last modified: 2025.04.17 17:52:17
 *
\****************************************************/

#ifndef _MODEL_H__
#define _MODEL_H__

#define C10_USE_GLOG
#include <torch/torch.h>
#include <torch/script.h>
#include <glog/logging.h>

class VoiceModel {
public:
  VoiceModel();
  virtual ~VoiceModel();

public:
  int load(const std::string& llm_model_path, const std::string& flow_model_path);
  int tts(
    const torch::Tensor& text,
    const torch::Tensor& text_len,
    const torch::Tensor& prompt_text,
    const torch::Tensor& prompt_text_len,
    const torch::Tensor& prompt_speech_token,
    const torch::Tensor& prompt_speech_token_len,
    const torch::Tensor& embedding
    );

public:
  int infer_flow(
		const torch::Tensor& token,
		const torch::Tensor& prompt_speech_token,
		const torch::Tensor& prompt_speech_feat,
		const torch::Tensor& embedding,
		float speed,
		torch::Tensor& tts_mel
	);

protected:
  std::unique_ptr<torch::jit::script::Module> _llm;
  std::unique_ptr<torch::jit::script::Module> _flow;
  torch::Tensor _flow_cache_dict;
};

#endif

/* vim: set expandtab nu ts=2 sw=2 sts=2: */
