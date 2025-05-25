/****************************************************\
 *
 * Copyright (C) 2019 All Rights Reserved
 * Last modified: 2025.05.13 20:13:45
 *
\****************************************************/

#ifndef _AUDIO_UTILS_H__
#define _AUDIO_UTILS_H__

#include <string>
#include <vector>

int ConvertToWav(const std::string& path, const std::string& output_path);
int GetWavInfo(const std::string& path, int& nchannels, int& sampwidth, int& framerate, int& nframes, int& duration);
int LoadWav(const std::string& path, int target_sr, std::vector<float>& result, int& channels);
int SaveWav(const std::string& path, int target_sr, const std::vector<float>& data, int channels);

#endif

/* vim: set expandtab nu ts=2 sw=2 sts=2: */
