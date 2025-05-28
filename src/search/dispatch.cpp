/****************************************************\
 *
 * Copyright (C) 2019 All Rights Reserved
 * Last modified: 2025.05.18 19:38:36
 *
\****************************************************/

#include "dispatch.h"
#include <crypt_utils.h>
#include "audio_utils.h"
#include "frontend_utils.h"
#include "frontend_audio_utils.h"
#include "inference_zero_shot.h"
  
static InferenceZeroShot *_infer = NULL;

void InitAudio() {
  LOG(INFO) << "InitAudio Start...";
  _infer = new InferenceZeroShot("./data", false);
  std::vector<float> prompt_speech_16k;
  int channels = 0;
  {
    const std::string& path = "data/mda-qmwfy2k746929rxh.mp3";
    std::string output_path = "out_" + Crypt::gen_random_string(8) + ".wav";
    ConvertToWav(path, output_path);
    LoadWav(output_path, 16000, prompt_speech_16k, channels);
    LOG(INFO) << "Len[" << prompt_speech_16k.size() << "]";
    unlink(output_path.c_str());
  }
  const std::string& tts_text = "今天天气真的很不错，风和日丽";
  const std::string& prompt_text = "2024年，我们一起走过春夏秋冬，一道经历风雨彩虹，一个个瞬间定格在这不平凡的一年，令人感慨、难以忘怀。";
  _infer->inference_zero_shot(tts_text, prompt_text, prompt_speech_16k);
  LOG(INFO) << "InitAudio End";
}

void UninitAudio() {
  delete _infer;
}

int InferAudio(
  const std::string& text,
  const std::string& text_speech,
  const std::string& out_text,
  float speed,
  std::string& result,
  std::string& error) {
  
  std::vector<float> prompt_speech_16k;
  int channels = 0;
  /*
  {
    const std::string& path = "data/mda-qmwfy2k746929rxh.mp3";
    std::string output_path = "out_" + Crypt::gen_random_string(8) + ".wav";
    ConvertToWav(path, output_path);
    int ret = LoadWav(output_path, 16000, prompt_speech_16k, channels);
    ASSERT_EQ(ret, 0);
    LOG(INFO) << "Len[" << prompt_speech_16k.size() << "]";
    unlink(output_path.c_str());
  }
  
  InferenceZeroShot infer("./model", false);
  const std::string& tts_text = "主席说我开飞机的水平很高";
  const std::string& prompt_text = "2024年，我们一起走过春夏秋冬，一道经历风雨彩虹，一个个瞬间定格在这不平凡的一年，令人感慨、难以忘怀。";
  std::vector<float> wav_data = infer.inference_zero_shot(tts_text, prompt_text, prompt_speech_16k);
  int ret = SaveWav("/tmp/infer_output.wav", 22050, wav_data, 1);
  LOG(INFO) << "Save Success";
  */
  return 0;
}

/* vim: set expandtab nu ts=2 sw=2 sts=2: */
