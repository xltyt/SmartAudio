/****************************************************\
 *
 * Copyright (C) 2020 360.com, Inc. All Rights Reserved
 * Last modified: 2023.09.01 09:15:25
 *
\****************************************************/

#ifndef _FLAGS_H__
#define _FLAGS_H__

#include <gflags/gflags.h>

DECLARE_bool(debug);
DECLARE_string(http_host);
DECLARE_int32(http_port);
DECLARE_int32(https_port);
DECLARE_int32(http_api_port);
DECLARE_int32(timeout_connect);
DECLARE_int32(timeout_read);

DECLARE_string(mysql_sock);
DECLARE_string(mysql_host);
DECLARE_int32(mysql_port);
DECLARE_string(mysql_user);
DECLARE_string(mysql_passwd);
DECLARE_string(mysql_dbname);
DECLARE_string(mysql_charset);

#endif

/* vim: set expandtab nu ts=2 sw=2 sts=2: */
