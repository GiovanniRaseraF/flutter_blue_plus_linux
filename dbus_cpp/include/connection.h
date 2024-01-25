#pragma once

#include <dbus/dbus.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <memory>
#include <mutex>

#include <exceptions.h>

class Connection { 
  public:
  Connection(DBusBusType bt) : _bus_type{bt}{}

  DBusBusType _bus_type;
  DBusConnection* _conn = nullptr;
  std::recursive_mutex _mutex;
  bool is_init = false;

  void init(){
    if(is_init){
      return;
    }

    std::lock_guard<std::recursive_mutex> lock(_mutex);

    //dbus_threads_init_default();
    DBusError errors;
    _conn = dbus_bus_get(_bus_type, &errors);

    if (dbus_error_is_set(&errors)) {
        std::string err_name = errors.name;
        std::string err_message = errors.message;
        dbus_error_free(&errors);
        throw Exception::DBusException(err_name, err_message);
    }

    is_init = true;
  }
  
//   void uninit() {
//     if (!is_init) {
//         return;
//     }
    
//     std::lock_guard<std::recursive_mutex> lock(_mutex);
    
//     DBusMessage *message;
//     bool done = false;
//     do {
//         std::this_thread::sleep_for(std::chrono::milliseconds(10));
//         read_write();
//         message = pop_message();
//         if(message == nullptr){
//           done = true;
//         }else{
//           dbus_message_unref(message);
//         }
//     } while (!done);

//     dbus_connection_unref(_conn);
//     is_init = false;
// }

// bool is_initialized() { return is_init; }

//   // get unique_name
//   std::string unique_name() {
//       if (!is_init) {
//           throw Exception::NotInitialized();
//       }
//       std::lock_guard<std::recursive_mutex> lock(_mutex);
//       return std::string(dbus_bus_get_unique_name(_conn));
//   }

//   void read_write() {
//     if (!is_init) {
//         throw Exception::NotInitialized();
//     }
    
//     // Non blocking read of the next available message
//     dbus_connection_read_write(_conn, 0);
//   }

//   DBusMessage *pop_message() {
//     if (!is_init) {
//         throw Exception::NotInitialized();
//     }
//     std::lock_guard<std::recursive_mutex> lock(_mutex);
//     DBusMessage* msg = dbus_connection_pop_message(_conn);
//     return (msg);
//   }

//   void send(DBusMessage *msg) {
//     if (!is_init) {
//         throw Exception::NotInitialized();
//     }
//     std::lock_guard<std::recursive_mutex> lock(_mutex);
//     uint32_t msg_serial = 0;
//     dbus_connection_send(_conn, msg, &msg_serial);
//     dbus_connection_flush(_conn);
//   }

//   DBusMessage *send_with_reply_and_block(DBusMessage *msg) {
//     if (!is_init) {
//         throw Exception::NotInitialized();
//     }
//     std::lock_guard<std::recursive_mutex> lock(_mutex);
//     ::DBusError err;
//     dbus_error_init(&err);
//     DBusMessage* msg_tmp = dbus_connection_send_with_reply_and_block(_conn, msg, -1, &err);

//     if (dbus_error_is_set(&err)) {
//         std::string err_name = err.name;
//         std::string err_message = err.message;
//         dbus_error_free(&err);
//         throw Exception::SendFailed(err_name, err_message, "message_problem");
//     }

//     return msg_tmp;
//   }

  ~Connection(){
    //uninit();
  }
};