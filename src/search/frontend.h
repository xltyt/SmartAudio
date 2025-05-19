/****************************************************\
 *
 * Copyright (C) 2025. All Rights Reserved
 * Last modified: 2025.05.18 16:45:49
 *
\****************************************************/

#ifndef _FRONTEND_H__
#define _FRONTEND_H__

#include <string>
#include <vector>
#define C10_USE_GLOG
#include <torch/torch.h>
#include <glog/logging.h>
#include <onnxruntime_cxx_api.h>
#include "whisper_token.h"

class Frontend {
public:
  Frontend(
    const std::string& speech_tokenizer_model_path,
    const std::string& campplus_model_path 
    );
  virtual ~Frontend();
    
public:
  struct ZeroShotInput {
    torch::Tensor text;
    torch::Tensor text_len;
    torch::Tensor prompt_text;
    torch::Tensor prompt_text_len;
    torch::Tensor prompt_speech_token;
    torch::Tensor prompt_speech_token_len;
    torch::Tensor prompt_speech_feat;
    torch::Tensor prompt_speech_feat_len;
    torch::Tensor embedding;
  };
  int frontend_zero_shot(
    const std::string& tts_text,
    const std::string& prompt_text,
    const std::vector<float>& prompt_speech_16k,
    int sample_rate,
    ZeroShotInput& result
    );

public:
  std::pair<torch::Tensor, torch::Tensor> extract_text_token(const std::string& text);
  std::pair<torch::Tensor, torch::Tensor> extract_speech_feat(const torch::Tensor& speech);
  std::pair<torch::Tensor, torch::Tensor> extract_speech_token(const torch::Tensor& speech);
  torch::Tensor extract_spk_embedding(const torch::Tensor& speech);

protected:
  std::unique_ptr<WhisperToken> _tokenizer;
  std::unique_ptr<Ort::Session> _speech_tokenizer_session;
  std::unique_ptr<Ort::Env> _speech_tokenizer_env;
  std::unique_ptr<Ort::Session> _campplus_session;
  std::unique_ptr<Ort::Env> _campplus_env;
};

#endif

/* vim: set expandtab nu ts=2 sw=2 sts=2: */
