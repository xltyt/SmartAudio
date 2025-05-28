/****************************************************\
 *
 * Copyright (C) 2019 All Rights Reserved
 * Last modified: 2025.05.18 19:38:36
 *
\****************************************************/

#include "dispatch.h"
#include <crypt_utils.h>
#include <file_utils.h>
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
  std::string error;
  mycommon::mkdirs("tmp", error);
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
  
  std::string unique_id = Crypt::gen_random_string(16);
  std::vector<float> prompt_speech_16k;
  {
    std::string path = "tmp/input_" + unique_id + ".mp3";
    std::string path_wav = "tmp/input_" + unique_id + ".wav";
    mycommon::file_write(path, text_speech);
    ConvertToWav(path, path_wav);
    int channels = 0;
    int ret = LoadWav(path_wav, 16000, prompt_speech_16k, channels);
    if (0 != ret) {
      error = "Load Failed";
      LOG(WARNING) << "InferAudio Load[" << path_wav << "] Failed[" << ret << "]";
      return ret;
    }
    //unlink(path.c_str());
    //unlink(path_wav.c_str());
    LOG(INFO) << "Len[" << prompt_speech_16k.size() << "]";
  }
  std::string output_path = "tmp/output_" + unique_id + ".wav";
  std::vector<float> wav_data = _infer->inference_zero_shot(out_text, text, prompt_speech_16k);
  int ret = SaveWav(output_path, 22050, wav_data, 1);
  if (0 != ret) {
    error = "Write Failed";
    LOG(WARNING) << "InferAudio Write[" << output_path << "] Failed[" << ret << "]";
    return ret;
  }
  mycommon::file_read(output_path, result);
  LOG(INFO) << "Save Finished";
  //unlink(output_path.c_str());
  return 0;
}

/* vim: set expandtab nu ts=2 sw=2 sts=2: */
