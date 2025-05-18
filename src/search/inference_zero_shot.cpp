/****************************************************\
 *
 * Copyright (C) 2025. All Rights Reserved
 * Last modified: 2025.05.15 16:26:02
 *
\****************************************************/

#include "inference_zero_shot.h"
#include <future>
#include <glog/logging.h>
#include <string_utils.h>
#include "frontend.h"
#include "frontend_utils.h"

InferenceZeroShot::InferenceZeroShot(const std::string& dir, bool is_fp16 /*= true*/) {
  LOG(INFO) << "InferenceZeroShot::InferenceZeroShot Start";
  const std::string campplus_path = mycommon::str_format("%s/campplus.onnx", dir.c_str());
  const std::string token_path = mycommon::str_format("%s/speech_tokenizer_v1.onnx", dir.c_str());
  const std::string spk_path = mycommon::str_format("%s/spk2info.pt", dir.c_str());
  _sample_rate = 22050;
  LOG(INFO) << "InferenceZeroShot::InferenceZeroShot Init Frontend";
  _frontend = std::make_unique<Frontend>(token_path);
  LOG(INFO) << "InferenceZeroShot::InferenceZeroShot Init Token";
  _tokenizer = std::make_unique<WhisperToken>(".");
  LOG(INFO) << "InferenceZeroShot::InferenceZeroShot End";
}

InferenceZeroShot::~InferenceZeroShot() {
  _tokenizer.reset();
  _frontend.reset();
}

std::vector<float> InferenceZeroShot::inference_zero_shot(
  const std::string& tts_text,
  const std::string& prompt_text,
  const std::vector<float>& prompt_speech_16k,
  bool stream /*= false*/,
  float speed /*= 1.0*/,
	bool text_frontend /*= true*/
	) {
  std::vector<float> result;
  auto prompt_result = text_normalize(_tokenizer.get(), std::unordered_set<std::string>({"all"}), prompt_text, false, text_frontend);
  auto *prompt_str = std::get_if<std::string>(&prompt_result);
  if (NULL == prompt_str) {
    LOG(INFO) << "InferenceZeroShot::inference_zero_shot Process Prompt Failed";
    return result;
  }
    
  auto text_result = text_normalize(_tokenizer.get(), std::unordered_set<std::string>({"all"}), tts_text, true, text_frontend);
  auto *text_vec = std::get_if<std::vector<std::string>>(&text_result);
  if (NULL == text_vec) {
    LOG(INFO) << "InferenceZeroShot::inference_zero_shot Process Text Failed";
    return result;
  }

  auto job_results = std::vector<std::future<int>>();
  for (int i = 0; i < (int)text_vec->size(); i++) {
    const std::string& text = text_vec->at(i);
    job_results.push_back(std::async(std::launch::async, [&]() -> int {
      if (text.size() < 0.5 * prompt_text.size()) {
        LOG(INFO) << "synthesis text " << text << " too short than prompt text " << prompt_text << ", this may lead to bad performance";
      }
      Frontend::ZeroShotInput infer_result;
      int ret = _frontend->frontend_zero_shot(text, prompt_text, prompt_speech_16k, _sample_rate, infer_result);
      return ret;
    }));
  }
  
  std::vector<int> job_ret;
  for (auto &job_result : job_results) {
    job_ret.push_back(job_result.get());
  }
  for (auto &_ : job_ret) {
    if (_ != 0) {
      LOG(INFO) << "InferenceZeroShot::inference_zero_shot Run Infer Failed[" << _ << "]";
      return result;
    }
  }
	
  return result;
}

/* vim: set expandtab nu ts=2 sw=2 sts=2: */
