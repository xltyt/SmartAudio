/****************************************************\
 *
 * Copyright (C) 2012 Baidu.com, Inc. All Rights Reserved
 * Last modified: 2023.06.03 11:42:24
 *
\****************************************************/

#ifndef _WEB_COMMON_H__
#define _WEB_COMMON_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <json-c/json.h>
#include <sqlite_common.h>

void webRedirect();

int WI(char *key);
const char *WS(char *key);
int SI(char *key);
const char *SS(char *key);
void SSI(char *key, int val);
void SSS(char *key, char *val);

#define USER_TYPE_ADMIN     0
#define USER_TYPE_USER      1
#define USER_TYPE_INNER     2

#ifdef __cplusplus 
#define EXTERN_C extern "C"
#else
#define EXTERN_C
#endif

#ifndef SAFE_FREE
#define SAFE_FREE(ptr) if (ptr) { free(ptr); ptr = NULL; }
#endif

#ifndef _countof
#define _countof(_Array) (int)(sizeof(_Array) / sizeof(_Array[0]))
#endif

int checkLogin();
void get_url_info(char *controller_path, int controller_path_len, char *action_path, int action_path_len);

#endif

/* vim: set expandtab ts=2 sw=2 sts=2: */
