/****************************************************\
 *
 * Copyright (C) 2025. All Rights Reserved
 * Last modified: 2025.05.15 16:26:02
 *
\****************************************************/

#include "inference_zero_shot.h"
#include <future>
#include <signal.h>
#include <glog/logging.h>
#include <string_utils.h>
#include "frontend.h"
#include "frontend_utils.h"
#include "frontend_audio_utils.h"

#define REQ_AND_WAIT \
  sem_post(&_req_sem); \
  sem_wait(&_resp_sem)

InferenceZeroShot::InferenceZeroShot(
  const std::string& dir,
  bool is_fp16 /*= true*/
  ) {  
	_dir = dir;
  _is_fp16 = is_fp16;
  sem_init(&_req_sem, 0, 0);
  sem_init(&_resp_sem, 0, 0);
  if (pthread_create(&_run_thread_id, NULL, _run_thread, this)) {
    fprintf(stderr, "pthread_create Failed, Suicided...\n");
    sync();
    kill(getpid(), SIGKILL);
  }
}

InferenceZeroShot::~InferenceZeroShot() {
  LOG(INFO) << "InferenceZeroShot::~InferenceZeroShot Start";
  _want_exit = true;
	sem_post(&_req_sem);
  pthread_join(_run_thread_id, NULL);
  _run_thread_id = 0;
  sem_destroy(&_req_sem);
  sem_destroy(&_resp_sem);
  LOG(INFO) << "InferenceZeroShot::~InferenceZeroShot End";
}

std::vector<float> InferenceZeroShot::inference_zero_shot(
  const std::string& tts_text,
  const std::string& prompt_text,
  const std::vector<float>& prompt_speech_16k,
  bool stream /*= false*/,
  float speed /*= 1.0*/,
	bool text_frontend /*= true*/
	) {
  common::ScopedLockW<common::RWLock> lock(_run_lock);
  _tts_text = tts_text;
  _prompt_text = prompt_text;
  _prompt_speech_16k = prompt_speech_16k;
  _stream = stream;
  _speed = speed;
  _text_frontend = text_frontend;
  REQ_AND_WAIT;
  return _result_wav_data;
}

void* InferenceZeroShot::_run_thread(void *arg) {
  InferenceZeroShot *pthis = (InferenceZeroShot *)arg;
  pthis->run();
  pthread_exit(0);
  return NULL;
}

