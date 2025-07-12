#include <glog/logging.h>
#include <sqlite_common.h>
#include <net_utils.h>
#include <system_utils.h>
#include <string_utils.h>
#include <file_utils.h>
#include "web_init.h"
#include "flags.h"
#include "common_version.h"

#define CONFIG_PATH "etc/manager.bin"

int main(int argc, char *argv[]) {
  gflags::SetVersionString(APP_VERSION);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  //std::string flags_path = mycommon::get_current_dir() + "/../etc/proxy_manager.ini";
  //gflags::ReadFromFlagsFile(flags_path.c_str(), argv[0], false);
  
  FLAGS_log_dir = "logs";
  FLAGS_logbuflevel = -1;
#if 0
  int debug_mode = -1; 
  int opt;
  char *param_string = "hd";
  while ((opt = getopt(argc, argv, param_string))!= -1) {
    switch (opt) {
    case 'h':
      break;
    case 'd':
      debug_mode = 0;
      break;
    default:
      break;
    }
  }
  FLAGS_minloglevel = (0 == debug_mode ? google::INFO : google::FATAL);
#else
  FLAGS_minloglevel = (FLAGS_debug ? google::INFO : google::FATAL);
#endif
  google::InitGoogleLogging("main");
  FLAGS_stderrthreshold = google::FATAL;
  FLAGS_alsologtostderr = false;
  FLAGS_colorlogtostderr = true;
  LOG(INFO) << "Start...";

  std::string cfg_content = mycommon::file_read("etc/web_ori.conf");
  mycommon::str_replace(cfg_content, "{ROOT_DIR}", mycommon::get_current_dir());
  mycommon::str_replace(cfg_content, "{ROOT_PORT}", std::to_string(FLAGS_http_port));
  mycommon::str_replace(cfg_content, "{ROOT_SSL_PORT}", std::to_string(FLAGS_https_port));
  mycommon::file_write("etc/web.conf", cfg_content);

  mycommon::net_init();
  int ret;
  // Init Sql
  ret = mysqlite_init(CONFIG_PATH);
  if (0 != ret) {
    LOG(WARNING) << "Init Sql Failed";
    return 1;
  }
  LOG(INFO) << "Init Sql Success";

  // Init Web
  ret = init_web();
  if (0 != ret) {
    LOG(WARNING) << "Init Web Failed";
    return 1;
  }
  LOG(INFO) << "Init Web Success";
  loop_web();

  return 0;
}

/* vim: set expandtab ts=2 sw=2 sts=2: */
