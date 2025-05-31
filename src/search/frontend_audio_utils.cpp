/****************************************************\
 *
 * Copyright (C) 2019 All Rights Reserved
 * Last modified: 2025.05.15 15:45:06
 *
\****************************************************/

#include "frontend_audio_utils.h"
#include <fstream>
#include <samplerate.h>
#include <kaldi-native-fbank/csrc/online-feature.h>
#include <kaldi-native-fbank/csrc/feature-fbank.h>
#include <string_utils.h>

#if 0
std::vector<float> resample(const std::vector<float>& input, int orig_freq, int new_freq, int channels) {
	double ratio = static_cast<double>(new_freq) / orig_freq;
	int output_len = static_cast<int>(input.size() * ratio) * channels + 4096;

	std::vector<float> output(output_len);

	SRC_DATA src_data = {0};
	src_data.data_in       = input.data();
	src_data.input_frames  = input.size();
	src_data.data_out      = output.data();
	src_data.output_frames = output_len;
	src_data.src_ratio     = ratio;

  LOG(INFO) << channels;
	int ret = src_simple(&src_data, SRC_SINC_BEST_QUALITY, channels);
  if (0 != ret) {
    LOG(WARNING) << "resample Failed[" << ret << "]";
    return std::vector<float>();
  }
  LOG(INFO) << "aaaaaaaa[" << src_data.output_frames_gen << "," << src_data.output_frames;

	output.resize(src_data.output_frames_gen);
	return output;
}
#else

/**
 * @brief 生成 sinc 重采样卷积核
 *
 * @param orig_freq           原始采样率 (GCD 约简前)
 * @param new_freq            目标采样率 (GCD 约简前)
 * @param gcd                 orig_freq 和 new_freq 的最大公约数
 * @param lowpass_filter_width 低通滤波器宽度 (默认 6)
 * @param rolloff             滚降系数 (默认 0.99)
 * @param resampling_method   重采样方法 (默认 "sinc_interp_hann")
 * @param beta                Kaiser 窗 beta 参数 (可选)
 * @param device              设备 (默认 CPU)
 * @param dtype               数据类型 (可选, 默认 float32)
 * @return SincKernelResult   包含 kernel 和 width
 */
