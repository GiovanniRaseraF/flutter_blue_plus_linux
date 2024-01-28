#include "my_application.h"

#include <flutter_linux/flutter_linux.h>

#ifdef GDK_WINDOWING_X11
#include <gdk/gdkx.h>
#endif

#include "flutter/generated_plugin_registrant.h"
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>

struct _MyApplication {
  GtkApplication parent_instance;
  char** dart_entrypoint_arguments;
  FlMethodChannel* battery_channel;
};

G_DEFINE_TYPE(MyApplication, my_application, GTK_TYPE_APPLICATION)

// bluez
SimpleBluez::Bluez bluez;

// async thread
std::atomic_bool async_thread_active = true;
void async_thread_function() {
    int count = 0;
    while (async_thread_active) {
        bluez.run_async();
        std::this_thread::sleep_for(std::chrono::microseconds(100));

        // every 10 seconds
        if(count % 1000 == 0){
          std::cout << "- Bluez Running -" << std::endl;
        }

        count++;
    }
}

class FlutterBluePlusPlugin {
  public:
  FlutterBluePlusPlugin(){
    bluez.init();
    // need to run bluez into separate thread all the time
    async_bluez_thread = std::make_shared<std::thread>(async_thread_function);

    std::cout << "Bluez: FlutterBluePlusPlugin()" << std::endl;
  }

  ~FlutterBluePlusPlugin(){
    turnOff();

    std::cout << "Bluez: ~()" << std::endl;
  }

  int adaptersCount(){
    auto adapters = bluez.get_adapters();

    return adapters.size();
  }

  bool isSupported(){
    return adaptersCount() > 0;
  }

  void turnOn(){
    turnOff(); 

    std::lock_guard<std::mutex> lk_turn{turn_mutex};
    async_thread_active = true;

    async_bluez_thread = std::make_shared<std::thread>(async_thread_function);

    std::cout << "Bluez: turnOn()" << std::endl;
  }

  void turnOff(){
    std::lock_guard<std::mutex> lk_turn{turn_mutex};

    async_thread_active = false;
    std::this_thread::sleep_for(std::chrono::microseconds(300));
    async_thread_active = false;

    std::cout << "Bluez: turnOFF()" << std::endl;
  }

  TurnedState getTurnedState(){
    std::lock_guard<std::mutex> lk_turn{turn_mutex};

    std::cout << "Bluez: getTurnedState()" << std::endl;

    return async_thread_active ? ON : OFF;
  }
  
  private:
  std::shared_ptr<std::thread> async_bluez_thread;

  // mutex
  std::mutex turn_mutex;

};

FlutterBluePlusPlugin my_plugin{};

static FlMethodResponse* get_battery_level() {
  return FL_NOIMP;
}

static FlMethodResponse* get_platform_verision(){
  return FL_STR("Linux Ubuntu");
}

static FlMethodResponse* connected_count(){
  return FL_NOIMP;
}

static FlMethodResponse* set_log_level(){
  return FL_BOOL(true);
}

static FlMethodResponse* flutter_hot_restart(){
  //my_plugin.turnOff();

  return FL_INT(0);
}

static FlMethodResponse* is_supported(){
  bool ret = my_plugin.isSupported();

  return FL_BOOL(ret);
}

static FlMethodResponse* get_adapter_state(){
  // create map
  g_autoptr(FlValue) value = fl_value_new_map ();

  // set value
  fl_value_set_string_take(value, 
    "adapter_state", fl_value_new_int(bmAdapterStateEnum(my_plugin.getTurnedState())));

  return FL_METHOD_RESPONSE(fl_method_success_response_new(value));
}

static FlMethodResponse* turn_on() {
  bool ret_value = false;
  TurnedState current_state = my_plugin.getTurnedState();

  if(current_state == ON){
    // no change
    ret_value = false;
  }else{
    my_plugin.turnOn();

    // change
    ret_value = true;
  }

  return FL_BOOL(ret_value);
}

static FlMethodResponse* turn_off() {
  bool ret_value = false;
  TurnedState current_state = my_plugin.getTurnedState();

  if(current_state == OFF){
    // no change
    ret_value = false;
  }else{
    my_plugin.turnOff();

    // change
    ret_value = true;
  }

  return FL_BOOL(ret_value);
}

