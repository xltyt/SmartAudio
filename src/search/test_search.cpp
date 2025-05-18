/****************************************************\
 *
 * Copyright (C) 2019 All Rights Reserved
 * Last modified: 2025.05.18 19:47:45
 *
\****************************************************/

#include <iostream>
#include <gtest/gtest.h>
#include <glog/logging.h>
#include <crypt_utils.h>
#include <file_utils.h>
#include <string_utils.h>
#include <timer.h>
#include "tiktoken.h"
#include "whisper_token.h"
#include "audio_utils.h"
#include "frontend_utils.h"
#include "frontend_audio_utils.h"
#include "inference_zero_shot.h"

TEST(Search, Utf8) {
  {
    std::vector<uint32_t> ids = CoreBPE::decode_utf8("Ġ");
    ASSERT_EQ(ids.size(), 1);
    ASSERT_EQ(ids[0], 288);
  }
  {
    std::vector<uint32_t> ids = CoreBPE::decode_utf8("Ġt");
    ASSERT_EQ(ids.size(), 2);
    ASSERT_EQ(ids[0], 288);
    ASSERT_EQ(ids[1], 116);
  }
  {
    std::vector<uint32_t> ids = CoreBPE::decode_utf8("Ġbr");
    ASSERT_EQ(ids.size(), 3);
    ASSERT_EQ(ids[0], 288);
    ASSERT_EQ(ids[1], 98);
    ASSERT_EQ(ids[2], 114);
  }
}

TEST(Search, TikTokenEncodingGpt) {
  const std::string& ENDOFTEXT = "<|endoftext|>";
  const std::string& FIM_PREFIX = "<|fim_prefix|>";
  const std::string& FIM_MIDDLE = "<|fim_middle|>";
  const std::string& FIM_SUFFIX = "<|fim_suffix|>";
  const std::string& ENDOFPROMPT = "<|endofprompt|>";

  auto ToString = [](const std::vector<size_t>& ids) -> std::string {
    std::string str;
    for (int i = 0; i < ids.size(); i++) {
      if (str.size()) {
        str += " ";
      }
      str += std::to_string(ids[i]);
    }
    return str;
  };
  // gpt2
  CoreBPE::HashMap<std::vector<uint8_t>, size_t> mergeable_ranks = CoreBPE::data_gym_to_mergeable_bpe_ranks("data/gpt2/vocab.bpe", "data/gpt2/encoder.json");
  LOG(INFO) << "Convert BPE Finished, Len[" << mergeable_ranks.size() << "]";
  ASSERT_EQ(50256, mergeable_ranks.size());
  CoreBPE::StringMap<size_t> special_tokens;
  special_tokens[ENDOFTEXT] = 50256;
  TikTokenEncoding encoding(
    "gpt2",
    "'s|'t|'re|'ve|'m|'ll|'d| ?\\p{L}+| ?\\p{N}+| ?[^\\s\\p{L}\\p{N}]+|\\s+(?!\\S)|\\s+",
    //"'s|'t|'re|'ve|'m|'ll|'d| ?\\p{L}+| ?\\p{N}+| ?[^\\s\\p{L}\\p{N}]+|\\s+",
    //"'s|'t|'re|'ve|'m|'ll|'d|\\s+",
    mergeable_ranks,
    special_tokens,
    50257
    );
  LOG(INFO) << "Init Encoding Finished";
  ASSERT_EQ(true, encoding.encode("hello world") == std::vector<size_t>({31373, 995}));
  ASSERT_EQ(true, encoding.encode("hello <|endoftext|>", std::unordered_set<std::string>{"all"}) == std::vector<size_t>({31373, 220, 50256}));
  ASSERT_EQ(true, encoding.encode("<|endoftext|>", std::unordered_set<std::string>{"all"}) == std::vector<size_t>({50256}));
  ASSERT_EQ(true, encoding.encode("<|endoftext|>", std::unordered_set<std::string>{"<|endoftext|>"}) == std::vector<size_t>({50256}));
  ASSERT_EQ(true, encoding.encode("<|endoftext|>", std::unordered_set<std::string>{}, std::unordered_set<std::string>{}) == std::vector<size_t>({27, 91, 437, 1659, 5239, 91, 29}));
	try {
  	std::vector<size_t> ids = encoding.encode("<|endoftext|>");
    ASSERT_EQ(true, false);
	}
	catch (const std::exception& e) {
    ASSERT_EQ(true, true);
	}
	ASSERT_EQ(true, encoding.encode("0") == std::vector<size_t>({15}));
  ASSERT_EQ(true, encoding.encode("000000000") == std::vector<size_t>({10535, 830}));
  ASSERT_EQ(true, encoding.encode("00000000000000000") == std::vector<size_t>({8269, 10535, 830}));
  ASSERT_EQ(true, encoding.encode("今天天气真不错") == std::vector<size_t>({20015, 232, 25465, 25465, 36365, 242, 40367, 253, 38834, 165, 242, 247}));
  ASSERT_EQ(true, encoding.encode("i'm Jack") == std::vector<size_t>({72, 1101, 3619}));
  ASSERT_EQ(true, encoding.encode("") == std::vector<size_t>());
  LOG(INFO) << "Finished";
}