// shape: [new_freq, 1, kernel_width]
static std::pair<torch::Tensor, int> get_sinc_resample_kernel(
  int orig_freq,
  int new_freq,
  int gcd,
  int lowpass_filter_width,
  float rolloff,
  const std::string& resampling_method,
  std::optional<float> beta,
  torch::Device device,
  std::optional<torch::Dtype> dtype) {
  
  if (resampling_method != "sinc_interp_hann" && resampling_method != "sinc_interp_kaiser") {
    throw std::invalid_argument("Invalid resampling method: " + resampling_method);
  }

  orig_freq = orig_freq / gcd;
  new_freq = new_freq / gcd;

  if (lowpass_filter_width <= 0) {
    throw std::invalid_argument("Low pass filter width should be positive.");
  }

  float base_freq = static_cast<float>(std::min(orig_freq, new_freq));
  
  // This will perform antialiasing filtering by removing the highest frequencies.
  // At first I thought I only needed this when downsampling, but when upsampling
  // you will get edge artifacts without this, as the edge is equivalent to zero padding,
  // which will add high freq artifacts.
  // 抗混叠滤波: 移除最高频率
  base_freq *= rolloff;

  // The key idea of the algorithm is that x(t) can be exactly reconstructed from x[i] (tensor)
  // using the sinc interpolation formula:
  //   x(t) = sum_i x[i] sinc(pi * orig_freq * (i / orig_freq - t))
  // We can then sample the function x(t) with a different sample rate:
  //    y[j] = x(j / new_freq)
  // or,
  //    y[j] = sum_i x[i] sinc(pi * orig_freq * (i / orig_freq - j / new_freq))

  // We see here that y[j] is the convolution of x[i] with a specific filter, for which
  // we take an FIR approximation, stopping when we see at least `lowpass_filter_width` zeros crossing.
  // But y[j+1] is going to have a different set of weights and so on, until y[j + new_freq].
  // Indeed:
  // y[j + new_freq] = sum_i x[i] sinc(pi * orig_freq * ((i / orig_freq - (j + new_freq) / new_freq))
  //                 = sum_i x[i] sinc(pi * orig_freq * ((i - orig_freq) / orig_freq - j / new_freq))
  //                 = sum_i x[i + orig_freq] sinc(pi * orig_freq * (i / orig_freq - j / new_freq))
  // so y[j+new_freq] uses the same filter as y[j], but on a shifted version of x by `orig_freq`.
  // This will explain the F.conv1d after, with a stride of orig_freq.
  int width = static_cast<int>(std::ceil(static_cast<float>(lowpass_filter_width) * orig_freq / base_freq));

  // If orig_freq is still big after GCD reduction, most filters will be very unbalanced, i.e.,
  // they will have a lot of almost zero values to the left or to the right...
  // There is probably a way to evaluate those filters more efficiently, but this is kept for
  // future work.
  auto idx_dtype = dtype.has_value() ? dtype.value() : torch::kFloat64;
  auto default_dtype = dtype.has_value() ? dtype.value() : torch::kFloat32;

  // idx shape: [1, 1, 2*width + orig_freq]
  auto idx = torch::arange(-width, width + orig_freq, torch::TensorOptions().dtype(idx_dtype).device(device))
    .unsqueeze(0)
    .unsqueeze(0) /
    static_cast<float>(orig_freq);

  // t_offset shape: [new_freq, 1, 1]
  auto t = torch::arange(0, -new_freq, -1, torch::TensorOptions().dtype(default_dtype).device(device))
    .unsqueeze(1)
    .unsqueeze(2) /
    static_cast<float>(new_freq);

  // t shape: [new_freq, 1, 2*width + orig_freq]
  t = t + idx;
  t = t * base_freq;
  t = t.clamp(-lowpass_filter_width, lowpass_filter_width);

  // 窗函数
  torch::Tensor window;
  if (resampling_method == "sinc_interp_hann") {
    window = torch::cos(t * M_PI / lowpass_filter_width / 2.0).pow(2);
  }
  else {
    // sinc_interp_kaiser
    float beta_val = beta.has_value() ? beta.value() : 14.769656459379492;
    auto beta_tensor = torch::tensor(beta_val, torch::TensorOptions().dtype(default_dtype).device(device));
    window = torch::i0(beta_tensor * torch::sqrt(1.0 - (t / lowpass_filter_width).pow(2))) / torch::i0(beta_tensor);
  }

  // sinc 函数: where(t == 0, 1.0, sin(t*pi) / (t*pi))
  t *= M_PI;

  float scale = base_freq / orig_freq;
  auto kernels = torch::where(
      t == 0,
      torch::tensor(1.0, torch::TensorOptions().dtype(default_dtype).device(device)),
      t.sin() / t);
  kernels = kernels * window * scale;

  // 如果未指定 dtype, 转为 float32
  if (!dtype.has_value()) {
    kernels = kernels.to(torch::kFloat32);
  }

  return {kernels, width};
}

/**
 * @brief 使用预计算的 sinc 核对波形进行重采样
 *
 * @param waveform 输入波形, 形状 (..., time)
 * @param orig_freq 原始采样率
 * @param new_freq  目标采样率
 * @param gcd       最大公约数
 * @param kernel    预计算的卷积核
 * @param width     滤波器宽度
 * @return torch::Tensor 重采样后的波形
 */
static torch::Tensor apply_sinc_resample_kernel(
  const torch::Tensor& waveform,
  int orig_freq,
  int new_freq,
  int gcd,
  const torch::Tensor& kernel,
  int width) {
  
  if (!waveform.is_floating_point()) {
    throw std::runtime_error("Expected floating point type for waveform tensor, but received non-floating type.");
  }

  int of = orig_freq / gcd;
  int nf = new_freq / gcd;

  // pack batch: reshape 为 [-1, length]
  auto shape = waveform.sizes();
  auto wf = waveform.view({-1, shape.back()});

  int64_t num_wavs = wf.size(0);
  int64_t length = wf.size(1);

  // pad: (width, width + of)
  wf = torch::nn::functional::pad(wf, torch::nn::functional::PadFuncOptions({width, width + of}));

  // conv1d: input [num_wavs, 1, padded_len], kernel [nf, 1, kw], stride=of
  auto resampled = torch::nn::functional::conv1d(wf.unsqueeze(1), kernel, torch::nn::functional::Conv1dFuncOptions().stride(of));

  // transpose(1, 2) 并 reshape
  resampled = resampled.transpose(1, 2).reshape({num_wavs, -1});

  // target_length = ceil(nf * length / of)
  int64_t target_length = static_cast<int64_t>(std::ceil(static_cast<float>(nf) * length / of));

  // 截断到 target_length
  resampled = resampled.narrow(-1, 0, target_length);

  // unpack batch: 恢复原始 batch 维度
  std::vector<int64_t> out_shape(shape.begin(), shape.end() - 1);
  out_shape.push_back(resampled.size(-1));
  resampled = resampled.view(out_shape);

  return resampled;
}

