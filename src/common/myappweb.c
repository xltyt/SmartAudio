#include "myappweb.h"
#include <appweb.h>

/*
  Global application object. Provides the top level roots of all data objects for the GC.
 */
typedef struct AppwebApp {
}
AppwebApp;
static AppwebApp *app;
static void manageApp(AppwebApp *app, int flags) {
}

int start_web(LPWEB_RUN_CUSTOM_PAGE pCall, int loglevel) {
  Mpr *mpr;
  HttpHost  *host;
  int next;
  //int workers = -1;
  char *ip = NULL;
  int port = ME_HTTP_PORT;
  //int secure;
  //int loglevel = 5;
  //int tracelevel = 5;
  int tracelevel = loglevel;
  
  char home[1024];
  char documents[1024];
  char configfile[1024];
  char logfile[1024];
  char tracefile[1024];
  //getcwd(home, sizeof(home));
  //int i;
  int rslt = readlink("/proc/self/exe", home, sizeof(home) - 1);
  if (rslt < 0 || (rslt >= sizeof(home) - 1)) {
    return -1;
  }
  home[rslt] = '\0';
  char *p = strrchr(home, '/');
  if (NULL != p) {
    *p = 0;
  }
  fprintf(stderr, "Home[%s]\n", home);
  sprintf(documents, "%s/documents/", home);
  sprintf(configfile, "%s/etc/web.conf", home);
  sprintf(logfile, "%s/log/log.log:%d", home, loglevel);
  sprintf(tracefile, "%s/log/trace.log:%d", home, tracelevel);

  //int argc = 1;
  //char *argv[0] = {home};
  int argc = 0;
  char *argv[2] = {NULL};
  if ((mpr = mprCreate(argc, argv, 0)) == NULL) {
    return 1;
  }
  if ((app = mprAllocObj(AppwebApp, manageApp)) == NULL) {
    return 2;
  }
  mprAddRoot(app);
  mprAddStandardSignals();
  if (httpCreate(HTTP_CLIENT_SIDE | HTTP_SERVER_SIDE) == 0) {
    return 3;
  }
  //mprStartLogging("stdout:5", MPR_LOG_CONFIG | MPR_LOG_CMDLINE);
  //httpStartTracing("stdout:5");
  //mprSetLogLevel(5);
  //httpSetTraceLevel(5);
  mprStartLogging(logfile, MPR_LOG_CONFIG | MPR_LOG_CMDLINE);
  httpStartTracing(tracefile);
  if (mprStart() < 0) {
    mprLog("error appweb", 0, "Cannot start MPR");
    mprDestroy();
    return MPR_ERR_CANT_INITIALIZE;
  }
  mprGC(MPR_GC_FORCE | MPR_GC_COMPLETE);
  int ret = maConfigureServer(configfile, /*documents*/home, documents, ip, port);
  if (ret < 0) {
    fprintf(stderr, "maConfigureServer Failed[%d]\n", ret);
    return MPR_ERR_CANT_CREATE;
  }
  mprGC(MPR_GC_FORCE | MPR_GC_COMPLETE);
  
  httpSetInfoLevel(0);
  if (httpStartEndpoints() < 0) {
    mprLog("error appweb", 0, "Cannot listen on HTTP endpoints, exiting.");
    exit(9);
  }

  for (ITERATE_ITEMS(HTTP->hosts, host, next)) {
    httpLogRoutes(host, 1 > 1); 
    //HttpRoute *route = httpGetDefaultRoute(host);
    //esp_app_smart_combine(route, NULL);
  }

  /*
    Invoke ESP initializers here
   */
  //HttpRoute *route = getRoute();
  HttpRoute *route = httpGetDefaultRoute(NULL);
  pCall(route);

  return 0;
}

int stop_web() {
  /*
    Events thread will service requests. We block here.
   */
  mprYield(MPR_YIELD_STICKY);
  while (!mprIsStopping()) {
    mprSuspendThread(-1);
  }
  mprResetYield();

  mprLog("info appweb", 1, "Stopping Appweb ...");
  mprDestroy();
  return mprGetExitStatus();
}

/* vim: set expandtab nu smartindent ts=2 sw=2 sts=2: */