void InferenceZeroShot::run() {
  LOG(INFO) << "InferenceZeroShot::run Start";
  _sample_rate = 22050;
  LOG(INFO) << "InferenceZeroShot::run Init Frontend";
  _frontend = std::make_unique<Frontend>(_dir);
  LOG(INFO) << "InferenceZeroShot::run Init Token";
  _tokenizer = std::make_unique<WhisperToken>(_dir);
  // Model
  LOG(INFO) << "InferenceZeroShot::run Init Model";
  std::string llm_model_path = mycommon::str_format("%s/model/llm.fp%d.jit", _dir.c_str(), _is_fp16 ? 16 : 32);
  std::string flow_model_path = mycommon::str_format("%s/model/flow.fp%d.jit", _dir.c_str(), _is_fp16 ? 16 : 32);
  std::string hift_model_path = mycommon::str_format("%s/model/hift.fp%d.jit", _dir.c_str(), _is_fp16 ? 16 : 32);
  _model = std::make_unique<VoiceModel>();
  _model->load(llm_model_path, flow_model_path, hift_model_path);
  LOG(INFO) << "InferenceZeroShot::run End";
  
  while (!_want_exit) {
    sem_wait(&_req_sem);
    if (_tts_text.empty()) {
      continue;
    }
    
    std::vector<float> result;
    auto prompt_result = text_normalize(_tokenizer.get(), std::unordered_set<std::string>({"all"}), _prompt_text, false, _text_frontend);
    auto *prompt_str = std::get_if<std::string>(&prompt_result);
    if (NULL == prompt_str) {
      LOG(INFO) << "InferenceZeroShot::inference_zero_shot Process Prompt Failed";
      _result_wav_data.clear();
      sem_post(&_resp_sem);
      continue;
    }
      
    auto text_result = text_normalize(_tokenizer.get(), std::unordered_set<std::string>({"all"}), _tts_text, true, _text_frontend);
    auto *text_vec = std::get_if<std::vector<std::string>>(&text_result);
    if (NULL == text_vec) {
      LOG(INFO) << "InferenceZeroShot::inference_zero_shot Process Text Failed";
      _result_wav_data.clear();
      sem_post(&_resp_sem);
      continue;
    }
    LOG(INFO) << "InferenceZeroShot::inference_zero_shot Text Len[" << text_vec->size() << "]";

#if 0
    auto job_results = std::vector<std::future<std::pair<int, std::vector<float>>>>();
    for (int i = 0; i < (int)text_vec->size(); i++) {
      const std::string& text = text_vec->at(i);
      job_results.push_back(std::async(std::launch::async, [&]() -> std::pair<int, std::vector<float>> {
        if (text.size() < 0.5 * prompt_text.size()) {
          LOG(INFO) << "synthesis text " << text << " too short than prompt text " << prompt_text << ", this may lead to bad performance";
        }
        std::vector<float> wav_data;
        Frontend::ZeroShotInput infer_result;
        int ret = _frontend->frontend_zero_shot(text, prompt_text, prompt_speech_16k, _sample_rate, infer_result);
        if (0 == ret) {
          ret = _model->tts(
#if USE_GPU
            infer_result.text.to(torch::kCUDA),
            infer_result.prompt_text.to(torch::kCUDA),
            infer_result.prompt_speech_token.to(torch::kCUDA),
            infer_result.prompt_speech_feat.to(torch::kCUDA),
            infer_result.embedding.to(torch::kCUDA),
#else
            infer_result.text,
            infer_result.prompt_text,
            infer_result.prompt_speech_token,
            infer_result.prompt_speech_feat,
            infer_result.embedding,
#endif
            speed,
            wav_data
            );
        }
        return {ret, wav_data};
      }));
    }
    
    std::vector<std::pair<int, std::vector<float>>> job_ret;
    for (auto &job_result : job_results) {
      job_ret.push_back(job_result.get());
    }
    for (auto &_ : job_ret) {
      if (_.first != 0) {
        LOG(INFO) << "InferenceZeroShot::inference_zero_shot Run Infer Failed[" << _.first << "]";
        return result;
      }
    }
    for (auto &_ : job_ret) {
      for (auto data : _.second) {
        result.push_back(data);
      }
    }
#else
    for (int i = 0; i < (int)text_vec->size(); i++) {
      const std::string& text = text_vec->at(i);
      if (text.size() < 0.5 * prompt_str->size()) {
        LOG(INFO) << "synthesis text " << text << " too short than prompt text " << *prompt_str << ", this may lead to bad performance";
      }
      std::vector<float> wav_data;
      Frontend::ZeroShotInput infer_result;
      int ret = _frontend->frontend_zero_shot(text, *prompt_str, _prompt_speech_16k, _sample_rate, infer_result);
      if (0 == ret) {
        ret = _model->tts(
#if USE_GPU
          infer_result.text.to(torch::kCUDA),
          infer_result.prompt_text.to(torch::kCUDA),
          infer_result.prompt_speech_token.to(torch::kCUDA),
          infer_result.prompt_speech_feat.to(torch::kCUDA),
          infer_result.embedding.to(torch::kCUDA),
#else
          infer_result.text,
          infer_result.prompt_text,
          infer_result.prompt_speech_token,
          infer_result.prompt_speech_feat,
          infer_result.embedding,
#endif
          _speed,
          wav_data
          );
        if (0 == ret) {
          for (auto data : wav_data) {
            result.push_back(data);
          }
        }
      }
    }
#endif
    _result_wav_data = result;
    _tts_text.clear();
    sem_post(&_resp_sem);
  }
  _tokenizer.reset();
  _frontend.reset();
  _model.reset();
  frontend_clear_cache();
  LOG(INFO) << "InferenceZeroShot::run End";
}

/* vim: set expandtab nu ts=2 sw=2 sts=2: */
