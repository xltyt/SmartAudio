/****************************************************\
 *
 * Copyright (C) 2019 All Rights Reserved
 * Last modified: 2025.05.18 19:38:36
 *
\****************************************************/

#ifndef _DISPATCH_H__
#define _DISPATCH_H__

#include <string>

void InitAudio();
void UninitAudio();
int InferAudio(
  const std::string& text,
  const std::string& text_speech,
  const std::string& out_text,
  float speed,
  std::string& result,
  std::string& error
  );

#endif

/* vim: set expandtab nu ts=2 sw=2 sts=2: */