TEST(Search, TikTokenEncodingCl100K) {
  CoreBPE::HashMap<std::vector<uint8_t>, size_t> mergeable_ranks = CoreBPE::load_tiktoken_bpe("data/cl100k_base/cl100k_base.tiktoken");
  LOG(INFO) << "Convert BPE Finished, Len[" << mergeable_ranks.size() << "]";
  ASSERT_EQ(100256, mergeable_ranks.size());
  CoreBPE::StringMap<size_t> special_tokens;
  special_tokens["ENDOFTEXT"] = 100257;
  special_tokens["FIM_PREFIX"] = 100258;
  special_tokens["FIM_MIDDLE"] = 100259;
  special_tokens["FIM_SUFFIX"] = 100260;
  special_tokens["ENDOFPROMPT"] = 100276;
  TikTokenEncoding encoding(
    "cl100k_base",
    "(?i:'s|'t|'re|'ve|'m|'ll|'d)|[^\\r\\n\\p{L}\\p{N}]?\\p{L}+|\\p{N}{1,3}| ?[^\\s\\p{L}\\p{N}]+[\\r\\n]*|\\s*[\\r\\n]+|\\s+(?!\\S)|\\s+",
    mergeable_ranks,
    special_tokens,
    50257
    );
  LOG(INFO) << "Init Encoding Finished";
  ASSERT_EQ(true, encoding.encode("rer") == std::vector<size_t>({38149}));
  ASSERT_EQ(true, encoding.encode("'rer") == std::vector<size_t>({2351, 81}));
  ASSERT_EQ(true, encoding.encode("today\n ") == std::vector<size_t>({31213, 198, 220}));
  ASSERT_EQ(true, encoding.encode("today\n \n") == std::vector<size_t>({31213, 27907}));
  ASSERT_EQ(true, encoding.encode("today\n  \n") == std::vector<size_t>({31213, 14211}));
  ASSERT_EQ(true, encoding.encode("hello world") == std::vector<size_t>({15339, 1917}));
  //std::vector<size_t> ids = encoding.encode(" \x85""0");
  //LOG(INFO) << ids.size();
  //for (auto id : ids) {
  //  LOG(INFO) << id;
  //}
  //ASSERT_EQ(true, encoding.encode(" \x85""0") == std::vector<size_t>({220, 126, 227, 15}));
    
  ASSERT_EQ(true, encoding.encode("👍") == std::vector<size_t>({9468, 239, 235}));
  //
  //  # surrogate pair gets converted to codepoint
  //  assert enc.encode("") == []
  //  # lone surrogate just gets replaced
  //  assert enc.encode("\ud83d") == enc.encode("�")
  //ASSERT_EQ(true, encoding.encode("\ud83d\udc4d") == std::vector<size_t>({9468, 239, 235}));
  //ASSERT_EQ(true, encoding.encode("") == std::vector<size_t>({}));
  LOG(INFO) << "Finished";
}

TEST(Search, TikTokenEncodingR50K) {
}

TEST(Search, WhisperToken) {
  const std::string text = "다람쥐 헌 쳇바퀴에 타고파";
  WhisperToken token(".");
  std::vector<size_t> multilingual_tokens = token.encode(text);
  ASSERT_EQ(true, multilingual_tokens == std::vector<size_t>({9835, 22855, 168, 98, 238, 13431, 234, 43517, 229, 47053, 169, 222, 19086, 19840, 1313, 17974}));
}

