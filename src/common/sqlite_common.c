#include "sqlite_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define SQLITE_HAS_CODEC
#include <sqlcipher/sqlite3.h>

#define SQLITE_KEY          "etISzn01z4w7BbQnTYZIShPaXQBq37Ld"

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef _countof
#define _countof(_Array) (int)(sizeof(_Array) / sizeof(_Array[0]))
#endif

static sqlite3 *_mysqlite_db = NULL;
static char _mysqlite_error[1024];

int mysqlite_init(const char *path) {
  int ret = sqlite3_open(path, &_mysqlite_db);
  if (ret != SQLITE_OK) {
    const char *err = sqlite3_errmsg(_mysqlite_db);
    strncpy(_mysqlite_error, err, _countof(_mysqlite_error) - 1);
    _mysqlite_error[_countof(_mysqlite_error) - 1] = 0;
    return -1;
  }
  sqlite3_key(_mysqlite_db, SQLITE_KEY, strlen(SQLITE_KEY));
  sqlite3_busy_timeout(_mysqlite_db, 60000);
  return 0;
}

int mysqlite_exec_dml(const char *format, ...) {
  if (NULL == _mysqlite_db) {
    return -1;
  }

  //char sql[1024] = {0};
  va_list args;
  va_start(args, format);
  //vsnprintf(sql, _countof(sql) - 1, format, args);
  //sqlite3_vsnprintf(sql, _countof(sql) - 1, format, args);
  char *sql = sqlite3_vmprintf(format, args);
  va_end(args);
  //fprintf(stdout, "mysqlite_exec_dml[%s]\n", sql);

  char *err = 0;
  int ret = sqlite3_exec(_mysqlite_db, sql, 0, 0, &err);
  if (sql) {
    sqlite3_free(sql);
    sql = NULL;
  }
  if (ret == SQLITE_OK) {
    return sqlite3_changes(_mysqlite_db);
  }
  else {
    if (err) {
      strncpy(_mysqlite_error, err, _countof(_mysqlite_error) - 1);
      _mysqlite_error[_countof(_mysqlite_error) - 1] = 0;
    }
    return -2;
  }
}

int mysqlite_exec_scalar(const char *sql, int nNullValue) {
  if (NULL == _mysqlite_db) {
    return -1;
  }

  mysqlite_query_param_t *param = mysqlite_exec_query(sql);
  if (NULL == param) {
    return -3;
  }
  if (param->_bEof || param->_nCols < 1) {
    mysqlite_query_close(param);
    return -2;
  }

  int ret = mysqlite_query_get_int_field_by_index(param, 0, nNullValue);
  mysqlite_query_close(param);
  return ret;
}

int mysqlite_exec_scalar_string(const char *sql, const char* nNullValue, char *val, int val_len) {
  if (NULL == _mysqlite_db) {
    return -1;
  }

  mysqlite_query_param_t *param = mysqlite_exec_query(sql);
  if (NULL == param) {
    return -3;
  }
  if (param->_bEof || param->_nCols < 1) {
    mysqlite_query_close(param);
    return -2;
  }

  const char *buf = mysqlite_query_get_string_field_by_index(param, 0, nNullValue);
  strncpy(val, buf, val_len - 1);
  val[val_len - 1] = 0;
  mysqlite_query_close(param);
  return 0;
}

int mysqlite_table_exist(const char *table) {
  char sql[1024];
  sprintf(sql, "select count(*) from mysqlite_master where type='table' and name='%s'", table);
  int ret = mysqlite_exec_scalar(sql, 0);
  return ret > 0 ? TRUE : FALSE;
}

int mysqlite_last_row_id() {
  return sqlite3_last_insert_rowid(_mysqlite_db);
}

static sqlite3_stmt *compile(const char *szSQL) {
  if (NULL == _mysqlite_db) {
    return NULL;
  }

  const char *szTail=0;
  sqlite3_stmt* pVM;
  int nRet = sqlite3_prepare_v2(_mysqlite_db, szSQL, -1, &pVM, &szTail);

  if (nRet != SQLITE_OK) {
    const char *err = sqlite3_errmsg(_mysqlite_db);
    strncpy(_mysqlite_error, err, _countof(_mysqlite_error) - 1);
    _mysqlite_error[_countof(_mysqlite_error) - 1] = 0;
    return NULL;
  }

  return pVM;
}

static int fieldIndex(mysqlite_query_param_t *param, const char *szField) {
  if (szField) {
    int nField;
    for (nField = 0; nField < param->_nCols; nField++) {
      const char *szTemp = sqlite3_column_name(param->_pVM, nField);
      if (strcmp(szField, szTemp) == 0) {
        return nField;
      }
    }
  }

  return -2;
}

static int fieldDataType(mysqlite_query_param_t *param, int nCol) {
  if (nCol < 0 || nCol > param->_nCols - 1) {
    return SQLITE_NULL;
  }

  return sqlite3_column_type(param->_pVM, nCol);
}

