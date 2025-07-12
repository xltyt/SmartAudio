/****************************************************\
 *
 * Copyright (C) 2019 All Rights Reserved
 * Last modified: 
 *
\****************************************************/

#include "global.h"
#include <unistd.h>
#include <string.h>
#include <glog/logging.h>
#include <system_utils.h>
#include <crypt_utils.h>
#include <net_utils.h>
#include <sqlite_common.h>
#include <timer.h>
#include <base64.h>
#include <common.h>
#include "flags.h"

extern "C" int global_upload_file(const char *client_filename, const char *filepath, char *save_filename) {
  std::string new_name = "template_" + Crypt::gen_random_string(16) + ".mp3";
  std::string path = mycommon::get_current_dir() + "/data/" + new_name;
  unlink(path.c_str());
  int ret = rename(filepath, path.c_str());
  strcpy(save_filename, new_name.c_str());
  LOG(INFO) << "Upload Ori[" << filepath << "] New[" << path << "] Ret[" << ret << "] Err[" << errno << "]";
  return ret;
}

extern "C" int global_generate(int user_id, int template_id, const char *content, char *audio_name) {
  std::string template_content;
  std::string template_speech_name;
  {
    mysqlite_query_param_t *param = mysqlite_exec_query("SELECT * FROM template WHERE id=%d", template_id);
    if (param) {
      while (!mysqlite_query_is_eof(param)) {
        int template_id = mysqlite_query_get_int_field(param, "id", 0);
        const char *content = mysqlite_query_get_string_field(param, "content", "");
        const char *filename = mysqlite_query_get_string_field(param, "filename", "");
        template_content = content;
        template_speech_name = filename;
        mysqlite_query_next_row(param);
      }
      mysqlite_query_close(param);
    }
  }
  if (template_content.empty()) {
    LOG(WARNING) << "Generate [" << template_id << "] Failed, Empty";
    return -1;
  }
	LOG(INFO) << "Generate Id[" << template_id << "] TplContent[" << template_content << "] Content[" << content << "]";
  std::string speech_path = mycommon::get_current_dir() + "/data/" + template_speech_name;
  std::string speech;
  int ret = mycommon::file_read(speech_path, speech);
  if (0 != ret) {
    LOG(WARNING) << "Generate [" << template_id << "] Failed, Read Speech[" << speech_path << "] Failed";
    return -1;
  }
  speech = common::base64_encode(speech);
  
  std::string url = "http://127.0.0.1:" + std::to_string(FLAGS_http_api_port) + "/audio";
  std::string req_body;
  {
    rapidjson::Document doc_result;
    rapidjson::Document::AllocatorType& allocator_result = doc_result.GetAllocator();
    rapidjson::Value json_val_result_obj(rapidjson::kObjectType);
    JSON_ADD_STRING(json_val_result_obj, text, "text", template_content);
    JSON_ADD_STRING(json_val_result_obj, text_speech, "text_speech", speech);
    JSON_ADD_STRING(json_val_result_obj, out_text, "out_text", std::string(content));
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
    writer.SetMaxDecimalPlaces(3);
    json_val_result_obj.Accept(writer);
    req_body = sb.GetString();
  }
  LOG(INFO) << "Request Uri[" << url << "] Req[" << req_body << "]";
  long http_code = -1;
  std::string resp_body;
  uint64_t time_start = mycommon::getMilliTime();
  std::vector<std::string> headers;
  headers.push_back("Content-Type: application/json");
  ret = mycommon::net_post_url(
    url,
    req_body,
    resp_body,
    NULL,
    &headers,
    NULL,
    NULL,
    FLAGS_timeout_connect,
    FLAGS_timeout_read,
    &http_code,
    NULL
    );
  if (0 != ret) {
    LOG(WARNING) << "Request Failed[" << ret << "] Req[" << req_body << "] Time[" << (mycommon::getMilliTime() - time_start) << "]";
    return -1;
  }
  LOG(INFO) << "Request Req[" << req_body << "] Time[" << (mycommon::getMilliTime() - time_start) << "] Resp[" << resp_body << "]";
  {
    rapidjson::Document doc;
    rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
    rapidjson::ParseResult ok = doc.Parse(resp_body.c_str());
    if (!ok || !doc.IsObject()) {
      LOG(WARNING) << "Parse Failed";
      return -1;
    }
    int status = GET_JSON_INT(doc, "status", -1);
    std::string content = GET_JSON_STRING(doc, "data", "");
    if (0 != status || content.empty()) {
      LOG(WARNING) << "Generate Failed";
      return -2;
    }
    content = common::base64_decode(content);
    std::string result_name = "result_" + Crypt::gen_random_string(16) + ".wav";
    std::string result_path = mycommon::get_current_dir() + "/data/" + result_name;
    mycommon::file_write(result_path, content);
    strcpy(audio_name, result_name.c_str());
  }
  mysqlite_exec_dml("INSERT INTO generate(\"template_id\",\"content\",\"user_id\",\"time\") VALUES(%d,\"%s\",%d,\"%s\")",
		template_id,
    content,
    user_id,
    mycommon::getSafeNow().c_str()
  );
  return 0;
}

/* vim: set expandtab nu ts=2 sw=2 sts=2: */
