/****************************************************\
 *
 * Copyright (C) 2025. All Rights Reserved
 * Last modified: 2025.05.13 15:38:13
 *
\****************************************************/

#ifndef _INFERENCE_ZERO_SHOT_H__
#define _INFERENCE_ZERO_SHOT_H__

#include <string>
#include <memory>
#include <semaphore.h>
#include <lock.h>
#include "frontend.h"
#include "whisper_token.h"
#include "v1/model.h"

class InferenceZeroShot {
public:
  InferenceZeroShot(
    const std::string& dir,
    bool is_fp16 = true
    );
  virtual ~InferenceZeroShot();

public:
  std::vector<float> inference_zero_shot(
    const std::string& tts_text,
    const std::string& prompt_text,
    const std::vector<float>& prompt_speech_16k,
    bool stream = false,
    float speed = 1.0,
		bool text_frontend = true
		);

protected:
  std::atomic<bool> _want_exit = {false};
  std::string _dir;
  bool _is_fp16 = true;
  std::unique_ptr<Frontend> _frontend;
  std::unique_ptr<WhisperToken> _tokenizer;
  std::unique_ptr<VoiceModel> _model;
  int _sample_rate = 0;
  pthread_t _run_thread_id;
  static void* _run_thread(void *arg);
  void run();
  sem_t _req_sem;
  sem_t _resp_sem;
  common::RWLock _run_lock;
  
  std::string _tts_text;
  std::string _prompt_text;
  std::vector<float> _prompt_speech_16k;
  bool _stream = false;
  float _speed = 1.0;
  bool _text_frontend = true;
  std::vector<float> _result_wav_data;
};

#endif

/* vim: set expandtab nu ts=2 sw=2 sts=2: */
