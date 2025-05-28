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
  _flow.reset();
  _hift.reset();
}
  
int VoiceModel::load(const std::string& llm_model_path, const std::string& flow_model_path, const std::string& hift_model_path) {
  LOG(INFO) << "VoiceModel::load LLM Init";
  if (torch::cuda::is_available()) {
    LOG(INFO) << "VoiceModel::load CUDA Available";
  }
  else {
    LOG(INFO) << "VoiceModel::load CUDA Not Available";
  }
  
  torch::autograd::GradMode::set_enabled(false);
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
#if USE_GPU
    _llm->to(torch::kCUDA);
#endif
    _llm->eval();
  }
  LOG(INFO) << "VoiceModel::load LLM End";
  
  LOG(INFO) << "VoiceModel::load FLow Init";
  {
    torch::jit::script::Module flow = torch::jit::load(flow_model_path, torch::kCPU);
    _flow = std::make_unique<torch::jit::script::Module>(flow);
#if USE_GPU
    _flow->to(torch::kCUDA);
#endif
    _flow->eval();
    _flow_cache_dict = torch::zeros({1, 80, 0, 2}, torch::kFloat32);
  }
  LOG(INFO) << "VoiceModel::load Flow End";

  LOG(INFO) << "VoiceModel::load Hift Init";
  {
    torch::jit::script::Module hift = torch::jit::load(hift_model_path, torch::kCPU);
    _hift = std::make_unique<torch::jit::script::Module>(hift);
    _hift->eval();
    _hift_cache_source = torch::zeros({1, 1, 0}, torch::kFloat32);
  }
  LOG(INFO) << "VoiceModel::load Hift End";
  
  return 0;
}
  
int VoiceModel::tts(
  const torch::Tensor& text,
  const torch::Tensor& prompt_text,
  const torch::Tensor& prompt_speech_token,
  const torch::Tensor& prompt_speech_feat,
  const torch::Tensor& embedding,
  float speed,
  std::vector<float>& wav_data
  ) {
	std::vector<int64_t> output_ids;
  try {
    infer_llm(
      text,
      prompt_text,
      prompt_speech_token,
      embedding,
      output_ids
      );
  }
	catch (const std::exception& e) {
    LOG(WARNING) << "VoiceModel::tts Infer LLM Failed[" << e.what() << "]";
    return -1;
	}
  torch::Tensor tts_mel;
  try {
    infer_flow(
      torch::tensor(output_ids, torch::kInt32).to(text.device()),
      prompt_speech_token,
      prompt_speech_feat,
      embedding,
      speed,
      tts_mel
    );
  }
	catch (const std::exception& e) {
    LOG(WARNING) << "VoiceModel::tts Infer Flow Failed[" << e.what() << "]";
    return -1;
	}
  torch::Tensor tts_speech;
	torch::Tensor tts_source;
  try {
    infer_hift(
      tts_mel.to(torch::kCPU),
      tts_speech,
      tts_source
      );
  }
	catch (const std::exception& e) {
    LOG(WARNING) << "VoiceModel::tts Infer Hift Failed[" << e.what() << "]";
    return -1;
	}
  if (tts_speech.dim() != 2 || tts_speech.sizes()[0] != 1) {
    LOG(WARNING) << "VoiceModel::tts Infer Hift Return Invalid";
    return -1;
  }
  float *tts_speech_data = tts_speech[0].to(torch::kCPU).contiguous().data_ptr<float>();
	wav_data = std::vector<float>(tts_speech_data, tts_speech_data + tts_speech[0].numel());
  return 0;
}

int VoiceModel::infer_llm(
  const torch::Tensor& text,
  const torch::Tensor& prompt_text,
  const torch::Tensor& prompt_speech_token,
  const torch::Tensor& embedding,
	std::vector<int64_t>& output
  ) {
  std::vector<torch::jit::IValue> inputs;
  inputs.push_back(text.detach());
  inputs.push_back(torch::tensor({text.size(1)}, torch::kInt32).to(text.device()).detach());
  inputs.push_back(prompt_text.detach());
  inputs.push_back(torch::tensor({prompt_text.size(1)}, torch::kInt32).to(prompt_text.device()).detach());
  inputs.push_back(prompt_speech_token.detach());
  inputs.push_back(torch::tensor({prompt_speech_token.size(1)}, torch::kInt32).to(prompt_speech_token.device()).detach());
  inputs.push_back(embedding.detach());
  inputs.push_back(25);          // sampling
  inputs.push_back(20.0);        // max_token_text_ratio
  inputs.push_back(2.0);         // min_token_text_ratio
  //torch::NoGradGuard no_grad;
  output = _llm->get_method("inference")(inputs).toIntVector();
  return 0;
}
  
int VoiceModel::infer_flow(
  const torch::Tensor& token,
  const torch::Tensor& prompt_speech_token,
  const torch::Tensor& prompt_speech_feat,
  const torch::Tensor& embedding,
	float speed,
	torch::Tensor& tts_mel
	) {
  torch::Tensor this_tts_speech_token = torch::Tensor(token).unsqueeze(0);
	std::vector<torch::jit::IValue> inputs;
  inputs.push_back(this_tts_speech_token.detach());
  inputs.push_back(torch::tensor({this_tts_speech_token.size(1)}, torch::kInt32).to(token.device()).detach());
  inputs.push_back(prompt_speech_token.detach());
  inputs.push_back(torch::tensor({prompt_speech_token.size(1)}, torch::kInt32).to(prompt_speech_token.device()).detach());
  inputs.push_back(prompt_speech_feat.detach());
  inputs.push_back(torch::tensor({prompt_speech_token.size(1)}, torch::kInt32).to(prompt_speech_feat.device()).detach());
  inputs.push_back(embedding.to(token.device()).detach());
  inputs.push_back(_flow_cache_dict.to(token.device()).detach());
  //torch::NoGradGuard no_grad;
  auto output_elements = _flow->get_method("inference")(inputs).toTuple()->elements();
  tts_mel = output_elements[0].toTensor();
  //_flow_cache_dict = output_elements[1].toTensor();
  return 0;
}

int VoiceModel::infer_hift(
	const torch::Tensor& speech_feat,
	torch::Tensor& tts_speech,
	torch::Tensor& tts_source
  ) {
	std::vector<torch::jit::IValue> inputs;
  inputs.push_back(speech_feat.detach());
  inputs.push_back(_hift_cache_source.to(speech_feat.device()).detach());
  auto output_elements = _hift->get_method("inference")(inputs).toTuple()->elements();
  tts_speech = output_elements[0].toTensor();
  //_hift_cache_source = output_elements[1].toTensor();
	return 0;
}

/* vim: set expandtab nu ts=2 sw=2 sts=2: */