TEST(Search, Wav) {
  const std::string& path = "data/mda-qmwfy2k746929rxh.mp3";
  std::string output_path = "out_" + Crypt::gen_random_string(8) + ".wav";
  ConvertToWav(path, output_path);
  std::vector<float> result;
  int channels = 0;
  int ret = LoadWav(output_path, 160000, result, channels);
  ASSERT_EQ(ret, -1);
  ret = LoadWav(output_path, 16000, result, channels);
  ASSERT_EQ(ret, 0);
  LOG(INFO) << "Len[" << result.size() << "]";
  unlink(output_path.c_str());
  mycommon::file_write("data.txt", mycommon::str_join(result, "\n"));
}

TEST(Search, FrontendUtils) {
  ASSERT_EQ(true, contains_chinese("中国"));
  ASSERT_EQ(true, contains_chinese("1中2国3"));
  ASSERT_EQ(true, contains_chinese("12国3"));
  ASSERT_EQ(false, contains_chinese("123"));
  ASSERT_EQ(false, contains_chinese("a"));

  ASSERT_EQ(true, replace_blank("a b") == "a b");
  ASSERT_EQ(true, replace_blank("a  b") == "ab");
  ASSERT_EQ(true, replace_blank("中 国") == "中国");
  ASSERT_EQ(true, replace_blank("中  国") == "中国");
  ASSERT_EQ(true, replace_blank("中 a") == "中a");
  ASSERT_EQ(true, replace_blank("中  a") == "中a");
  
  ASSERT_EQ(true, remove_tail_mark("中国，") == "中国。");
  ASSERT_EQ(true, remove_tail_mark("中国，，") == "中国。");
  ASSERT_EQ(true, remove_tail_mark("中国，，,") == "中国。");
  ASSERT_EQ(true, remove_tail_mark("中国，，,、、") == "中国。");
  ASSERT_EQ(true, remove_tail_mark("中国A，，,、、") == "中国A。");
  
  ASSERT_EQ(false, is_only_punctuation("中国"));
  ASSERT_EQ(true, is_only_punctuation("."));
  ASSERT_EQ(true, is_only_punctuation("。"));
  ASSERT_EQ(true, is_only_punctuation("。、."));

  ASSERT_EQ(true, number_to_words((long long)0) == "zero");
  ASSERT_EQ(true, number_to_words((long long)1) == "one");
  ASSERT_EQ(true, number_to_words((long long)100) == "one hundred");
  ASSERT_EQ(true, number_to_words((long long)123) == "one hundred and twenty-three");
  ASSERT_EQ(true, number_to_words((long long)12345) == "twelve thousand, three hundred and forty-five");
  ASSERT_EQ(true, number_to_words((long long)-1) == "minus one");
  
  ASSERT_EQ(true, number_to_words(123.2) == "one hundred and twenty-three point two");
  ASSERT_EQ(true, number_to_words(123.23456) == "one hundred and twenty-three point two three four five six");
  ASSERT_EQ(true, number_to_words(-123.2) == "minus one hundred and twenty-three point two");
  
  ASSERT_EQ(true, spell_out_number("A 123 B") == "A one hundred and twenty-three B");

  WhisperToken tokenize(".");
  {
    auto result = text_normalize(&tokenize, std::unordered_set<std::string>({"all"}), "中国A，，,、、", false, true);
    if (auto* str = std::get_if<std::string>(&result)) {
      const std::string& word = *str;
      LOG(INFO) << "Match[" << word << "]";
      ASSERT_EQ(true, word == "中国A。");
    }
    else {
      ASSERT_EQ(false, true);
    }
  }
  {
    std::string text = "家事国事天下事，让人民过上幸福生活是头等大事。家家户户都盼着孩子能有好的教育，老人能有好的养老服务，年轻人能有更多发展机会。这些朴实的愿望，";
    auto result = text_normalize(&tokenize, std::unordered_set<std::string>({"all"}), text, true, true);
    if (auto* vec = std::get_if<std::vector<std::string>>(&result)) {
      for (auto word : *vec) {
        LOG(INFO) << "Match[" << word << "]";
      }
    }
    else {
      ASSERT_EQ(false, true);
    }
  }
  {
    std::string text = "习近平总书记高度重视新型基础设施发展，不仅对构建新型基础设施体系作出全局谋划，提出“要适度超前，布局有利于引领产业发展和维护国家安全的基础设施”，“打造集约高效、经济适用、智能绿色、安全可靠的现代化基础设施体系”，并对网络强国、数字中国、交通强国、能源强国等作出一系列具体部署，强调要“加快建设高速泛在、天地一体、云网融合、智能敏捷、绿色低碳、安全可控的智能化综合性数字信息基础设施”，为新时代推动新型基础设施发展提供了思想指引和根本遵循。";
    auto result = text_normalize(&tokenize, std::unordered_set<std::string>({"all"}), text, true, true);
    if (auto* vec = std::get_if<std::vector<std::string>>(&result)) {
      for (auto word : *vec) {
        LOG(INFO) << "Match[" << word << "]";
      }
    }
    else {
      ASSERT_EQ(false, true);
    }
  }
  {
    auto result = text_normalize(&tokenize, std::unordered_set<std::string>({"all"}), "中国A，，,、、", true, true);
    if (auto* vec = std::get_if<std::vector<std::string>>(&result)) {
      for (auto word : *vec) {
        LOG(INFO) << "Match[" << word << "]";
      }
    }
    else {
      ASSERT_EQ(false, true);
    }
  }
}

