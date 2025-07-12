#include "web_init.h"
#include <myappweb.h>
#include <esp.h>
#include "web_common.h"
#include "global.h"

#if 0
static int get_api_port() {
  const char *port_str = getSessionVar("port");
  if (NULL != port_str && 0 != strlen(port_str)) {
    return atoi(port_str);
  }
  return _api_port_internal;
}

static int get_api_https_port() {
  const char *port_str = getSessionVar("port");
  if (NULL != port_str && 0 != strlen(port_str)) {
    return atoi(port_str);
  }
  return _api_https_port_internal;
}
#endif

#include "documents/cache/smart.c"

static void OnPageLoad(void *route) {
  esp_app_smart_combine(route, NULL);
}

int init_web() {
  return start_web(OnPageLoad, 0);
}

void loop_web() {
  stop_web();
}

/* vim: set expandtab ts=2 sw=2 sts=2: */
