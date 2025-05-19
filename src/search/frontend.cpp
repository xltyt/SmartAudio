/****************************************************\
 *
 * Copyright (C) 2025. All Rights Reserved
 * Last modified: 2025.05.18 19:38:36
 *
\****************************************************/

#include "frontend.h"
#include "frontend_audio_utils.h"
#include <kaldi-native-fbank/csrc/online-feature.h>
#include <kaldi-native-fbank/csrc/feature-fbank.h>

Frontend::Frontend(
  const std::string& speech_tokenizer_model_path,
  const std::string& campplus_model_path 
  ) {
  LOG(INFO) << "Frontend::Frontend Init Token";
  _tokenizer = std::make_unique<WhisperToken>(".");
  // Speech Token Model
  LOG(INFO) << "Frontend::Frontend Init Speech Token";
  {
    Ort::SessionOptions option;
    option.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
    option.SetIntraOpNumThreads(16);
#if 0
    if (torch::cuda::is_available()) {
      // 旧版 API: OrtSessionOptionsAppendExecutionProvider_CUDA(option, 0);
      // 新版 API (ORT >= 1.12):
      OrtCUDAProviderOptionsV2 cuda_options;
      cuda_options.device_id = 0;
      option.AppendExecutionProvider_CUDA(cuda_options);
    }
#endif
    _speech_tokenizer_env = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_WARNING, "speech_tokenizer");
    _speech_tokenizer_session = std::make_unique<Ort::Session>(*_speech_tokenizer_env.get(), speech_tokenizer_model_path.c_str(), option);
  }
  // Campplus Model
  LOG(INFO) << "Frontend::Frontend Init Campplus Token";
  {
    Ort::SessionOptions option;
    option.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
    option.SetIntraOpNumThreads(16);
    _campplus_env = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_VERBOSE, "campplus");
    _campplus_session = std::make_unique<Ort::Session>(*_campplus_env.get(), campplus_model_path.c_str(), option);
  }
  LOG(INFO) << "Frontend::Frontend End";
}

Frontend::~Frontend() {
  _tokenizer.reset();
  _speech_tokenizer_session.reset();
  _speech_tokenizer_env.reset();
  _campplus_session.reset();
  _campplus_env.reset();
}

int Frontend::frontend_zero_shot(
  const std::string& tts_text,
  const std::string& prompt_text,
  const std::vector<float>& prompt_speech_16k,
  int sample_rate,
  ZeroShotInput& result
  ) {
  auto [tts_text_token, tts_text_token_len] = extract_text_token(tts_text);
  auto [prompt_text_token, prompt_text_token_len] = extract_text_token(prompt_text);
  int channels = 0;
  std::vector<float> prompt_speech_resample = ::resample(prompt_speech_16k, 16000, sample_rate, channels);
  
  auto prompt_speech_resample_tensor = torch::zeros({1, (int)prompt_speech_resample.size()}, torch::kFloat32);
  for (int i = 0; i < (int)prompt_speech_resample.size(); ++i) {
    prompt_speech_resample_tensor[0][i] = prompt_speech_resample[i];
  }
  auto [speech_feat, speech_feat_len] = ::extract_speech_feat(prompt_speech_resample_tensor);
  
  auto prompt_speech_16k_tensor = torch::zeros({1, (int)prompt_speech_16k.size()}, torch::kFloat32);
  for (int i = 0; i < (int)prompt_speech_16k.size(); ++i) {
    prompt_speech_16k_tensor[0][i] = prompt_speech_16k[i];
  }
  auto [speech_token, speech_token_len] = extract_speech_token(prompt_speech_16k_tensor);
  
  auto spk_embedding = extract_spk_embedding(prompt_speech_16k_tensor);
  
  result.text = tts_text_token;
  result.text_len = tts_text_token_len;
  result.prompt_text = prompt_text_token;
  result.prompt_text_len = prompt_text_token_len;
  result.prompt_speech_token = speech_token;
  result.prompt_speech_token_len = speech_token_len;
  result.prompt_speech_feat = speech_feat;
  result.prompt_speech_feat_len = speech_feat_len;
  result.embedding = spk_embedding;
  
  return 0;
}
  
