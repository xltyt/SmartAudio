/****************************************************\
 *
 * Copyright (C) 2020 360.com, Inc. All Rights Reserved
 * Last modified: 2023.09.01 09:15:18
 *
\****************************************************/

#include <gflags/gflags.h>

DEFINE_bool(debug, true, "");
DEFINE_string(http_host, "127.0.0.1", "");
DEFINE_int32(http_port, 8888, "");
DEFINE_int32(https_port, 8886, "");
DEFINE_int32(http_api_port, 28800, "");
DEFINE_int32(timeout_connect, 1000 * 3, "");
DEFINE_int32(timeout_read, 1000 * 60, "");

DEFINE_string(mysql_sock, "/var/lib/mysql/mysql.sock", "");
DEFINE_string(mysql_host, "127.0.0.1", "");
DEFINE_int32(mysql_port, 3306, "");
DEFINE_string(mysql_user, "root", "");
DEFINE_string(mysql_passwd, "dJbkLpObg2f737uQtNTqhNfrkTTmX6eF", "");
DEFINE_string(mysql_dbname, "smart", "");
DEFINE_string(mysql_charset, "utf8", "");

/* vim: set expandtab nu ts=2 sw=2 sts=2: */
