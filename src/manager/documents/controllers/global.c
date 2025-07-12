/*
  global Controller (esp-html-skeleton)
 */
#include "esp.h"

static void commonGlobal(HttpConn *conn) {
}

/*
  Redirect to get a properly terminated URL. Essential for relative URL references.
 */
static void redirectGlobal() {
  redirect(sjoin(getUri(), "/app", NULL));
}

static void appGlobal() {
  if (checkLogin()) return;
  renderView("global/global_app");
}

static void uploadGlobal() {
  char client_filename[128] = {0};
  char save_filename[128] = {0};
  char filepath[128] = {0};
  MprList *files = getUploads();
  HttpUploadFile *file;
  int index;
  for (ITERATE_ITEMS(files, file, index)) {
    if (0 == strcmp(file->name, "audio")) {
      if (file->filename) {
        strcpy(filepath, file->filename);
      }
      if (file->clientFilename) {
        strcpy(client_filename, file->clientFilename);
      }
    }
  }
  {
    int ret = global_upload_file(client_filename, filepath, save_filename);
    const char *json_output_str;
    json_object *json_root = json_object_new_object();
    json_object_object_add(json_root, "status", json_object_new_int(ret));
    json_object_object_add(json_root, "filename", json_object_new_string(save_filename));
    json_output_str = json_object_to_json_string(json_root);
    render("%s", NULL == json_output_str ? "" : json_output_str);
    json_object_put(json_root);
  }
}

static void templateGlobal() {
  const char *param_method = param("method");
  if (NULL != param_method) {
    if (0 == strcmp(param_method, "add")) {
      int ret = -1;
      int template_id = -1;
      int affected = mysqlite_exec_dml("INSERT INTO template(\"name\",\"content\",\"filename\",\"user_id\") VALUES(\"%s\",\"%s\",\"%s\",%d)",
				WS("name"),
        WS("content"),
        WS("filename"),
        WI("user_id")
      );
      if (affected > 0) {
        ret = 0;
        template_id = mysqlite_last_row_id();
      }   
      const char *json_output_str;
      json_object *json_root = json_object_new_object();
      json_object_object_add(json_root, "status", json_object_new_int(ret));
      json_object_object_add(json_root, "template_id", json_object_new_int(template_id));
      json_output_str = json_object_to_json_string(json_root);
      render("%s", NULL == json_output_str ? "" : json_output_str);
      json_object_put(json_root);
      return;
    }
    else if (0 == strcmp(param_method, "delete")) {
			int ret = -1;
			int affected = mysqlite_exec_dml("DELETE FROM template WHERE \"id\"=%d", WI("template_id"));
      if (affected > 0) {
        ret = 0;
      }
      const char *json_output_str;
      json_object *json_root = json_object_new_object();
      json_object_object_add(json_root, "status", json_object_new_int(ret));
      json_output_str = json_object_to_json_string(json_root);
      render("%s", NULL == json_output_str ? "" : json_output_str);
      json_object_put(json_root);
      return;
    }
  }
}

static void generateGlobal() {
  char audio_name[512] = {0};
  int ret = global_generate(WI("user_id"), WI("template_id"), WS("content"), audio_name);
  const char *json_output_str;
  json_object *json_root = json_object_new_object();
  json_object_object_add(json_root, "status", json_object_new_int(ret));
  json_object_object_add(json_root, "audio_name", json_object_new_string(audio_name));
  json_output_str = json_object_to_json_string(json_root);
  render("%s", NULL == json_output_str ? "" : json_output_str);
  json_object_put(json_root);
}

/*
  Dynamic module initialization
 */
ESP_EXPORT int esp_controller_smart_global(HttpRoute *route, MprModule *module) {
  espDefineBase(route, commonGlobal);
  espDefineAction(route, "global/", redirectGlobal);
  espDefineAction(route, "global", redirectGlobal);
  espDefineAction(route, "global/app", appGlobal);
  espDefineAction(route, "global/template_upload", uploadGlobal);
  espDefineAction(route, "global/template", templateGlobal);
  espDefineAction(route, "global/generate", generateGlobal);
  return 0;
}

/* vim: set expandtab ts=2 sw=2 sts=2: */