template <typename T>
std::vector<T> tensor_to_list_1d(torch::Tensor tensor) {
	tensor = tensor.to(torch::kCPU).contiguous();
	T* data = tensor.data_ptr<T>();
	return std::vector<T>(data, data + tensor.numel());
}

TEST(Search, Frontend) {
  const std::string speech_token_path = mycommon::str_format("./data/model/speech_tokenizer_v1.onnx");
  Frontend frontend(speech_token_path);
  {
    auto [text_token, text_token_len] = frontend.extract_text_token("主席说我开飞机的水平很高。");
  	ASSERT_EQ(2, text_token.dim());
    std::vector<int> text_token_d0 = tensor_to_list_1d<int>(text_token[0]);
    ASSERT_EQ(15, text_token_d0.size());
    ASSERT_EQ(true, text_token_d0 == std::vector<int>({13557, 4845, 255, 8090, 1654, 18937, 11808, 252, 37960, 1546, 15590, 16716, 4563, 12979, 1543}));
  	
    ASSERT_EQ(0, text_token_len.dim());
    ASSERT_EQ(true, text_token_len.scalar_type() == torch::kInt32);
    ASSERT_EQ(15, *(text_token_len.to(torch::kInt32).data_ptr<int>()));
	}
  {
    //auto [text_token, text_token_len] = frontend.extract_text_token("2024年，我们一起走过春夏秋冬，一道经历风雨彩虹，一个个瞬间定格在这不平凡的一年，令人感慨、难以忘怀。");
    auto [text_token, text_token_len] = frontend.extract_text_token("二零二四年，我们一起走过春夏秋冬，一道经历风雨彩虹，一个个瞬间定格在这不平凡的一年，令人感慨、难以忘怀。");
  	ASSERT_EQ(2, text_token.dim());
    std::vector<int> text_token_d0 = tensor_to_list_1d<int>(text_token[0]);
    ASSERT_EQ(66, text_token_d0.size());
    ASSERT_EQ(true, text_token_d0 == std::vector<int>({11217, 6306, 114, 11217, 19425, 5157, 171, 120, 234, 15003, 29567, 9575, 16866, 46953, 42708, 40190, 5676, 105, 171, 120, 234, 2257, 6025, 30276, 5014, 228, 47209, 35339, 7391, 102, 12026, 117, 171, 120, 234, 20182, 7549, 36733, 105, 31685, 12088, 30921, 3581, 5562, 1960, 16716, 6336, 94, 1546, 2257, 5157, 171, 120, 234, 49061, 4035, 9709, 12358, 101, 1231, 46531, 3588, 26677, 3757, 222, 1543}));
  	
    ASSERT_EQ(0, text_token_len.dim());
    ASSERT_EQ(true, text_token_len.scalar_type() == torch::kInt32);
    ASSERT_EQ(66, *(text_token_len.to(torch::kInt32).data_ptr<int>()));
	}

  // Resample
  std::vector<float> prompt_speech_16k;
  int channels = 0;
  {
    const std::string& path = "data/mda-qmwfy2k746929rxh.mp3";
    std::string output_path = "out_" + Crypt::gen_random_string(8) + ".wav";
    ConvertToWav(path, output_path);
    int ret = LoadWav(output_path, 16000, prompt_speech_16k, channels);
    ASSERT_EQ(ret, 0);
    LOG(INFO) << "Len[" << prompt_speech_16k.size() << "]";
    unlink(output_path.c_str());
  }
	LOG(INFO) << "Len[" << prompt_speech_16k.size() << "]";
  mycommon::file_write("data.txt", mycommon::str_join(prompt_speech_16k, "\n"));
  ASSERT_EQ(304000, prompt_speech_16k.size());
  ASSERT_EQ(true, std::fabs(prompt_speech_16k[0] - -0.000946044921875) < 0.00000000001);
  ASSERT_EQ(true, std::fabs(prompt_speech_16k[304000 - 1] - -0.004119873046875) < 0.00000000001);
  std::vector<float> prompt_speech_resample = resample(prompt_speech_16k, 16000, 22050, channels);
	LOG(INFO) << "Len[" << prompt_speech_resample.size() << "]";
  mycommon::file_write("data_resample.txt", mycommon::str_join(prompt_speech_resample, "\n"));
  ASSERT_EQ(418950, prompt_speech_resample.size());
  ASSERT_EQ(true, std::fabs(prompt_speech_resample[0] - -0.0009422693401575089) < 0.00000000001);
  LOG(INFO) << prompt_speech_resample[418950 - 1];
  ASSERT_EQ(true, std::fabs(prompt_speech_resample[418950 - 1] - -0.0029986100271344185) < 0.00000000001);

  // Feat
  auto prompt_speech_resample_tensor = torch::zeros({1, (int)prompt_speech_resample.size()}, torch::kFloat32);
  for (int i = 0; i < (int)prompt_speech_resample.size(); ++i) {
    prompt_speech_resample_tensor[0][i] = prompt_speech_resample[i];
  }
  auto [speech_feat, speech_feat_len] = extract_speech_feat(prompt_speech_resample_tensor);
  {
    LOG(INFO) << "Feat Dim[" << speech_feat.dim() << "]";
  	ASSERT_EQ(3, speech_feat.dim());
    auto sizes = speech_feat.sizes();
  	ASSERT_EQ(1, sizes[0]);
  	ASSERT_EQ(1636, sizes[1]);
  	ASSERT_EQ(80, sizes[2]);
    std::vector<float> speech_feat_d0 = tensor_to_list_1d<float>(speech_feat[0][0]);
    ASSERT_EQ(80, speech_feat_d0.size());
    ASSERT_EQ(true, std::fabs(speech_feat_d0[0] - -7.598185062408447) < 0.000001);
    ASSERT_EQ(true, std::fabs(speech_feat_d0[80 - 1] - -11.512925148010254) < 0.000001);
    std::vector<float> speech_feat_d1 = tensor_to_list_1d<float>(speech_feat[0][1636 - 1]);
    ASSERT_EQ(80, speech_feat_d1.size());
    ASSERT_EQ(true, std::fabs(speech_feat_d1[0] - -4.29597282409668) < 0.000001);
    ASSERT_EQ(true, std::fabs(speech_feat_d1[80 - 1] - -9.77196216583252) < 0.000001);
  }
  {
    LOG(INFO) << "Feat Len Dim[" << speech_feat_len.dim() << "]";
  	ASSERT_EQ(1, speech_feat_len.dim());
    std::vector<int> speech_feat_len_d0 = tensor_to_list_1d<int>(speech_feat_len[0]);
    ASSERT_EQ(1, speech_feat_len_d0.size());
  	ASSERT_EQ(1636, speech_feat_len_d0[0]);
  }

  // Log Mel
  auto prompt_speech_16k_tensor = torch::zeros({1, (int)prompt_speech_16k.size()}, torch::kFloat32);
  for (int i = 0; i < (int)prompt_speech_16k.size(); ++i) {
    prompt_speech_16k_tensor[0][i] = prompt_speech_16k[i];
  }
  torch::Tensor prompt_speech_16k_log_mel = log_mel_spectrogram(prompt_speech_16k_tensor, 128);
  {
    LOG(INFO) << "Log Mel Dim[" << prompt_speech_16k_log_mel.dim() << "]";
    ASSERT_EQ(3, prompt_speech_16k_log_mel.dim());
    auto sizes = prompt_speech_16k_log_mel.sizes();
  	ASSERT_EQ(1, sizes[0]);
  	ASSERT_EQ(128, sizes[1]);
  	ASSERT_EQ(1900, sizes[2]);
    std::vector<float> prompt_speech_16k_log_mel_d0 = tensor_to_list_1d<float>(prompt_speech_16k_log_mel[0][0]);
    ASSERT_EQ(1900, prompt_speech_16k_log_mel_d0.size());
    ASSERT_EQ(true, std::fabs(prompt_speech_16k_log_mel_d0[0] - -0.5229721069335938) < 0.000001);
    std::vector<float> prompt_speech_16k_log_mel_d1 = tensor_to_list_1d<float>(prompt_speech_16k_log_mel[0][1]);
    ASSERT_EQ(1900, prompt_speech_16k_log_mel_d1.size());
    ASSERT_EQ(true, std::fabs(prompt_speech_16k_log_mel_d1[0] - -0.4254077672958374) < 0.000001);
    std::vector<float> prompt_speech_16k_log_mel_d2 = tensor_to_list_1d<float>(prompt_speech_16k_log_mel[0][128 - 1]);
    ASSERT_EQ(1900, prompt_speech_16k_log_mel_d2.size());
    ASSERT_EQ(true, std::fabs(prompt_speech_16k_log_mel_d2[1900 - 1] - -0.6115405559539795) < 0.000001);
  }

  // Speech Token
  uint64_t time_start = mycommon::getMilliTime();
  auto [speech_token, speech_token_len] = frontend.extract_speech_token(prompt_speech_16k_tensor);
  {
    LOG(INFO) << "Speech Token Dim[" << speech_token.dim() << "]";
    ASSERT_EQ(2, speech_token.dim());
    auto sizes = speech_token.sizes();
  	ASSERT_EQ(1, sizes[0]);
  	ASSERT_EQ(950, sizes[1]);
    std::vector<int> speech_token_d0 = tensor_to_list_1d<int>(speech_token[0]);
    ASSERT_EQ(950, speech_token_d0.size());
    ASSERT_EQ(203, speech_token_d0[0]);
    ASSERT_EQ(26, speech_token_d0[1]);
    ASSERT_EQ(40, speech_token_d0[950 - 2]);
    ASSERT_EQ(2130, speech_token_d0[950 - 1]);
  }
  LOG(INFO) << "Speech Token Time[" << (mycommon::getMilliTime() - time_start) << "]";
}