std::vector<float> resample(const std::vector<float>& input, int orig_freq, int new_freq, int channels) {
  // Ref
  //   src/3rd_party/torchaudio/pkg/src/torchaudio/transforms/_transforms.py
  //   src/3rd_party/torchaudio/pkg/src/torchaudio/functional/functional.py
  #ifndef M_PI
  #define M_PI 3.14159265358979323846
  #endif
  class Resample : public torch::nn::Module {
  public:
    /**
     * @brief 构造重采样器
     *
     * @param orig_freq            原始采样率 (默认 16000)
     * @param new_freq             目标采样率 (默认 16000)
     * @param resampling_method    重采样方法: "sinc_interp_hann"(默认, Hann 窗 sinc 插值) 或 "sinc_interp_kaiser"(Kaiser 窗 sinc 插值)
     * @param lowpass_filter_width 低通滤波器宽度 (默认 6)
     * @param rolloff              滚降系数 (默认 0.99)
     * @param beta                 Kaiser 窗 beta 参数 (可选)
     * @param dtype                数据类型 (可选)
     */
    Resample(
      int orig_freq = 16000,
      int new_freq = 16000,
      const std::string& resampling_method = "sinc_interp_hann",
      int lowpass_filter_width = 6,
      float rolloff = 0.99,
      std::optional<float> beta = std::nullopt,
      std::optional<torch::Dtype> dtype = std::nullopt) :
      
      orig_freq_(orig_freq),
      new_freq_(new_freq),
      resampling_method_(resampling_method),
      lowpass_filter_width_(lowpass_filter_width),
      rolloff_(rolloff),
      beta_(beta),
      gcd_(std::gcd(orig_freq, new_freq)) {
      
      if (orig_freq_ != new_freq_) {
        auto [kernel, width] = get_sinc_resample_kernel(
          orig_freq_,
          new_freq_,
          gcd_,
          lowpass_filter_width_,
          rolloff_,
          resampling_method_,
          beta_,
          torch::kCPU,
          dtype
          );
        kernel_ = kernel;
        register_buffer("kernel", kernel_);
        width_ = width;
      }
    }

    torch::Tensor forward(const torch::Tensor& waveform) {
      if (orig_freq_ == new_freq_) {
        return waveform;
      }
      return apply_sinc_resample_kernel(waveform, orig_freq_, new_freq_, gcd_, kernel_, width_);
    }

  private:
    int orig_freq_;
    int new_freq_;
    std::string resampling_method_;
    int lowpass_filter_width_;
    float rolloff_;
    std::optional<float> beta_;
    int gcd_;
    torch::Tensor kernel_;
    int width_ = 0;
  };

  auto waveform = torch::zeros({1, (int)input.size()}, torch::kFloat32);
  for (int i = 0; i < (int)input.size(); ++i) {
    waveform[0][i] = input[i];
  }
  Resample resampler(orig_freq, new_freq);
  auto output = resampler.forward(waveform);
	
  float* output_data = output.data_ptr<float>();
	return std::vector<float>(output_data, output_data + output.numel());
}
#endif

static torch::Tensor fft_frequencies(float sr, int n_fft, torch::Dtype dtype) {
  // 等同于 np.fft.rfftfreq(n=n_fft, d=1.0/sr)
  // 返回 (0, sr/n_fft, 2*sr/n_fft, ..., sr/2), 共 1 + n_fft/2 个点
  int n = 1 + n_fft / 2;
  auto freqs = torch::arange(0, n, dtype);
  freqs = freqs * (static_cast<double>(sr) / n_fft);
  return freqs;
}

