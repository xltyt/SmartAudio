/*
  system Controller (esp-html-skeleton)
 */
#include "esp.h"

static void commonSystem(HttpConn *conn) {
}

/*
  Redirect to get a properly terminated URL. Essential for relative URL references.
 */
static void redirectSystem() {
  redirect(sjoin(getUri(), "/user", NULL));
}

static void loginSystem() {
  const char *param_method = param("method");
  if (NULL != param_method) {
    if (0 == strcmp(param_method, "login")) {
      int user_id = 0;
      int count = 0;
      int type = 0;
      mysqlite_query_param_t *param = mysqlite_exec_query("SELECT * FROM system_user WHERE \"user\"=\"%s\" AND \"passwd\"=\"%s\"", WS("username"), WS("passwd"));
      if (param) {
        while (!mysqlite_query_is_eof(param)) {
          count++;
          user_id = mysqlite_query_get_int_field(param, "id", 0);
          type = mysqlite_query_get_int_field(param, "type", 0);
          mysqlite_query_next_row(param);
        }
        mysqlite_query_close(param);
      }
      if (count > 0) {
        espSetSessionVar(getConn(), "username", WS("username"));
        espSetSessionVar(getConn(), "port", WS("port"));
        SSI("USER_TYPE", type);
        SSI("USER_ID", user_id);
        if (0 == strlen(WS("url"))) {
          redirect("/global");
        }
        else {
          redirect(WS("url"));
        }
        return;
      }
      webRedirect();
      return;
    }
  }
  {
    const char *username = getSessionVar("username");
    if (NULL == username || 0 == strlen(username)) {
      renderView("system/login");
    }
    else {
      redirect("/global");
    }
  }
}

static void logoutSystem() {
  removeSessionVar("username");
  removeSessionVar("port");
  //redirect("/system/login");
  
  const char *json_output_str;
  json_object *json_root = json_object_new_object();
  json_object_object_add(json_root, "status", json_object_new_int(0));
  json_output_str = json_object_to_json_string(json_root);
  render("%s", NULL == json_output_str ? "" : json_output_str);
  json_object_put(json_root);
}

static void userSystem() {
  const char *param_method = param("method");
  if (NULL != param_method) {
    if (0 == strcmp(param_method, "add")) {
      int ret = -1;
      int user_id = -1;
      int affected = mysqlite_exec_dml("INSERT INTO system_user(\"type\",\"user\",\"passwd\",\"mail\") VALUES(%d,\"%s\",\"%s\",\"%s\")",
        WI("type"),
				WS("name"),
        WS("passwd"),
        WS("mail")
        );
      if (affected > 0) {
        ret = 0;
        user_id = mysqlite_last_row_id();
      }   
      const char *json_output_str;
      json_object *json_root = json_object_new_object();
      json_object_object_add(json_root, "status", json_object_new_int(ret));
      json_object_object_add(json_root, "user_id", json_object_new_int(user_id));
      json_output_str = json_object_to_json_string(json_root);
      render("%s", NULL == json_output_str ? "" : json_output_str);
      json_object_put(json_root);
      return;
    }
    else if (0 == strcmp(param_method, "edit")) {
      int ret = -1;
      int affected;
      if (0 == strlen(WS("passwd"))) {
        affected = mysqlite_exec_dml("UPDATE system_user SET \"user\"=\"%s\", \"type\"=%d, \"mail\"=\"%s\" WHERE \"id\"=%d",
          WS("name"),
          WI("type"),
          WS("mail"),
          WI("user_id")
          );
      }   
      else {
        affected = mysqlite_exec_dml("UPDATE system_user SET \"user\"=\"%s\", \"passwd\"=\"%s\", \"type\"=%d, \"mail\"=\"%s\" WHERE \"id\"=%d",
          WS("name"),
          WS("passwd"),
          WI("type"),
          WS("mail"),
          WI("user_id")
          );
      }
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
    else if (0 == strcmp(param_method, "delete")) {
			int ret = -1;
			int affected = mysqlite_exec_dml("DELETE FROM system_user WHERE \"id\"=%d", WI("user_id"));
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

/*
  Dynamic module initialization
 */
ESP_EXPORT int esp_controller_smart_system(HttpRoute *route, MprModule *module) {
  espDefineBase(route, commonSystem);
  espDefineAction(route, "system/", redirectSystem);
  espDefineAction(route, "system", redirectSystem);
  espDefineAction(route, "system/login", loginSystem);
  espDefineAction(route, "system/logout", logoutSystem);
  espDefineAction(route, "system/user", userSystem);
  return 0;
}

/* vim: set expandtab ts=2 sw=2 sts=2: */