mysqlite_query_param_t* mysqlite_exec_query(const char *format, ...) {
  if (NULL == _mysqlite_db) {
    return NULL;
  }

  //char sql[1024] = {0};
  va_list args;
  va_start(args, format);
  //vsnprintf(sql, _countof(sql) - 1, format, args);
  //sqlite3_vsnprintf(sql, _countof(sql) - 1, format, args);
  char *sql = sqlite3_vmprintf(format, args);
  va_end(args);

  sqlite3_stmt* pVM = compile(sql);
  if (sql) {
    sqlite3_free(sql);
    sql = NULL;
  }
  int ret = sqlite3_step(pVM);
  if (ret == SQLITE_DONE) {
    // no rows
    mysqlite_query_param_t *param = (mysqlite_query_param_t *)malloc(sizeof(mysqlite_query_param_t));
    param->_pVM = pVM;
    param->_bEof = TRUE;
    param->_nCols = sqlite3_column_count(pVM);
    return param;
  }
  else if (ret == SQLITE_ROW) {
    // at least 1 row
    mysqlite_query_param_t *param = (mysqlite_query_param_t *)malloc(sizeof(mysqlite_query_param_t));
    param->_pVM = pVM;
    param->_bEof = FALSE;
    param->_nCols = sqlite3_column_count(pVM);
    return param;
  }
  else {
    const char *err = sqlite3_errmsg(_mysqlite_db);
    strncpy(_mysqlite_error, err, _countof(_mysqlite_error) - 1);
    _mysqlite_error[_countof(_mysqlite_error) - 1] = 0;
    sqlite3_finalize(pVM);
    return NULL;
  }
}

int mysqlite_query_is_eof(mysqlite_query_param_t *param) {
  return param->_bEof;
}

void mysqlite_query_next_row(mysqlite_query_param_t *param) {
  int ret = sqlite3_step(param->_pVM);
  if (ret == SQLITE_DONE) {
    // no rows
    param->_bEof = TRUE;
  }
  else if (ret == SQLITE_ROW) {
    // more rows, nothing to do
  }
  else {
    const char *err = sqlite3_errmsg(_mysqlite_db);
    strncpy(_mysqlite_error, err, _countof(_mysqlite_error) - 1);
    _mysqlite_error[_countof(_mysqlite_error) - 1] = 0;
    sqlite3_finalize(param->_pVM);
    param->_pVM = NULL;
  }
}

const char *mysqlite_query_get_string_field_by_index(mysqlite_query_param_t *param, int nField, const char *szNullValue) {
  if (fieldDataType(param, nField) == SQLITE_NULL) {
    return szNullValue;
  }
  else {
    return (const char*)sqlite3_column_text(param->_pVM, nField);
  }
}
const char *mysqlite_query_get_string_field(mysqlite_query_param_t *param, const char *szField, const char *szNullValue) {
  int nField = fieldIndex(param, szField);
  return mysqlite_query_get_string_field_by_index(param, nField, szNullValue);
}

int mysqlite_query_get_int_field_by_index(mysqlite_query_param_t *param, int nField, int nNullValue) {
  if (fieldDataType(param, nField) == SQLITE_NULL) {
    return nNullValue;
  }
  else {
    return sqlite3_column_int(param->_pVM, nField);
  }
}
int mysqlite_query_get_int_field(mysqlite_query_param_t *param, const char *szField, int nNullValue) {
  int nField = fieldIndex(param, szField);
  return mysqlite_query_get_int_field_by_index(param, nField, nNullValue);
}

int64_t mysqlite_query_get_int64_field_by_index(mysqlite_query_param_t *param, int nField, int64_t nNullValue) {
  if (fieldDataType(param, nField) == SQLITE_NULL) {
    return nNullValue;
  }
  else {
    return sqlite3_column_int64(param->_pVM, nField);
  }
}
int64_t mysqlite_query_get_int64_field(mysqlite_query_param_t *param, const char *szField, int64_t nNullValue) {
  int nField = fieldIndex(param, szField);
  return mysqlite_query_get_int64_field_by_index(param, nField, nNullValue);
}

void mysqlite_query_close(mysqlite_query_param_t *param) {
  if (param->_pVM) {
    int ret = sqlite3_finalize(param->_pVM);
    param->_pVM = NULL;
    if (ret != SQLITE_OK) {
      const char *err = sqlite3_errmsg(_mysqlite_db);
      strncpy(_mysqlite_error, err, _countof(_mysqlite_error) - 1);
      _mysqlite_error[_countof(_mysqlite_error) - 1] = 0;
    }
  }
  free(param);
}

char *mysqlite_error() {
  return _mysqlite_error;
}

void mysqlite_close() {
  if (_mysqlite_db) {
    if (sqlite3_close(_mysqlite_db) == SQLITE_OK) {
      _mysqlite_db = NULL;
    }
    else {
    }
  }
}

/* vim: set expandtab ts=2 sw=2 sts=2: */