static double hz_to_mel(double frequency, bool htk) {
  if (htk) {
    return 2595.0 * std::log10(1.0 + frequency / 700.0);
  }

  // Slaney's Auditory Toolbox 实现
  double f_min = 0.0;
  double f_sp = 200.0 / 3.0;

  double mel = (frequency - f_min) / f_sp;

  double min_log_hz = 1000.0;
  double min_log_mel = (min_log_hz - f_min) / f_sp;
  double logstep = std::log(6.4) / 27.0;

  if (frequency >= min_log_hz) {
    mel = min_log_mel + std::log(frequency / min_log_hz) / logstep;
  }

  return mel;
}

static torch::Tensor hz_to_mel(const torch::Tensor& frequencies, bool htk) {
  if (htk) {
    return 2595.0 * torch::log10(1.0 + frequencies / 700.0);
  }

  // Slaney's Auditory Toolbox 实现
  double f_min = 0.0;
  double f_sp = 200.0 / 3.0;

  auto mels = (frequencies - f_min) / f_sp;

  double min_log_hz = 1000.0;
  double min_log_mel = (min_log_hz - f_min) / f_sp;
  double logstep = std::log(6.4) / 27.0;

  // 向量化: 对 >= min_log_hz 的部分使用对数刻度
  auto log_t = frequencies >= min_log_hz;
  auto log_freqs = torch::masked_select(frequencies, log_t);
  auto log_mels = min_log_mel + torch::log(log_freqs / min_log_hz) / logstep;
  mels.masked_scatter_(log_t, log_mels);

  return mels;
}

static double mel_to_hz(double mel, bool htk) {
  if (htk) {
    return 700.0 * (std::pow(10.0, mel / 2595.0) - 1.0);
  }

  // Slaney's Auditory Toolbox 实现
  double f_min = 0.0;
  double f_sp = 200.0 / 3.0;

  double freq = f_min + f_sp * mel;

  double min_log_hz = 1000.0;
  double min_log_mel = (min_log_hz - f_min) / f_sp;
  double logstep = std::log(6.4) / 27.0;

  if (mel >= min_log_mel) {
    freq = min_log_hz * std::exp(logstep * (mel - min_log_mel));
  }

  return freq;
}

static torch::Tensor mel_to_hz(const torch::Tensor& mels, bool htk) {
  if (htk) {
      return 700.0 * (torch::pow(10.0, mels / 2595.0) - 1.0);
  }

  // Slaney's Auditory Toolbox 实现
  double f_min = 0.0;
  double f_sp = 200.0 / 3.0;

  auto freqs = f_min + f_sp * mels;

  double min_log_hz = 1000.0;
  double min_log_mel = (min_log_hz - f_min) / f_sp;
  double logstep = std::log(6.4) / 27.0;

  // 向量化: 对 >= min_log_mel 的部分使用指数刻度
  auto log_t = mels >= min_log_mel;
  auto log_mels = torch::masked_select(mels, log_t);
  auto log_freqs = min_log_hz * torch::exp(logstep * (log_mels - min_log_mel));
  freqs.masked_scatter_(log_t, log_freqs);

  return freqs;
}

static torch::Tensor mel_frequencies(int n_mels, float fmin, float fmax, bool htk, torch::Dtype dtype) {
  // 计算 mel 刻度上均匀分布的频率
  double min_mel = hz_to_mel(static_cast<double>(fmin), htk);
  double max_mel = hz_to_mel(static_cast<double>(fmax), htk);

  // np.linspace(min_mel, max_mel, n_mels)
  auto mels = torch::linspace(min_mel, max_mel, n_mels, dtype);

  // 转换回 Hz
  return mel_to_hz(mels, htk);
}

