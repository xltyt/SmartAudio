#include "web_common.h"
#include <esp.h>
#include <mpr.h>

void webRedirect() {
  char url[1024] = {0};
  if (getQuery()) {
    snprintf(url, _countof(url) - 1, "%s?%s", getUri(), getQuery());
  }
  else {
    snprintf(url, _countof(url) - 1, "%s", getUri());
  }
  redirect(url);
}

int WI(char *key) {
  int default_val = 0;
  const char *param_val = param(key);
  int val = (NULL == param_val ? default_val : atoi(param_val));
  return val;
}

const char *WS(char *key) {
  const char *default_val = "";
  const char *param_val = param(key);
  const char *val = (NULL == param_val ? default_val : param_val);
  return val;
}

int SI(char *key) {
  const char *param_val = getSessionVar(key);
  if (NULL == param_val) {
    return 0;
  }
  return atoi(param_val);
}

const char *SS(char *key) {
  return getSessionVar(key);
}

void SSI(char *key, int val) {
  char tmp[128] = {0};
  snprintf(tmp, sizeof(tmp) - 1, "%d", val);
  espSetSessionVar(getConn(), key, tmp);
}

void SSS(char *key, char *val) {
  espSetSessionVar(getConn(), key, val);
}

int checkLogin() {
  const char *username = getSessionVar("username");
  if (NULL == username || 0 == strlen(username)) {
    char url[1024] = {0};
    if (getQuery()) {
      snprintf(url, _countof(url) - 1, "%s?%s", getUri(), getQuery());
    }
    else {
      snprintf(url, _countof(url) - 1, "%s", getUri());
    }
    char *url_encoded = mprUriEncode(url, MPR_ENCODE_URI_COMPONENT);
    char url_redirect[4096] = {0};
    snprintf(url_redirect, _countof(url_redirect) - 1, "/system/login?url=%s", url_encoded);
    mprRelease(url_encoded);
    redirect(url_redirect);
    return 1;
  }
  return 0;
}

void get_url_info(char *controller_path, int controller_path_len, char *action_path, int action_path_len) {
  char path_tmp[512] = {0};
  strncpy(path_tmp, getUri(), sizeof(path_tmp) - 1);
  char *delim = "/";
  char *token;
  char *rest = path_tmp;
  int index = 0;
  while ((token = strtok_r(rest, delim, &rest))) {
    if (0 == index) {
      strncpy(controller_path, token, controller_path_len - 1);
    }
    else if (1 == index) {
      strncpy(action_path, token, action_path_len - 1);
    }
    else {
      break;
    }
    index++;
  }
}

/* vim: set expandtab ts=2 sw=2 sts=2: */
