/****************************************************\
 *
 * Copyright (C) 2019 All Rights Reserved
 * Last modified: 2025.05.11 00:07:10
 *
\****************************************************/

#include "whisper_token.h"
#include <iomanip>
#include <sstream>

WhisperToken::WhisperToken(const std::string& dir) {
  const std::string encoding_name = "multilingual";
  const std::string language = "en";
  const std::string task = "transcribe";
  int num_languages = 100;
  const std::unordered_map<std::string, std::string>& LANGUAGES = {
    {"en", "english"},
    {"zh", "chinese"},
    {"de", "german"},
    {"es", "spanish"},
    {"ru", "russian"},
    {"ko", "korean"},
    {"fr", "french"},
    {"ja", "japanese"},
    {"pt", "portuguese"},
    {"tr", "turkish"},
    {"pl", "polish"},
    {"ca", "catalan"},
    {"nl", "dutch"},
    {"ar", "arabic"},
    {"sv", "swedish"},
    {"it", "italian"},
    {"id", "indonesian"},
    {"hi", "hindi"},
    {"fi", "finnish"},
    {"vi", "vietnamese"},
    {"he", "hebrew"},
    {"uk", "ukrainian"},
    {"el", "greek"},
    {"ms", "malay"},
    {"cs", "czech"},
    {"ro", "romanian"},
    {"da", "danish"},
    {"hu", "hungarian"},
    {"ta", "tamil"},
    {"no", "norwegian"},
    {"th", "thai"},
    {"ur", "urdu"},
    {"hr", "croatian"},
    {"bg", "bulgarian"},
    {"lt", "lithuanian"},
    {"la", "latin"},
    {"mi", "maori"},
    {"ml", "malayalam"},
    {"cy", "welsh"},
    {"sk", "slovak"},
    {"te", "telugu"},
    {"fa", "persian"},
    {"lv", "latvian"},
    {"bn", "bengali"},
    {"sr", "serbian"},
    {"az", "azerbaijani"},
    {"sl", "slovenian"},
    {"kn", "kannada"},
    {"et", "estonian"},
    {"mk", "macedonian"},
    {"br", "breton"},
    {"eu", "basque"},
    {"is", "icelandic"},
    {"hy", "armenian"},
    {"ne", "nepali"},
    {"mn", "mongolian"},
    {"bs", "bosnian"},
    {"kk", "kazakh"},
    {"sq", "albanian"},
    {"sw", "swahili"},
    {"gl", "galician"},
    {"mr", "marathi"},
    {"pa", "punjabi"},
    {"si", "sinhala"},
    {"km", "khmer"},
    {"sn", "shona"},
    {"yo", "yoruba"},
    {"so", "somali"},
    {"af", "afrikaans"},
    {"oc", "occitan"},
    {"ka", "georgian"},
    {"be", "belarusian"},
    {"tg", "tajik"},
    {"sd", "sindhi"},
    {"gu", "gujarati"},
    {"am", "amharic"},
    {"yi", "yiddish"},
    {"lo", "lao"},
    {"uz", "uzbek"},
    {"fo", "faroese"},
    {"ht", "haitian creole"},
    {"ps", "pashto"},
    {"tk", "turkmen"},
    {"nn", "nynorsk"},
    {"mt", "maltese"},
    {"sa", "sanskrit"},
    {"lb", "luxembourgish"},
    {"my", "myanmar"},
    {"bo", "tibetan"},
    {"tl", "tagalog"},
    {"mg", "malagasy"},
    {"as", "assamese"},
    {"tt", "tatar"},
    {"haw", "hawaiian"},
    {"ln", "lingala"},
    {"ha", "hausa"},
    {"ba", "bashkir"},
    {"jw", "javanese"},
    {"su", "sundanese"},
    {"yue", "cantonese"}
  };
  std::string vocab_path = dir + "/whisper/" + encoding_name + ".tiktoken";
  CoreBPE::HashMap<std::vector<uint8_t>, size_t> ranks = CoreBPE::load_tiktoken_bpe(vocab_path);
  int n_vocab = ranks.size();

  std::vector<std::string> specials;
  specials.push_back("<|endoftext|>");
  specials.push_back("<|startoftranscript|>");
  int i = 0;
  for (std::unordered_map<std::string, std::string>::const_iterator iter = LANGUAGES.begin(); iter != LANGUAGES.end() && i < num_languages; iter++, i++) {
    specials.push_back("<|" + iter->first + "|>");
  }
  specials.push_back("<|translate|>");
  specials.push_back("<|transcribe|>");
  specials.push_back("<|startoflm|>");
  specials.push_back("<|startofprev|>");
  specials.push_back("<|nospeech|>");
  specials.push_back("<|notimestamps|>");
  for (i = 0; i < 1501; ++i) {
    double val = i * 0.02;
    std::ostringstream oss;
    oss << "<|" << std::fixed << std::setprecision(2) << val << "|>";
    specials.push_back(oss.str());
  }

  CoreBPE::StringMap<size_t> special_tokens;
  for (auto token : specials) {
    special_tokens[token] = n_vocab++;
  }
  _encoding = new TikTokenEncoding(
    encoding_name,
    "'s|'t|'re|'ve|'m|'ll|'d| ?\\p{L}+| ?\\p{N}+| ?[^\\s\\p{L}\\p{N}]+|\\s+(?!\\S)|\\s+",
    ranks,
    special_tokens,
    n_vocab
    );
}

WhisperToken::~WhisperToken() {
  delete _encoding;
}

std::vector<size_t> WhisperToken::encode(
  const std::string& text,
  const std::unordered_set<std::string>& allowed_special /*= std::unordered_set<std::string>()*/,
  const std::unordered_set<std::string>& disallowed_special /*= std::unordered_set<std::string>({"all"})*/
  ) {
  return _encoding->encode(text, allowed_special, disallowed_special);
}

/* vim: set expandtab nu ts=2 sw=2 sts=2: */