std::pair<torch::Tensor, torch::Tensor> Frontend::extract_text_token(const std::string& text) {
  /*
  text_token = self.tokenizer.encode(text, allowed_special=self.allowed_special)
  text_token = torch.tensor([text_token], dtype=torch.int32).to(self.device)
  text_token_len = torch.tensor([text_token.shape[1]], dtype=torch.int32).to(self.device)
  return text_token, text_token_len
  */
  
  std::vector<size_t> token_int_ids = _tokenizer->encode(text, std::unordered_set<std::string>({"all"}));
  std::vector<int> token_ids;
  for (auto _ : token_int_ids) {
    token_ids.push_back(_);
  }

  // 将 token ids 转为形状为 (1, seq_len) 的 int32 tensor，并移到指定设备
  auto text_token = torch::tensor(token_ids, torch::dtype(torch::kInt32)).unsqueeze(0);
    
  // 序列长度作为标量 tensor (shape [1])，int32，移到相同设备
  auto text_token_len = torch::tensor(static_cast<int>(token_ids.size()), torch::dtype(torch::kInt32));
    
  return {text_token, text_token_len};
}
  
std::pair<torch::Tensor, torch::Tensor> Frontend::extract_speech_token(const torch::Tensor& speech) {
  torch::Tensor feat = log_mel_spectrogram(speech, 128);
#if 0
  speech_token = self.speech_tokenizer_session.run(None,
                                                   {self.speech_tokenizer_session.get_inputs()[0].name:
                                                    feat.detach().cpu().numpy(),
                                                    self.speech_tokenizer_session.get_inputs()[1].name:
                                                    np.array([feat.shape[2]], dtype=np.int32)})[0].flatten().tolist()
  speech_token = torch.tensor([speech_token], dtype=torch.int32).to(self.device)
  speech_token_len = torch.tensor([speech_token.shape[1]], dtype=torch.int32).to(self.device)
  return speech_token, speech_token_len
#endif

  auto memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

  // Ort Tensor
  torch::Tensor feat_cpu = feat.detach().to(torch::kCPU).contiguous();
  std::vector<int64_t> feat_shape(feat_cpu.sizes().begin(), feat_cpu.sizes().end());
  LOG(INFO) << feat_cpu.numel();
  Ort::Value feat_ort = Ort::Value::CreateTensor<float>(
      memory_info,
      feat_cpu.data_ptr<float>(),
      feat_cpu.numel(),
      feat_shape.data(),
      feat_shape.size()
      );

  // Ort Tensor Len
  //int32_t feat_len_val = static_cast<int32_t>(feat.size(2));
  //std::vector<int64_t> len_shape = {1};
  //Ort::Value feat_len_ort = Ort::Value::CreateTensor<int32_t>(
  //  memory_info,
  //  &feat_len_val,
  //  1,
  //  len_shape.data(),
  //  len_shape.size()
  //  );
  std::vector<int32_t> feat_len_data = { static_cast<int32_t>(feat.size(2)) };
  std::vector<int64_t> feat_len_shape = { 1 };
  Ort::Value feat_len_ort = Ort::Value::CreateTensor<int32_t>(
    memory_info,
    feat_len_data.data(),
    feat_len_data.size(),
    feat_len_shape.data(),
    feat_len_shape.size()
    );

  // Input Names
  std::vector<Ort::AllocatedStringPtr> input_name_ptrs;
  std::vector<const char*> input_names;
  for (size_t i = 0; i < _speech_tokenizer_session->GetInputCount(); i++) {
    auto ptr = _speech_tokenizer_session->GetInputNameAllocated(i, Ort::AllocatorWithDefaultOptions());
    input_names.push_back(ptr.get());
    input_name_ptrs.push_back(std::move(ptr));
  }

  // Output Names
  std::vector<Ort::AllocatedStringPtr> output_name_ptrs;
  std::vector<const char*> output_names;
  for (size_t i = 0; i < _speech_tokenizer_session->GetOutputCount(); i++) {
    auto ptr = _speech_tokenizer_session->GetOutputNameAllocated(i, Ort::AllocatorWithDefaultOptions());
    output_names.push_back(ptr.get());
    output_name_ptrs.push_back(std::move(ptr));
  }

  std::vector<Ort::Value> inputs;
  inputs.push_back(std::move(feat_ort));
  inputs.push_back(std::move(feat_len_ort));

  // Run
  LOG(INFO) << "extract_speech_token ONNX Run Start...";
  auto outputs = _speech_tokenizer_session->Run(
    Ort::RunOptions{nullptr},
    input_names.data(),
    inputs.data(),
    inputs.size(),
    output_names.data(),
    output_names.size()
    );
  LOG(INFO) << "extract_speech_token ONNX Run End";

  // Process
  auto& out_tensor = outputs[0];
  const Ort::TensorTypeAndShapeInfo& type_info = out_tensor.GetTensorTypeAndShapeInfo();
  //for (auto _ : type_info.GetShape()) {
  //  LOG(INFO) << _;
  //}
  assert(type_info.GetElementType() == ONNX_TENSOR_ELEMENT_DATA_TYPE_INT64);

  const int64_t* out_data = out_tensor.GetTensorData<int64_t>();
  size_t out_count = out_tensor.GetTensorTypeAndShapeInfo().GetElementCount();

  // flatten → shape [1, N]
  std::vector<int64_t> token_vec(out_data, out_data + out_count);
  //LOG(INFO) << "Out Len[" << token_vec << "]";
  //for (auto _ : token_vec) {
  //  LOG(INFO) << _;
  //}
  torch::Tensor speech_token = torch::tensor(token_vec, torch::kInt32).reshape({1, -1});//.to(device);

  // Len
  torch::Tensor speech_token_len = torch::tensor({speech_token.size(1)}, torch::kInt32);//.to(device);

  return {speech_token, speech_token_len};
}

