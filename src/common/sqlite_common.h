/****************************************************\
 *
 * Copyright (C) 2012 Baidu.com, Inc. All Rights Reserved
 * Last modified: 2021.04.02 16:16:04
 *
\****************************************************/

#ifndef _SQLITE_COMMON_H__
#define _SQLITE_COMMON_H__

#include <stdint.h>

typedef struct sqlite3_stmt sqlite3_stmt;
#ifdef __cplusplus 
extern "C" { 
#endif
typedef struct {
  sqlite3_stmt *_pVM;
  int _bEof;
  int _nCols;
}
mysqlite_query_param_t;
int mysqlite_init(const char *path);
int mysqlite_exec_dml(const char *format, ...);
int mysqlite_exec_scalar(const char *sql, int nNullValue);
int mysqlite_exec_scalar_string(const char *sql, const char* nNullValue, char *val, int val_len);
// bool
int mysqlite_table_exist(const char *table);
int mysqlite_last_row_id();
mysqlite_query_param_t* mysqlite_exec_query(const char *format, ...);
// bool
int mysqlite_query_is_eof(mysqlite_query_param_t *param);
void mysqlite_query_next_row(mysqlite_query_param_t *param);
const char *mysqlite_query_get_string_field_by_index(mysqlite_query_param_t *param, int nField, const char *szNullValue);
const char *mysqlite_query_get_string_field(mysqlite_query_param_t *param, const char *szField, const char *szNullValue);
int mysqlite_query_get_int_field_by_index(mysqlite_query_param_t *param, int nField, int nNullValue);
int mysqlite_query_get_int_field(mysqlite_query_param_t *param, const char *szField, int nNullValue);
int64_t mysqlite_query_get_int64_field_by_index(mysqlite_query_param_t *param, int nField, int64_t nNullValue);
int64_t mysqlite_query_get_int64_field(mysqlite_query_param_t *param, const char *szField, int64_t nNullValue);
void mysqlite_query_close(mysqlite_query_param_t *param);
char *mysqlite_error();
void mysqlite_close();
#ifdef __cplusplus 
}
#endif

#endif

/* vim: set expandtab ts=2 sw=2 sts=2: */
