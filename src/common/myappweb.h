/****************************************************\
 *
 * Copyright (C) 2012 Baidu.com, Inc. All Rights Reserved
 * Last modified: 2019.04.11 04:36:19
 *
\****************************************************/

#ifndef _MYAPPWEB_H__
#define _MYAPPWEB_H__

typedef void (LPWEB_RUN_CUSTOM_PAGE)(void *route);

int start_web(LPWEB_RUN_CUSTOM_PAGE pCall, int loglevel);
int stop_web();

#endif

/* vim: set expandtab nu smartindent ts=2 sw=2 sts=2: */