torch::Tensor Frontend::extract_spk_embedding(const torch::Tensor& speech) {
  auto fbank_tensor = kaldi_fbank(speech);
  fbank_tensor = fbank_tensor - fbank_tensor.mean(0, true);
  fbank_tensor = fbank_tensor.unsqueeze(0).contiguous();

  auto memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
  
  std::vector<int64_t> feat_shape(fbank_tensor.sizes().begin(), fbank_tensor.sizes().end());
  //for (auto _ : feat_shape) {
  //  LOG(INFO) << _;
  //}
  Ort::Value feat_ort = Ort::Value::CreateTensor<float>(
      memory_info,
      fbank_tensor.data_ptr<float>(),
      fbank_tensor.numel(),
      feat_shape.data(),
      feat_shape.size()
      );

  // Input Names
  std::vector<Ort::AllocatedStringPtr> input_name_ptrs;
  std::vector<const char*> input_names;
  for (size_t i = 0; i < _campplus_session->GetInputCount(); i++) {
    auto ptr = _campplus_session->GetInputNameAllocated(i, Ort::AllocatorWithDefaultOptions());
    input_names.push_back(ptr.get());
    input_name_ptrs.push_back(std::move(ptr));
  }

  // Output Names
  std::vector<Ort::AllocatedStringPtr> output_name_ptrs;
  std::vector<const char*> output_names;
  for (size_t i = 0; i < _campplus_session->GetOutputCount(); i++) {
    auto ptr = _campplus_session->GetOutputNameAllocated(i, Ort::AllocatorWithDefaultOptions());
    output_names.push_back(ptr.get());
    output_name_ptrs.push_back(std::move(ptr));
  }
  
  std::vector<Ort::Value> inputs;
  inputs.push_back(std::move(feat_ort));

  // Run
  auto outputs = _campplus_session->Run(
    Ort::RunOptions{nullptr},
    input_names.data(),
    inputs.data(),
    inputs.size(),
    output_names.data(),
    output_names.size()
    );
  
  // Process
  auto& out_tensor = outputs[0];
  const Ort::TensorTypeAndShapeInfo& type_info = out_tensor.GetTensorTypeAndShapeInfo();
  assert(type_info.GetElementType() == ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT);

  const float* out_data = out_tensor.GetTensorData<float>();
  size_t out_count = out_tensor.GetTensorTypeAndShapeInfo().GetElementCount();
  
  std::vector<float> embedding_cpu_vec(out_data, out_data + out_count);
  torch::Tensor embedding_cpu = torch::tensor(embedding_cpu_vec, torch::kFloat32);
  return embedding_cpu.unsqueeze(0);//.to(device);;
}

/* vim: set expandtab nu ts=2 sw=2 sts=2: */