static torch::Tensor compute_mel_filterbank(
  int sr,
  int n_fft,
  int n_mels = 128,
  float fmin = 0,
  float fmax = -1,
  bool htk = false,
  const std::string& norm = "slaney",
  torch::Dtype dtype = torch::kFloat32) {

	if (fmax < 0.0f) {
		fmax = sr / 2.0f;
	}

  // Initialize the weights
	int n_bins = 1 + n_fft / 2;
	auto weights = torch::zeros({n_mels, n_bins}, torch::TensorOptions().dtype(dtype));

  // Center freqs of each FFT bin
  // shape: (n_bins,)
	auto fftfreqs = fft_frequencies(sr, n_fft, dtype);

	// 'Center freqs' of mel bands - uniformly spaced between limits
	auto mel_f = mel_frequencies(n_mels + 2, fmin, fmax, htk, dtype);  // shape: (n_mels+2,)

	// ---- 计算差分 ----
	// fdiff[i] = mel_f[i+1] - mel_f[i],  shape: (n_mels+1,)
	auto fdiff = mel_f.slice(0, 1) - mel_f.slice(0, 0, -1);

	// ---- 外积减法：ramps[i, j] = mel_f[i] - fftfreqs[j] ----
	// shape: (n_mels+2, n_bins)
	auto ramps = mel_f.unsqueeze(1) - fftfreqs.unsqueeze(0);

  for (int i = 0; i < n_mels; ++i) {
    // lower and upper slopes for all bins
    auto lower = -ramps[i] / fdiff[i];
    auto upper = ramps[i + 2] / fdiff[i + 1];

    // .. then intersect them with each other and zero
    weights[i] = torch::maximum(torch::zeros_like(lower), torch::minimum(lower, upper));
  }

	// ---- 归一化处理 ----
	if (norm == "slaney") {
    // Slaney-style mel is scaled to be approx constant energy per channel
		auto enorm = 2.0 / (mel_f.slice(0, 2, n_mels + 2) - mel_f.slice(0, 0, n_mels));
		weights *= enorm.unsqueeze(1);
	}
  else if (!norm.empty()) {
    throw std::invalid_argument("Unsupported norm: " + norm);
  }

  // Only check weights if f_mel[0] is positive
  auto mel_f_head = mel_f.slice(0, 0, -2);  // mel_f[:-2]
  auto max_per_channel = std::get<0>(weights.max(1));  // shape: [n_mels]
  auto valid = (mel_f_head == 0) | (max_per_channel > 0);
  if (!torch::all(valid).item<bool>()) {
    LOG(WARNING) << "[WARNING] Empty filters detected in mel frequency basis. "
                 << "Some channels will produce empty responses. "
                 << "Try increasing your sampling rate (and fmax) or "
                 << "reducing n_mels.";
  }

	return weights;
}

static torch::Tensor dynamic_range_compression_torch(const torch::Tensor& x, float C = 1.0f, float clip_val = 1e-5f) {
	return torch::log(torch::clamp(x, clip_val) * C);
}

static torch::Tensor spectral_normalize_torch(const torch::Tensor& magnitudes) {
	return dynamic_range_compression_torch(magnitudes);
}

// Cache
static std::map<std::string, torch::Tensor> mel_basis_cache;
static std::map<std::string, torch::Tensor> hann_window_cache;

static torch::Tensor mel_spectrogram(
	const torch::Tensor& y,
  int n_fft,
  int num_mels,
  int sampling_rate,
  int hop_size,
  int win_size,
  float fmin,
  float fmax,
  bool center = false) {
  // Ref
  //   CosyVoice/third_party/Matcha-TTS/matcha/utils/audio.py
  //   librosa_mel_fn
  //     /usr/local/lib/python3.10/dist-packages/librosa/filters.py
    
	// 检查输入范围
	auto y_min = torch::min(y).item<float>();
	auto y_max = torch::max(y).item<float>();
	if (y_min < -1.0f) {
		LOG(WARNING) << "mel_spectrogram: min value is " << y_min;
	}
	if (y_max > 1.0f) {
		LOG(WARNING) << "mel_spectrogram: max value is " << y_max;
	}

	// Device
	std::string device_str = y.device().str();
	std::string mel_key = std::to_string(fmax) + "_" + device_str;
	std::string window_key = device_str;

  LOG(INFO) << "compute_mel_filterbank Start";
	// 缓存 Mel 滤波器组
	if (mel_basis_cache.find(mel_key) == mel_basis_cache.end()) {
		torch::Tensor mel_filter = compute_mel_filterbank(sampling_rate, n_fft, num_mels, fmin, fmax);
		mel_basis_cache[mel_key] = mel_filter.to(y.device());
		hann_window_cache[window_key] = torch::hann_window(win_size).to(y.device());
	}
  LOG(INFO) << "compute_mel_filterbank End";

  LOG(INFO) << "mel_spectrogram Pad";
	// Padding
	int padding = (n_fft - hop_size) / 2;
	auto y_padded = torch::nn::functional::pad(
		y.unsqueeze(1),
		torch::nn::functional::PadFuncOptions({padding, padding}).mode(torch::kReflect)
		)
    .squeeze(1);

	// STFT
  LOG(INFO) << "mel_spectrogram STFT";
	auto spec_complex = torch::stft(
		y_padded,
		n_fft,
		hop_size,
		win_size,
		hann_window_cache[window_key],
		center,
		"reflect",
		false,
		true,
		true
    );

  auto spec = torch::view_as_real(spec_complex);

	// 计算幅度谱
	auto spec_magnitude = torch::sqrt(torch::pow(spec, 2).sum(-1) + 1e-9);

	// 应用 Mel 滤波器组
	auto mel_spec = torch::matmul(mel_basis_cache[mel_key], spec_magnitude);

  LOG(INFO) << "mel_spectrogram Normal";
	// 频谱归一化
	mel_spec = spectral_normalize_torch(mel_spec);

  LOG(INFO) << "mel_spectrogram End";
	return mel_spec;
}

