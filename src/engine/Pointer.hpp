/*
** AgarthaSoftware, 2024
** Cevy
** File description:
** Pointer
*/

#pragma once

#include <exception>
#include <memory>
#include <string>

namespace cevy {
namespace engine {

template <typename T>
class pointer {
  public:
  class exception : public std::exception {
    public:
    exception(const std::string &msg) : _msg(msg){};
    const char *what() const noexcept override { return _msg.c_str(); };

    private:
    std::string _msg;
  };
  using value_type = T;
  using reference_type = T &;

  pointer() {
    _weak.reset();
    _value = nullptr;
  }

  pointer(const std::shared_ptr<int> &lock, T &value) {
    _weak = lock;
    _value = &value;
  };

  const T &operator*() const {
    if (_weak.expired())
      throw exception("expired access of unowned ptr");
    if (!_value)
      throw exception("access of nulled unowned ptr");
    return *_value;
  };

  const T *operator->() const {
    if (_weak.expired())
      throw exception("expired access unowned ptr");
    if (!_value)
      throw exception("access of nulled unowned ptr");
    return _value;
  };

  T &operator*() {
    if (_weak.expired())
      throw exception("expired access of unowned ptr");
    if (!_value)
      throw exception("access of nulled unowned ptr");
    return *_value;
  };

  T *operator->() {
    if (_weak.expired())
      throw exception("expired access unowned ptr");
    if (!_value)
      throw exception("access of nulled unowned ptr");
    return _value;
  };

  operator bool() const { return _value != nullptr && !_weak.expired(); };

  private:
  std::weak_ptr<void> _weak;
  T *_value;
};
} // namespace engine
} // namespace cevy