TEST(Search, Tensor) {
  auto x = torch::randn({5, 5}); 
  torch::save(x, "/tmp/1.pt");
  torch::Tensor y;
  torch::load(y, "/tmp/1.pt");
  LOG(INFO) << "Load Finished";
}

TEST(Search, Infer) {
  std::vector<float> prompt_speech_16k;
  int channels = 0;
  {
    const std::string& path = "data/mda-qmwfy2k746929rxh.mp3";
    std::string output_path = "out_" + Crypt::gen_random_string(8) + ".wav";
    ConvertToWav(path, output_path);
    int ret = LoadWav(output_path, 16000, prompt_speech_16k, channels);
    ASSERT_EQ(ret, 0);
    LOG(INFO) << "Len[" << prompt_speech_16k.size() << "]";
    unlink(output_path.c_str());
  }
  InferenceZeroShot infer(".");
  const std::string& tts_text = "主席说我开飞机的水平很高";
  const std::string& prompt_text = "2024年，我们一起走过春夏秋冬，一道经历风雨彩虹，一个个瞬间定格在这不平凡的一年，令人感慨、难以忘怀。";
  infer.inference_zero_shot(tts_text, prompt_text, prompt_speech_16k);
}

int main(int argc, char *argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

/* vim: set expandtab nu ts=2 sw=2 sts=2: */