std::pair<torch::Tensor, torch::Tensor> extract_speech_feat(const torch::Tensor& speech) {
  /*
  speech_feat = self.feat_extractor(speech).squeeze(dim=0).transpose(0, 1).to(self.device)
  #speech_feat = mel_spectrogram(speech, n_fft=1024, num_mels=80, sampling_rate=22050, hop_size=256, win_size=1024, fmin=0, fmax=8000, center=False).squeeze(dim=0).transpose(0, 1).to(self.device)
  speech_feat = speech_feat.unsqueeze(dim=0)
  speech_feat_len = torch.tensor([speech_feat.shape[1]], dtype=torch.int32).to(self.device)
  return speech_feat, speech_feat_len
  */

  LOG(INFO) << "extract_speech_feat";
  int n_fft = 1024;
  int num_mels = 80;
  int sampling_rate = 22050;
  int hop_size = 256;
  int win_size = 1024;
  float fmin = 0.0f;
  float fmax = 8000.0f;
  bool center = false;

  // 计算梅尔频谱
  LOG(INFO) << "mel_spectrogram";
  torch::Tensor speech_feat = mel_spectrogram(
    speech, 
    n_fft, 
    num_mels, 
    sampling_rate, 
    hop_size, 
    win_size, 
    fmin, 
    fmax, 
    center
    );

  LOG(INFO) << "mel_spectrogram 1";
  // 维度调整
  // speech_feat 原始形状: [1, n_mels, time_frames]
  speech_feat = speech_feat.squeeze(0);          // 移除批次维度 -> [n_mels, time_frames]
  speech_feat = speech_feat.transpose(0, 1);     // 转置 -> [time_frames, n_mels]
  //speech_feat = speech_feat.to(device);          // 移动到指定设备

  // 添加批次维度
  speech_feat = speech_feat.unsqueeze(0);        // -> [1, time_frames, n_mels]

  // 计算特征长度
  torch::Tensor speech_feat_len = torch::tensor(
    {speech_feat.size(1)}, 
    torch::TensorOptions().dtype(torch::kInt32));
    //.to(device);

  LOG(INFO) << "mel_spectrogram 2";
  return {speech_feat, speech_feat_len};
}

static std::map<int, torch::Tensor> mel_filters_cache;
static torch::Tensor mel_filters(const std::string& dir, torch::Device device, int n_mels) {
  if (n_mels != 80 && n_mels != 128) {
    std::ostringstream oss;
    oss << "Unsupported n_mels: " << n_mels;
    throw std::invalid_argument(oss.str());
  }

  torch::Tensor filters;
  auto it = mel_filters_cache.find(n_mels);
  if (it == mel_filters_cache.end()) {
    //torch::load(filters, mycommon::str_format("data/whisper/mel_filters_mel_%d.pt", n_mels));
		auto load_tensor_from_binary = [](const std::string& path, std::vector<int64_t> shape) -> torch::Tensor {
			int64_t total_elements = 1;
			for (auto dim : shape) {
				total_elements *= dim;
			}
			std::ifstream file(path, std::ios::binary);
			if (!file.is_open()) {
				throw std::runtime_error("Failed to open file: " + path);
			}
			auto options = torch::TensorOptions().dtype(torch::kFloat32);
			torch::Tensor tensor = torch::empty({total_elements}, options);
			file.read(reinterpret_cast<char*>(tensor.data_ptr()), total_elements * sizeof(float));
			file.close();
			return tensor.reshape(shape);
		};
    filters = load_tensor_from_binary(mycommon::str_format("%s/whisper/mel_filters_mel_%d.bin", dir.c_str(), n_mels), std::vector<int64_t>({n_mels, 201}));
    mel_filters_cache[n_mels] = filters;
  }
  else {
    filters = it->second;
  }
  return filters;
}

