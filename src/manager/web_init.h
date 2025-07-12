/****************************************************\
 *
 * Copyright (C) 2012 Baidu.com, Inc. All Rights Reserved
 * Last modified: 2019.04.02 03:58:37
 *
\****************************************************/

#ifndef _WEB_INIT_H__
#define _WEB_INIT_H__

#ifdef __cplusplus 
extern "C" { 
#endif
int init_web();
void loop_web();
void set_web_verbose();
void set_web_debug();
#ifdef __cplusplus 
}
#endif

#endif

/* vim: set expandtab ts=4 sw=4 sts=4: */
