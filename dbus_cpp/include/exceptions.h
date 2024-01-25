#pragma once

#include <stdexcept>
#include <string>

namespace Exception {

class BaseException : public std::exception {};

class NotInitialized : public BaseException {
  public:
    NotInitialized(){}
    const char* what() const noexcept override{
      return "Not Initialized";
    }
};

class DBusException : public BaseException {
  public:
    DBusException(const std::string& err_name, const std::string& err_message){
      _message = "err_name: " + err_name + "| err_message: " + err_message;
    }
    const char* what() const noexcept override{
      return _message.c_str();
    }

  private:
    std::string _message;
};

class SendFailed : public BaseException {
  public:
    SendFailed(const std::string& err_name, const std::string& err_message, const std::string& msg_str){
      _message = "err_name: " + err_name + "| err_message: " + err_message + "| msg_str: " + msg_str;
    }
    const char* what() const noexcept override{
      return _message.c_str();
    }

  private:
    std::string _message;
};

class InterfaceNotFoundException : public BaseException {
  public:
    InterfaceNotFoundException(const std::string& path, const std::string& interface){
      _message = "path: " + path + "| interface: " + interface;
    }
    const char* what() const noexcept override{
      return _message.c_str();
    }

  private:
    std::string _message;
};

class PathNotFoundException : public BaseException {
  public:
    PathNotFoundException(const std::string& path, const std::string& subpath){
      _message = "path: " + path + "| subpath: " + subpath;
    }
    const char* what() const noexcept override{
      return _message.c_str();
    }

  private:
    std::string _message;
};

}  // namespace Exception
