#ifndef FLUTTER_MY_APPLICATION_H_
#define FLUTTER_MY_APPLICATION_H_

#include <gtk/gtk.h>
#include <math.h>
#include <boost/array.hpp>
#include <boost/filesystem.hpp>
#include <simplebluez/Bluez.h>
#include <simplebluez/Adapter.h>
#include <iostream>

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
#define FL_STR(_str) FL_METHOD_RESPONSE(fl_method_success_response_new(fl_value_new_string((_str))))
#define FL_BOOL(_bl) FL_METHOD_RESPONSE(fl_method_success_response_new(fl_value_new_bool((_bl))))
#define FL_INT(_in) FL_METHOD_RESPONSE(fl_method_success_response_new(fl_value_new_int((_in))))
#define FL_FLOT(_in) FL_METHOD_RESPONSE(fl_method_success_response_new(fl_value_new_float((_in))))
#define FL_NOIMP FL_METHOD_RESPONSE(fl_method_not_implemented_response_new())

enum TurnedState{
  OFF = 0, ON
};


//////////////////////////////////////////////////////////////////////
// ███    ███  ███████   ██████      
// ████  ████  ██       ██           
// ██ ████ ██  ███████  ██   ███     
// ██  ██  ██       ██  ██    ██     
// ██      ██  ███████   ██████ 
//     
// ██   ██  ███████  ██       ██████   ███████  ██████   ███████ 
// ██   ██  ██       ██       ██   ██  ██       ██   ██  ██      
// ███████  █████    ██       ██████   █████    ██████   ███████ 
// ██   ██  ██       ██       ██       ██       ██   ██       ██ 
// ██   ██  ███████  ███████  ██       ███████  ██   ██  ███████ 

//See: BmAdapterStateEnum
static int bmAdapterStateEnum(TurnedState state) {
    switch (state) {
        case OFF:                                  return 6;
        case ON:                                 return 4;
    }
}

#endif // FLUTTER_MY_APPLICATION_H_
