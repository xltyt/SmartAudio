/****************************************************\
 *
 * Copyright (C) 2019 All Rights Reserved
 * Last modified: 2025.05.15 15:42:25
 *
\****************************************************/

#ifndef _FRONTEND_AUTIO_UTILS_H__
#define _FRONTEND_AUTIO_UTILS_H__

#include <vector>
#include <string>
#define C10_USE_GLOG
#include <torch/torch.h>
#include <glog/logging.h>

std::vector<float> resample(const std::vector<float>& input, int orig_freq, int new_freq, int channels);

std::pair<torch::Tensor, torch::Tensor> extract_speech_feat(const torch::Tensor& speech);

/**
 * @brief 计算 Log-Mel 频谱图
 *
 * 处理流程:
 *   1. 可选: 将音频移至指定设备
 *   2. 可选: 对音频末尾进行零填充
 *   3. 使用 Hann 窗口计算 STFT
 *   4. 计算功率谱 (幅度的平方)
 *   5. 应用 Mel 滤波器组
 *   6. 对数压缩: log10, 动态范围裁剪, 归一化
 *
 * @param audio   输入音频张量, 形状 [num_samples] 或 [batch, num_samples]
 * @param n_mels  Mel 频带数量 (默认 80)
 * @param padding 末尾零填充采样点数 (默认 0)
 * @param device  目标设备 (可选, 默认使用 audio 所在设备)
 * @return torch::Tensor Log-Mel 频谱图, 形状 [n_mels, num_frames]
 *                       或 [batch, n_mels, num_frames]
 */
torch::Tensor log_mel_spectrogram(
  torch::Tensor audio,
  int n_mels = 80,
  int padding = 0,
  std::optional<torch::Device> device = std::nullopt
  );

#endif

/* vim: set expandtab nu ts=2 sw=2 sts=2: */
