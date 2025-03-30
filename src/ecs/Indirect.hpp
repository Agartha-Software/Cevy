/*
** EPITECH PROJECT, 2024
** rtype
** File description:
** Indirect
*/

#pragma once

#include <exception>
#include <functional>
#include <optional>
#include <string>
#include <type_traits>
#include <typeinfo>

template <typename T>
class Indirect;

template <typename Type>
struct is_indirect : std::false_type {};

template <typename Type>
struct is_indirect<Indirect<Type>> : std::true_type {};

template <typename T>
struct remove_indirect_t {
  using value_type = T;
};

template <typename T>
struct remove_indirect_t<Indirect<T>> {
  using value_type = T;
};

template <typename T>
struct remove_indirect_t<Indirect<T &>> {
  using value_type = T &;
};

template <typename T>
using remove_indirect = typename remove_indirect_t<T>::value_type;

template <typename T>
constexpr Indirect<T> make_indirect(const std::function<T()> &f) {
  return Indirect<T>(f);
}

template <typename T>
class Indirect {
  public:
  using value_type = T;
  class exception : public std::exception {
    private:
    std::string msg;

    public:
    exception(const std::string &str) : msg(str) {};
    const char *what() const noexcept override { return msg.c_str(); }
  };

  template <std::enable_if_t<std::is_move_constructible_v<T>, bool>>
  Indirect(Indirect<T> &&i) : _obj(std::move(i._obj)), _gen(std::move(i._gen)) {};
  template <std::enable_if_t<std::is_copy_constructible_v<T>, bool>>
  Indirect(const Indirect<T> &i) : _obj(i._obj), _gen(i._gen) {};
  Indirect(T &&obj) : _obj(obj) {};
  Indirect(T (&&gen)()) : _gen(std::function<T()>(gen)) {};
  Indirect(const std::function<T()> &gen) : _gen(gen) {};

  // template<std::enable_if_t<!std::is_copy_constructible_v<T>, bool>>
  // T& get() {
  //     if (_obj.has_value()) {
  //         std::optional<T> obj = std::nullopt;
  //         std::swap(obj, _obj);
  //         return std::move(obj.value());
  //     } else if (_gen.has_value()) {
  //         std::swap(_obj, std::make_optional(_gen.value()()));
  //         return std::move(_obj.value());
  //     } else {
  //         throw exception(std::string("Indirect of type ") + typeid(T).name() + " is unbound");
  //     }
  // };

  // template<std::enable_if_t<std::is_copy_constructible_v<T>, bool>>
  // T& get() {
  //     if (_obj.has_value()) {
  //         return _obj.value();
  //     } else if (_gen.has_value()) {
  //         _obj = _gen.value()();
  //         return _obj.value();
  //     } else {
  //         throw exception(std::string("Indirect of type ") + typeid(T).name() + " is unbound");
  //     }
  // };

  template <class = std::enable_if<!std::is_copy_constructible_v<T>>>
  T &&get() {
    if (_obj.has_value()) {
      std::optional<T> obj = std::nullopt;
      std::swap(obj, _obj);
      return std::move(obj.value());
    } else if (_gen.has_value()) {
      std::swap(_obj, std::make_optional(_gen.value()()));
      return std::move(_obj.value());
    } else {
      throw exception(std::string("Indirect of type ") + typeid(T).name() + " is unbound");
    }
  };

  template <class = std::enable_if<std::is_copy_constructible_v<T>>>
  T &&get() const {
    if (_obj.has_value()) {
      return _obj.value();
    } else if (_gen.has_value()) {
      _obj = _gen.value()();
      return _obj.value();
    } else {
      throw exception(std::string("Indirect of type ") + typeid(T).name() + " is unbound");
    }
  };

  operator T() const { return get(); }

  operator T &() const { return get(); }

  operator const T &() const { return get(); }

  bool has_value() const noexcept { return _obj.has_value() || _gen.has_value(); }

  protected:
  std::optional<T> _obj;
  std::optional<std::function<T()>> _gen;

  private:
};