torch::Tensor log_mel_spectrogram(
  const std::string& dir,
  torch::Tensor audio,
  int n_mels /*= 80*/,
  int padding /*= 0*/,
  std::optional<torch::Device> device /*= std::nullopt*/
  ) {
  // Ref
  //   /usr/local/lib/python3.10/dist-packages/whisper/audio.py
  constexpr int N_FFT = 400;
  constexpr int HOP_LENGTH = 160;
  if (device.has_value()) {
    audio = audio.to(device.value());
  }
  if (padding > 0) {
    audio = torch::nn::functional::pad(
      audio,
      torch::nn::functional::PadFuncOptions({0, padding})
      );
  }
    
  // 使用 Hann 窗口计算 STFT
  auto window = torch::hann_window(N_FFT, audio.options());
  auto stft = torch::stft(
    audio,
    N_FFT,
    HOP_LENGTH,
    /*win_length=*/N_FFT,
    window,
    /*center=*/true,
    /*pad_mode=*/"reflect",
    /*normalized=*/false,
    /*onesided=*/true,
    /*return_complex=*/true
  );
  
  // 计算功率谱: 去掉最后一帧
  auto magnitudes = stft.slice(-1, 0, -1).abs().pow(2);
  
  // 应用 Mel 滤波器组
  auto filters = mel_filters(dir, audio.device(), n_mels);
  auto mel_spec = torch::matmul(filters, magnitudes);

  // 对数压缩与归一化
  auto log_spec = torch::clamp(mel_spec, /*min=*/1e-10).log10();
  log_spec = torch::maximum(log_spec, log_spec.max() - 8.0);
  log_spec = (log_spec + 4.0) / 4.0;

  return log_spec;
}

torch::Tensor kaldi_fbank(torch::Tensor speech) {
  // Convert Array, Shape: (num_samples,)
  speech = speech.squeeze(0);
  // Array
  TORCH_CHECK(speech.dim() == 1, "Input speech must be 1D waveform");
  auto speech_cpu = speech.contiguous().to(torch::kCPU);
  const float* speech_data = speech_cpu.data_ptr<float>();
  int64_t num_samples = speech_cpu.size(0);

  // Fbank Option
  knf::FbankOptions fbank_opts;
  fbank_opts.mel_opts.num_bins = 80;
  fbank_opts.frame_opts.samp_freq = 16000;
  fbank_opts.frame_opts.dither = 0;
  
  knf::OnlineFbank fbank(fbank_opts);
  fbank.AcceptWaveform(16000, speech_data, num_samples);
  fbank.InputFinished();

  int32_t num_frames = fbank.NumFramesReady();
  int32_t num_bins = fbank_opts.mel_opts.num_bins;
  TORCH_CHECK(num_frames > 0, "No frames computed from speech");
        
  std::vector<float> feat_vec(num_frames * num_bins);
  for (int32_t i = 0; i < num_frames; ++i) {
    const float* frame = fbank.GetFrame(i);
    std::copy(frame, frame + num_bins, feat_vec.begin() + i * num_bins);
  }
  
  //auto feat_vec_tensor = torch::zeros({1, (int)feat_vec.size()}, torch::kFloat32);
  //for (int i = 0; i < (int)feat_vec.size(); ++i) {
  //  feat_vec_tensor[0][i] = feat_vec[i];
  //}
  //return feat_vec_tensor;
  return torch::from_blob(feat_vec.data(), {num_frames, num_bins}, torch::kFloat32).clone();
}

void frontend_clear_cache() {
  mel_basis_cache.clear();
  hann_window_cache.clear();
  mel_filters_cache.clear();
}

/* vim: set expandtab nu ts=2 sw=2 sts=2: */