static void battery_method_call_handler(FlMethodChannel* channel,
                                        FlMethodCall* method_call,
                                        gpointer user_data) {
  g_autoptr(FlMethodResponse) response = nullptr;
  std::string mcall = fl_method_call_get_name(method_call);

  if("flutterHotRestart" == mcall){
    response = flutter_hot_restart();
  } 
  else if("getPlatformVersion" == mcall){
    response = get_platform_verision();
  }
  else if("connectionCount" == mcall){
    response = connected_count();
  }
  else if("setLogLevel" == mcall){
    response = set_log_level();
  }
  else if("isSupported" == mcall){
    response = is_supported();
  }
  else if("getAdapterName" == mcall){}
  else if("getAdapterState" == mcall){
    response = get_adapter_state();
  }
  else if("turnOn" == mcall){
    response = turn_on();
  }
  else if("turnOff" == mcall){
    response = turn_off();
  }
  else if("startScan" == mcall){}
  else if("stopScan" == mcall){}
  else if("getSystemDevices" == mcall){}
  else if("connect" == mcall){}
  else if("disconnect" == mcall){}
  else if("discoverServices" == mcall){}
  else if("readCharacteristic" == mcall){}
  else if("writeCharacteristic" == mcall){}
  else if("readDescriptor" == mcall){}
  else if("writeDescriptor" == mcall){}
  else if("setNotifyValue" == mcall){}
  else if("requestMtu" == mcall){}
  else if("readRssi" == mcall){}
  else if("requestConnectionPriority" == mcall){}
  else if("getPhySupport" == mcall){}
  else if("setPreferredPhy" == mcall){}
  else if("getBondedDevices" == mcall){}
  else if("getBondState" == mcall){}
  else if("createBond" == mcall){}
  else if("removeBond" == mcall){}
  else if("clearGattCache" == mcall){}
  else {
    response = FL_METHOD_RESPONSE(fl_method_not_implemented_response_new());
  }

  g_autoptr(GError) error = nullptr;
  if (!fl_method_call_respond(method_call, response, &error)) {
    g_warning("Failed to send response: %s", error->message);
  }
}

// Implements GApplication::activate.
static void my_application_activate(GApplication* application) {
  MyApplication* self = MY_APPLICATION(application);
  GtkWindow* window =
      GTK_WINDOW(gtk_application_window_new(GTK_APPLICATION(application)));

  gboolean use_header_bar = TRUE;
#ifdef GDK_WINDOWING_X11
  GdkScreen* screen = gtk_window_get_screen(window);
  if (GDK_IS_X11_SCREEN(screen)) {
    const gchar* wm_name = gdk_x11_screen_get_window_manager_name(screen);
    if (g_strcmp0(wm_name, "GNOME Shell") != 0) {
      use_header_bar = FALSE;
    }
  }
#endif
  if (use_header_bar) {
    GtkHeaderBar* header_bar = GTK_HEADER_BAR(gtk_header_bar_new());
    gtk_widget_show(GTK_WIDGET(header_bar));
    gtk_header_bar_set_title(header_bar, "dbuschanneltest");
    gtk_header_bar_set_show_close_button(header_bar, TRUE);
    gtk_window_set_titlebar(window, GTK_WIDGET(header_bar));
  } else {
    gtk_window_set_title(window, "dbuschanneltest");
  }

  gtk_window_set_default_size(window, 1280, 720);
  gtk_widget_show(GTK_WIDGET(window));

  g_autoptr(FlDartProject) project = fl_dart_project_new();
  fl_dart_project_set_dart_entrypoint_arguments(project, self->dart_entrypoint_arguments);

  FlView* view = fl_view_new(project);
  gtk_widget_show(GTK_WIDGET(view));
  gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(view));

  fl_register_plugins(FL_PLUGIN_REGISTRY(view));
g_autoptr(FlStandardMethodCodec) codec = fl_standard_method_codec_new();
  self->battery_channel = fl_method_channel_new(
      fl_engine_get_binary_messenger(fl_view_get_engine(view)),
      "samples.flutter.dev/dbuschannel", FL_METHOD_CODEC(codec));
  fl_method_channel_set_method_call_handler(
      self->battery_channel, battery_method_call_handler, self, nullptr);
  gtk_widget_grab_focus(GTK_WIDGET(view));
}

// Implements GApplication::local_command_line.
static gboolean my_application_local_command_line(GApplication* application, gchar*** arguments, int* exit_status) {
  MyApplication* self = MY_APPLICATION(application);
  // Strip out the first argument as it is the binary name.
  self->dart_entrypoint_arguments = g_strdupv(*arguments + 1);

  g_autoptr(GError) error = nullptr;
  if (!g_application_register(application, nullptr, &error)) {
     g_warning("Failed to register: %s", error->message);
     *exit_status = 1;
     return TRUE;
  }

  g_application_activate(application);
  *exit_status = 0;

  return TRUE;
}

// Implements GObject::dispose.
static void my_application_dispose(GObject* object) {
  MyApplication* self = MY_APPLICATION(object);
  g_clear_pointer(&self->dart_entrypoint_arguments, g_strfreev);
  g_clear_object(&self->battery_channel);
  G_OBJECT_CLASS(my_application_parent_class)->dispose(object);
}

static void my_application_class_init(MyApplicationClass* klass) {
  G_APPLICATION_CLASS(klass)->activate = my_application_activate;
  G_APPLICATION_CLASS(klass)->local_command_line = my_application_local_command_line;
  G_OBJECT_CLASS(klass)->dispose = my_application_dispose;
}

static void my_application_init(MyApplication* self) {
  
}

MyApplication* my_application_new() {
  return MY_APPLICATION(g_object_new(my_application_get_type(),
                                     "application-id", APPLICATION_ID,
                                     "flags", G_APPLICATION_NON_UNIQUE,
                                     nullptr));
}
