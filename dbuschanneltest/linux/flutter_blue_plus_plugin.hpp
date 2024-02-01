#pragma once
#include <gtk/gtk.h>
#include <math.h>
#include <gmodule.h>
#include <cstring>
#include <boost/array.hpp>
#include <boost/filesystem.hpp>
#include <sys/utsname.h>
#include <simplebluez/Bluez.h>
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>
#include <map>

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
        if(count % 10000 == 0){
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

  std::string getAdapterName(){
    auto adapters = bluez.get_adapters();
    std::cout << "Bluez: getAdapterName()" << std::endl;

    if(adapters.size() <= 0) return "None";

    auto ret = adapters[0]->identifier();
    return ret;
  }

  std::vector<std::shared_ptr<SimpleBluez::Device>> getSystemDevices(){
    std::cout << "Bluez: getSystemDevices()" << std::endl;

    auto adapter = bluez.get_adapters()[0];

    SimpleBluez::Adapter::DiscoveryFilter filter;
    filter.Transport = SimpleBluez::Adapter::DiscoveryFilter::TransportType::LE;
    adapter->discovery_filter(filter);

    std::vector<std::shared_ptr<SimpleBluez::Device>> ret{};

    ret = adapter->device_paired_get();

    return ret;
  }

  void startScan(std::function<void(std::shared_ptr<SimpleBluez::Device> device)> call_back){
    std::cout << "Bluez: startScan()" << std::endl;
    auto adapter = bluez.get_adapters()[0];
    adapter->clear_on_device_updated();
    devices.clear();
    
    adapter->set_on_device_updated(call_back);

    adapter->discovery_start();
  }

  void stopScan(){
    std::cout << "Bluez: stopScan()" << std::endl;
    auto adapter = bluez.get_adapters()[0];
    
    adapter->discovery_stop();
    std::this_thread::sleep_for(std::chrono::seconds(1));

    adapter->clear_on_device_updated();
  }

  TurnedState getTurnedState(){
    std::lock_guard<std::mutex> lk_turn{turn_mutex};

    std::cout << "Bluez: getTurnedState()" << std::endl;

    return async_thread_active ? ON : OFF;
  }

  void connectTo(std::string rm_id, std::function<void(std::string)> call_back){
    devices[rm_id]->connect();

    call_back(rm_id);
    
  }

  std::vector<std::shared_ptr<SimpleBluez::Service>> discoverServices(std::string rm_id){
    devices[rm_id]->set_on_services_resolved([](){
      std::cout << "new service discovered" << std::endl;
    });

    return devices[rm_id]->services();
  }
  
  public:
  std::shared_ptr<std::thread> async_bluez_thread;

  // devices
  std::map<std::string, std::shared_ptr<SimpleBluez::Device>> devices{};

  // mutex
  std::mutex turn_mutex;

};