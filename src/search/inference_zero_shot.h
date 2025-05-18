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
#include "frontend.h"
#include "whisper_token.h"

class InferenceZeroShot {
public:
  InferenceZeroShot(const std::string& dir, bool is_fp16 = true);
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
  std::string _dir;
  std::unique_ptr<Frontend> _frontend;
  std::unique_ptr<WhisperToken> _tokenizer;
  int _sample_rate = 0;
};

#endif

/* vim: set expandtab nu ts=2 sw=2 sts=2: */
