#include "my_application.h"

#include <flutter_linux/flutter_linux.h>

#ifdef GDK_WINDOWING_X11
#include <gdk/gdkx.h>
#endif

#include "flutter/generated_plugin_registrant.h"

#include "flutter_blue_plus_plugin.hpp"

struct _MyApplication {
  GtkApplication parent_instance;
  char** dart_entrypoint_arguments;
};
static FlMethodChannel* flutter_blue_plus_plugin_channel;

G_DEFINE_TYPE(MyApplication, my_application, GTK_TYPE_APPLICATION)

FlutterBluePlusPlugin my_plugin{};

static FlMethodResponse* get_battery_level() {
  return FL_NOIMP_RESPONSE;
}

static FlMethodResponse* get_platform_verision(){
  struct utsname name;
  uname(&name);
  std::string ret = std::string(name.sysname) + std::string(name.version);

  return FL_STR_RESPONSE(ret.c_str());
}

static FlMethodResponse* connected_count(){
  return FL_NOIMP_RESPONSE;
}

static FlMethodResponse* set_log_level(){
  return FL_BOOL_RESPONSE(true);
}

static FlMethodResponse* flutter_hot_restart(){
  //my_plugin.turnOff();

  return FL_INT_RESPONSE(0);
}

static FlMethodResponse* is_supported(){
  bool ret = my_plugin.isSupported();

  return FL_BOOL_RESPONSE(ret);
}

static FlMethodResponse* get_adapter_name(){
  auto name = my_plugin.getAdapterName();

  return FL_STR_RESPONSE(name.c_str() );
}

static FlMethodResponse* get_adapter_state(){
  // create map
  auto map = FL_MAP;
  auto value = (bmAdapterStateEnum(my_plugin.getTurnedState()));

  // set value
  FL_MAP_SET(map, "adapter_state", FL_INT(value));

  return FL_RESP(map);
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

  return FL_BOOL_RESPONSE(ret_value);
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

  return FL_BOOL_RESPONSE(ret_value);
}

static FlMethodResponse* start_scan() {
  my_plugin.startScan([&](std::shared_ptr<SimpleBluez::Device> device) {
      // create the response
      auto map = FL_MAP;
      auto map_l = FL_LIST;
      FL_MAP_SET(map, "advertisements", map_l);

      auto adver = FL_MAP;
      FL_MAP_SET(adver, "remote_id",      FL_STR(device->address().c_str()));
      FL_MAP_SET(adver, "platform_name",  FL_STR(device->name().c_str()));
      FL_MAP_SET(adver, "adv_name",       FL_STR(device->alias().c_str()));
      FL_MAP_SET(adver, "connectable",    FL_BOOL(true));
      FL_MAP_SET(adver, "tx_power_level", FL_INT((int)device->tx_power()));
      FL_MAP_SET(adver, "rssi",           FL_INT(1));
      FL_APPEND(map_l, adver);

      // devices
      my_plugin.devices[device->address()] = device;

      // responde to ui 
      TO_UI("OnScanResponse", map);
    });

  return FL_BOOL_RESPONSE(true);
}

static FlMethodResponse* stop_scan() {
  my_plugin.stopScan();

  return FL_BOOL_RESPONSE(true);
}

static FlMethodResponse* get_system_devices() {
  auto devices = my_plugin.getSystemDevices();

  auto ret = FL_MAP;
  auto ret_l = FL_LIST;

  for_each(devices.begin(), devices.end(), [&](auto d){
    auto dev_map = FL_MAP;

    FL_MAP_SET(dev_map, "remote_id", FL_STR(d->address().c_str()));
    FL_MAP_SET(dev_map, "platform_name", FL_STR(d->name().c_str()));
    FL_APPEND(ret_l, dev_map);
  });
  FL_MAP_SET(ret, "devices", ret_l);

  return FL_RESP(ret);
}

static FlMethodResponse* connect(std::string rm_id) {
  my_plugin.connectTo(rm_id, [&](std::string id){
    auto map = FL_MAP;
    
    FL_MAP_SET(map, "remote_id",        FL_STR(id.c_str()));
    FL_MAP_SET(map, "connection_state", FL_INT(1));

    // responde to ui 
    TO_UI("OnConnectionStateChanged", map);
  });

  return FL_BOOL_RESPONSE(true);
}

static FlMethodResponse* disconnect() {
  return FL_NOIMP_RESPONSE;
}

