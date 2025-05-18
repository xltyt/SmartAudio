/****************************************************\
 *
 * Copyright (C) 2019 All Rights Reserved
 * Last modified: 2025.05.13 20:13:36
 *
\****************************************************/

#include "audio_utils.h"
#include <glog/logging.h>
#include <sndfile.hh>
#include <samplerate.h>
#include <system_utils.h>
#include <string_utils.h>

int ConvertToWav(const std::string& path, const std::string& output_path) {
  unlink(output_path.c_str());
  std::string cmd = mycommon::str_format("ffmpeg -i %s -ar 16000 %s", path.c_str(), output_path.c_str());
  std::string output;
  int ret = mycommon::mysystem(cmd.c_str(), output);
  LOG(INFO) << "ConvertToWav Cmd[" << cmd << "] Ret[" << ret << "] Output[" << output << "]";
  return 0;
}

int GetWavInfo(const std::string& path, int& nchannels, int& sampwidth, int& framerate, int& nframes, int& duration) {
  return 0;
}

int LoadWav(const std::string& path, int target_sr, std::vector<float>& result, int& channels) {
  /*
  import torch
  import torchaudio
  def load_wav(wav, target_sr):
      speech, sample_rate = torchaudio.load(wav, backend='soundfile')
      speech = speech.mean(dim=0, keepdim=True)
      if sample_rate != target_sr:
          assert sample_rate > target_sr, 'wav sample rate {} must be greater than {}'.format(sample_rate, target_sr)
          speech = torchaudio.transforms.Resample(orig_freq=sample_rate, new_freq=target_sr)(speech)
      return speech
  */

  SF_INFO sf_info;
  SNDFILE* sndfile = sf_open(path.c_str(), SFM_READ, &sf_info);
  if (!sndfile) {
    LOG(WARNING) << "LoadWav Load[" << path << "] Failed";
    return -1;
  }
  int orig_sr = sf_info.samplerate;
  channels  = sf_info.channels;
  long long frames = sf_info.frames;  
  
  // 读取全部帧（交错格式，归一化到 [-1,1] 的 float）
  std::vector<float> interleaved(frames * channels);
  sf_readf_float(sndfile, interleaved.data(), frames);
  sf_close(sndfile);

  //
  //  对交错格式的多声道音频沿通道维求均值，保留维度。
  // 
  //  @param interleaved 输入：交错格式，长度为 channels * num_samples
  //                     布局：[s0_c0, s0_c1, s1_c0, s1_c1, ...]
  //  @param channels 声道数
  //  @param num_samples 帧数（每帧包含 channels 个样本）
  //  @return 输出：单声道样本序列，长度 = num_samples
  //
  auto mean_dim0_keepdim_interleaved = [](const std::vector<float>& interleaved, size_t channels, size_t num_samples) -> std::vector<float> {
    std::vector<float> mono(num_samples, 0.0f);
    for (size_t t = 0; t < num_samples; ++t) {
      float sum = 0.0f;
      for (size_t c = 0; c < channels; ++c) {
        sum += interleaved[t * channels + c];
      }
      mono[t] = sum / static_cast<float>(channels);
    }
    return mono;
	};
  
  LOG(INFO) << "LoadWav Rate[" << orig_sr << "] Channel[" << channels << "]";
  if (target_sr == 0 || target_sr == orig_sr) {
    //result = interleaved;
    result = mean_dim0_keepdim_interleaved(interleaved, channels, frames);
		return 0;
  }
  LOG(WARNING) << "LoadWav Sample Rate Dont Match";
  return -1;
}

/* vim: set expandtab nu ts=2 sw=2 sts=2: */
