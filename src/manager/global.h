/****************************************************\
 *
 * Copyright (C) 2019 All Rights Reserved
 * Last modified: 
 *
\****************************************************/

#ifndef _GLOBAL_H__
#define _GLOBAL_H__

#ifdef __cplusplus 
extern "C" { 
#endif

int global_upload_file(const char *client_filename, const char *filepath, char *save_filename);
int global_generate(int user_id, int template_id, const char *content, char *audio_name);

#ifdef __cplusplus 
}
#endif


#endif

/* vim: set expandtab nu ts=2 sw=2 sts=2: */