static FlMethodResponse* discover_services(std::string rm_id) {
  auto services = my_plugin.discoverServices(rm_id);

  auto map = FL_MAP;
  FL_MAP_SET(map, "remote_id",    FL_STR(rm_id.c_str()));
  FL_MAP_SET(map, "success",      FL_BOOL(true));
  FL_MAP_SET(map, "error_code",   FL_INT(0));
  FL_MAP_SET(map, "error_string", FL_STR(""));

  auto services_list = FL_LIST;
  for(auto s : services){
    auto serv = FL_MAP;  

    // service
    FL_MAP_SET(serv, "service_uuid",  FL_STR(s->uuid().c_str()));
    FL_MAP_SET(serv, "remote_id",     FL_STR(rm_id.c_str()));
    FL_MAP_SET(serv, "is_primary",    FL_BOOL(false));

    // charac
    auto characs = FL_LIST;
    for(auto c : s->characteristics()){
      auto c_map = FL_MAP;
      auto d_list = FL_LIST;
      auto p_map = FL_MAP;
      
      FL_MAP_SET(c_map, "descriptors",          d_list);
      FL_MAP_SET(c_map, "remote_id",            FL_STR(rm_id.c_str()));
      FL_MAP_SET(c_map, "service_uuid",         FL_STR(s->uuid().c_str()));
      FL_MAP_SET(c_map, "characteristic_uuid",  FL_STR(c->uuid().c_str()));

      // properties
      FL_MAP_SET(p_map, "broadcast",                    FL_BOOL(true));
      FL_MAP_SET(p_map, "read",                         FL_BOOL(true));
      FL_MAP_SET(p_map, "write_without_response",       FL_BOOL(true));
      FL_MAP_SET(p_map, "write",                        FL_BOOL(true));
      FL_MAP_SET(p_map, "notify",                       FL_BOOL(true));
      FL_MAP_SET(p_map, "indicate",                     FL_BOOL(true));
      FL_MAP_SET(p_map, "authenticated_signed_writes",  FL_BOOL(true));
      FL_MAP_SET(p_map, "extended_properties",          FL_BOOL(true));
      FL_MAP_SET(p_map, "notify_encryption_required",   FL_BOOL(true));
      FL_MAP_SET(p_map, "indicate_encryption_required", FL_BOOL(true));
      FL_MAP_SET(c_map, "properties", p_map);

      FL_APPEND(characs, c_map);
    }
    FL_MAP_SET(serv, "characteristics", characs);

    FL_APPEND(services_list, serv);
  }

  FL_MAP_SET(map, "services", services_list);
  
  // responde to ui 
  TO_UI("OnDiscoveredServices", map);

  return FL_RESP(map);
}

static void battery_method_call_handler(FlMethodChannel* channel,
                                        FlMethodCall* method_call,
                                        gpointer user_data) {
  g_autoptr(FlMethodResponse) response = nullptr;
  std::string mcall = fl_method_call_get_name(method_call);

  auto args = FL_ARGS(method_call);

  // startScanning
  #define with_services fl_value_get_map_value(args, 0)
  #define with_remote_ids fl_value_get_map_value(args, 1)
  #define with_names fl_value_get_map_value(args, 2)
  #define with_keywords fl_value_get_map_value(args, 3)
  #define with_service_data fl_value_get_map_value(args, 4)
  #define continuous_updates fl_value_get_map_value(args, 5)
  #define continuous_divisor fl_value_get_map_value(args, 6)

  // connect
  #define remote_id fl_value_get_map_value(args, 0)
  #define auto_connect fl_value_get_map_value(args, 1)

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
  else if("getAdapterName" == mcall){
    response = get_adapter_name();
  }
  else if("getAdapterState" == mcall){
    response = get_adapter_state();
  }
  else if("turnOn" == mcall){
    response = turn_on();
  }
  else if("turnOff" == mcall){
    response = turn_off();
  }
  else if("startScan" == mcall){
    response = start_scan();
  }
  else if("stopScan" == mcall){
    response = stop_scan();
  }
  else if("getSystemDevices" == mcall){
    response = get_system_devices();
  }
  else if("connect" == mcall){
    std::string remote_id_str = fl_value_to_string(remote_id);
    
    response = connect(remote_id_str);
  }
  else if("disconnect" == mcall){
    response = disconnect();
  }
  else if("discoverServices" == mcall){
    std::string remote_id_str = fl_value_to_string(args);

    response = discover_services(remote_id_str);
  }
  else if("readCharacteristic" == mcall){
    response = FL_NOIMP_RESPONSE;
  }
  else if("writeCharacteristic" == mcall){
    response = FL_NOIMP_RESPONSE;
  }
  else if("readDescriptor" == mcall){
    response = FL_NOIMP_RESPONSE;
  }
  else if("writeDescriptor" == mcall){
    response = FL_NOIMP_RESPONSE;
  }
  else if("setNotifyValue" == mcall){
    response = FL_NOIMP_RESPONSE;
  }
  else if("requestMtu" == mcall){
    response = FL_NOIMP_RESPONSE;
  }
  else if("readRssi" == mcall){
    response = FL_NOIMP_RESPONSE;
  }
  else if("requestConnectionPriority" == mcall){
    response = FL_NOIMP_RESPONSE;
  }
  else if("getPhySupport" == mcall){
    response = FL_NOIMP_RESPONSE;
  }
  else if("setPreferredPhy" == mcall){
    response = FL_NOIMP_RESPONSE;
  }
  else if("getBondedDevices" == mcall){
    response = FL_NOIMP_RESPONSE;
  }
  else if("getBondState" == mcall){
    response = FL_NOIMP_RESPONSE;
  }
  else if("createBond" == mcall){
    response = FL_NOIMP_RESPONSE;
  }
  else if("removeBond" == mcall){
    response = FL_NOIMP_RESPONSE;
  }
  else if("clearGattCache" == mcall){
    response = FL_NOIMP_RESPONSE;
  }
  else {
    response = FL_NOIMP_RESPONSE;
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

  // specific to battery
  fl_register_plugins(FL_PLUGIN_REGISTRY(view));
  g_autoptr(FlStandardMethodCodec) codec = fl_standard_method_codec_new();

  flutter_blue_plus_plugin_channel = fl_method_channel_new(
      fl_engine_get_binary_messenger(fl_view_get_engine(view)),
      "samples.flutter.dev/dbuschannel", FL_METHOD_CODEC(codec));

  fl_method_channel_set_method_call_handler(
      flutter_blue_plus_plugin_channel, battery_method_call_handler, self, nullptr);
  // specific to battery

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
