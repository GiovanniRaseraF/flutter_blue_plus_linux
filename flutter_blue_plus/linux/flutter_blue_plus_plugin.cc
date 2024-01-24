#include "include/flutter_blue_plus/flutter_blue_plus_plugin.h"

#include <flutter_linux/flutter_linux.h>
#include <gtk/gtk.h>
#include <sys/utsname.h>

#include <cstring>
#include <iostream>

#include "flutter_blue_plus_plugin_private.h"

#define FLUTTER_BLUE_PLUS_PLUGIN(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), flutter_blue_plus_plugin_get_type(), \
                              FlutterBluePlusPlugin))

struct _FlutterBluePlusPlugin {
  GObject parent_instance;
  int *centralManager = nullptr;

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

  if (strcmp(method, "getPlatformVersion") == 0) {
    response = get_platform_version();

  }else if (strcmp(method, "flutterHotRestart") == 0) {
    // if ([@"flutterHotRestart" isEqualToString:call.method])
    //     {
    //         // no adapter?
    //         if (self.centralManager == nil) {
    //             result(@(0)); // no work to do
    //             return;
    //         }

    //         if ([self isAdapterOn]) {
    //             [self.centralManager stopScan];
    //         }

    //         [self disconnectAllDevices:@"flutterHotRestart"];

    //         Log(LDEBUG, @"connectedPeripherals: %lu", self.connectedPeripherals.count);

    //         if (self.connectedPeripherals.count == 0) {
    //             [self.knownPeripherals removeAllObjects];
    //         }
            
    //         result(@(self.connectedPeripherals.count));
    //         return;
    //     }

    // no adapter ? 
    std::cout << "-flutterHotRestart-" << std::endl;

    if(self->centralManager == nullptr){
      response = FL_METHOD_RESPONSE(fl_method_success_response_new(fl_value_new_int(0)));
    }

    if(self->isAdapterOn()){
      std::cout << "-Adapter is on-" << std::endl;
    }

    std::cout << "connectedPeripherals: " <<  std::endl;

  } else {
    response = FL_METHOD_RESPONSE(fl_method_not_implemented_response_new());
  }

  fl_method_call_respond(method_call, response, nullptr);
}

FlMethodResponse* get_platform_version() {
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
