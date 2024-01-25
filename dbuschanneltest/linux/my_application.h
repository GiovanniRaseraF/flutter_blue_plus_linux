#ifndef FLUTTER_MY_APPLICATION_H_
#define FLUTTER_MY_APPLICATION_H_

#include <gtk/gtk.h>
#include <math.h>
#include <boost/array.hpp>
#include <boost/filesystem.hpp>
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

struct MyDBus
{
  int count = 0;
  MyDBus()
  {
    boost::array<std::string, 10> myarray;
    myarray[5] = "hello";
    std::cout << myarray[5] << std::endl;

    boost::filesystem::path p{};
    std::cout << p << std::endl;

  }
};

#endif // FLUTTER_MY_APPLICATION_H_
