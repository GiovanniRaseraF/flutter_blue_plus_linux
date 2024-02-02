#ifndef FLUTTER_MY_APPLICATION_H_
#define FLUTTER_MY_APPLICATION_H_

#include <gtk/gtk.h>
#include <math.h>
#include <gmodule.h>
#include <cstring>
#include <sys/utsname.h>
#include <simplebluez/Bluez.h>
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>
#include <map>

G_DECLARE_FINAL_TYPE(MyApplication, my_application, MY, APPLICATION,
                     GtkApplication)

/**
 * my_application_new:
 *
 * Creates a new Flutter-based application.
 *
 * Returns: a new #MyApplication.
 */
MyApplication *my_application_new();

// global
#define FL_STR_RESPONSE(_str) FL_METHOD_RESPONSE(fl_method_success_response_new(fl_value_new_string((_str))))
#define FL_BOOL_RESPONSE(_bl) FL_METHOD_RESPONSE(fl_method_success_response_new(fl_value_new_bool((_bl))))
#define FL_INT_RESPONSE(_in) FL_METHOD_RESPONSE(fl_method_success_response_new(fl_value_new_int((_in))))
#define FL_FLOT_RESPONSE(_in) FL_METHOD_RESPONSE(fl_method_success_response_new(fl_value_new_float((_in))))
#define FL_NOIMP_RESPONSE FL_METHOD_RESPONSE(fl_method_not_implemented_response_new())
#define FL_RESP(resp) FL_METHOD_RESPONSE(fl_method_success_response_new(resp))

#define FL_STR(_str) (fl_value_new_string((_str)))
#define FL_INT(_int) (fl_value_new_int((_int)))
#define FL_BOOL(_b) (fl_value_new_bool((_b)))
#define FL_FLOAT(_f) (fl_value_new_float((_f)))
#define FL_MAP fl_value_new_map()
#define FL_MAP_SET(_map, _str, _val) fl_value_set_string_take(_map, _str, _val)
#define FL_LIST fl_value_new_list()
#define FL_APPEND(_list, _val) fl_value_append_take(_list, _val)

#define FL_ARGS(_m_call) fl_method_call_get_args(_m_call)
#define TO_STR(newv) fl_value_to_string((newv))

// send data to UI
#define TO_UI(method_name, value_to_send) \
fl_method_channel_invoke_method(flutter_blue_plus_plugin_channel, \
  (method_name), \
  (value_to_send), \
  nullptr, nullptr, nullptr)

#endif // FLUTTER_MY_APPLICATION_H_
