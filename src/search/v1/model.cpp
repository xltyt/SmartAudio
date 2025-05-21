/****************************************************\
 *
 * Copyright (C) 2025. All Rights Reserved
 * Last modified: 2025.04.17 17:52:38
 *
\****************************************************/

#include "model.h"

VoiceModel::VoiceModel() {
}

VoiceModel::~VoiceModel() {
  _llm.reset();
}
  
int VoiceModel::load(const std::string& llm_model_path) {
  LOG(INFO) << "VoiceModel::load LLM Init";
  {
    //_llm = std::make_unique<torch::nn::Module>();
    //_llm->to(torch::kCPU);
    //torch::serialize::InputArchive input_archive;
    //input_archive.load_from(llm_model_path, torch::kCPU);
    //_llm->load(input_archive);
    //_llm->to(torch::kCPU);
    //_llm->eval();
    torch::jit::script::Module llm = torch::jit::load(llm_model_path, torch::kCPU);
    _llm = std::make_unique<torch::jit::script::Module>(llm);
    _llm->eval();
  }
  LOG(INFO) << "VoiceModel::load LLM End";
  
  return 0;
}
  
int VoiceModel::tts(
  const torch::Tensor& text,
  const torch::Tensor& text_len,
  const torch::Tensor& prompt_text,
  const torch::Tensor& prompt_text_len,
  const torch::Tensor& prompt_speech_token,
  const torch::Tensor& prompt_speech_token_len,
  const torch::Tensor& embedding
  ) {
  std::vector<torch::jit::IValue> inputs;
  inputs.push_back(text.detach());
  inputs.push_back(torch::tensor({text.size(1)}, torch::kInt32).detach());
  inputs.push_back(prompt_text.detach());
  inputs.push_back(torch::tensor({prompt_text.size(1)}, torch::kInt32).detach());
  inputs.push_back(prompt_speech_token.detach());
  inputs.push_back(torch::tensor({prompt_speech_token.size(1)}, torch::kInt32).detach());
  inputs.push_back(embedding.detach());
  inputs.push_back(25);          // sampling
  inputs.push_back(20.0);        // max_token_text_ratio
  inputs.push_back(2.0);         // min_token_text_ratio
  std::vector<int64_t> output_tensor = _llm->get_method("inference")(inputs).toIntVector();
	for (auto _ : output_tensor) {
    LOG(INFO) << _;
  }
  return 0;
}

/* vim: set expandtab nu ts=2 sw=2 sts=2: */
