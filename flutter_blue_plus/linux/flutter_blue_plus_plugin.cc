#include "include/flutter_blue_plus/flutter_blue_plus_plugin.h"
#include "include/flutter_blue_plus/test.h"

#include <flutter_linux/flutter_linux.h>
#include <sys/utsname.h>

#include <cstring>
#include <iostream>
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <thread>

#include "flutter_blue_plus_plugin_private.h"

#include <boost/array.hpp>
#include <boost/filesystem.hpp>

#include <simplebluez/Bluez.h>


#define FLUTTER_BLUE_PLUS_PLUGIN(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), flutter_blue_plus_plugin_get_type(), \
                              FlutterBluePlusPlugin))



struct _FlutterBluePlusPlugin {
  
  GObject parent_instance;
  bool is_scanning = false;

  bool isAdapterOn(){
    return true;
  }
};

G_DEFINE_TYPE(FlutterBluePlusPlugin, flutter_blue_plus_plugin, g_object_get_type())

// Called when a method call is received from Flutter.
static void flutter_blue_plus_plugin_handle_method_call(
    FlutterBluePlusPlugin* self,
    FlMethodCall* method_call) {
  g_autoptr(FlMethodResponse) response = nullptr;

  const gchar* method = fl_method_call_get_name(method_call);
  SimpleBluez::Bluez bluez;
  if (strcmp(method, "getPlatformVersion") == 0) {
    response = get_platform_version();

  }else if (strcmp(method, "isSupported") == 0){
    hello();
    response = is_supported(self);

  }else if (strcmp(method, "flutterHotRestart") == 0) {
    response = flutter_hot_restart(self);
  } else {
    response = FL_METHOD_RESPONSE(fl_method_not_implemented_response_new());
  }

  fl_method_call_respond(method_call, response, nullptr);
}

FlMethodResponse* is_supported(FlutterBluePlusPlugin* self) {
    bool r = false;//self->conn.is_initialized();

    return FL_METHOD_RESPONSE(fl_method_success_response_new(fl_value_new_bool(r)));
}

FlMethodResponse* flutter_hot_restart(FlutterBluePlusPlugin* self) {
    // no adapter ? 
    std::cout << "-flutterHotRestart-" << std::endl;

    if(self->isAdapterOn()){
      std::cout << "-Adapter is on-" << std::endl;
    }

    std::cout << "connectedPeripherals: " <<  std::endl;

    return FL_METHOD_RESPONSE(fl_method_success_response_new(fl_value_new_int(0)));
}

FlMethodResponse* get_platform_version() {
  hello();
  struct utsname uname_data = {};
  uname(&uname_data);
  g_autofree gchar *version = g_strdup_printf("Linux %s", uname_data.version);
  g_autoptr(FlValue) result = fl_value_new_string(version);
  return FL_METHOD_RESPONSE(fl_method_success_response_new(result));
}

static void flutter_blue_plus_plugin_dispose(GObject* object) {
  G_OBJECT_CLASS(flutter_blue_plus_plugin_parent_class)->dispose(object);
}

static void flutter_blue_plus_plugin_class_init(FlutterBluePlusPluginClass* klass) {
  G_OBJECT_CLASS(klass)->dispose = flutter_blue_plus_plugin_dispose;
}

static void flutter_blue_plus_plugin_init(FlutterBluePlusPlugin* self) {}

static void method_call_cb(FlMethodChannel* channel, FlMethodCall* method_call,
                           gpointer user_data) {
  FlutterBluePlusPlugin* plugin = FLUTTER_BLUE_PLUS_PLUGIN(user_data);
  flutter_blue_plus_plugin_handle_method_call(plugin, method_call);
}

void flutter_blue_plus_plugin_register_with_registrar(FlPluginRegistrar* registrar) {
  FlutterBluePlusPlugin* plugin = FLUTTER_BLUE_PLUS_PLUGIN(
      g_object_new(flutter_blue_plus_plugin_get_type(), nullptr));

  g_autoptr(FlStandardMethodCodec) codec = fl_standard_method_codec_new();
  g_autoptr(FlMethodChannel) channel =
      fl_method_channel_new(fl_plugin_registrar_get_messenger(registrar),
                            "flutter_blue_plus",
                            FL_METHOD_CODEC(codec));
  fl_method_channel_set_method_call_handler(channel, method_call_cb,
                                            g_object_ref(plugin),
                                            g_object_unref);

  g_object_unref(plugin);
}
