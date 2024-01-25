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

void hello()
{
  std::cout << "hello from test" << std::endl;
  system("busctl tree org.bluez");
}