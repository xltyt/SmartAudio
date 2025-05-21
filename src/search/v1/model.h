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
  int load(const std::string& llm_model_path);
  int tts(
    const torch::Tensor& text,
    const torch::Tensor& text_len,
    const torch::Tensor& prompt_text,
    const torch::Tensor& prompt_text_len,
    const torch::Tensor& prompt_speech_token,
    const torch::Tensor& prompt_speech_token_len,
    const torch::Tensor& embedding
    );

protected:
  std::unique_ptr<torch::jit::script::Module> _llm;
};

#endif

/* vim: set expandtab nu ts=2 sw=2 sts=2: */
