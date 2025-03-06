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

template <typename t>
class Indirect;

template <typename Type>
struct is_indirect : std::false_type {};

template <typename Type>
struct is_indirect<Indirect<Type>> : std::true_type {};

template <typename t>
struct remove_indirect_t {
  using value_type = t;
};

template <typename t>
struct remove_indirect_t<Indirect<t>> {
  using value_type = t;
};

template <typename t>
struct remove_indirect_t<Indirect<t &>> {
  using value_type = t &;
};

template <typename t>
using remove_indirect = typename remove_indirect_t<t>::value_type;

template <typename t>
constexpr Indirect<t> make_indirect(const std::function<t()> &f) {
  return Indirect<t>(f);
}

template <typename t>
class Indirect {
  public:
  using value_type = t;
  class exception : public std::exception {
    private:
    std::string msg;

    public:
    exception(const std::string &str) : msg(str){};
    const char *what() const noexcept override { return msg.c_str(); }
  };

  template <std::enable_if_t<std::is_move_constructible_v<t>, bool>>
  Indirect(Indirect<t> &&i) : _obj(std::move(i._obj)), _gen(std::move(i._gen)){};
  template <std::enable_if_t<std::is_copy_constructible_v<t>, bool>>
  Indirect(const Indirect<t> &i) : _obj(i._obj), _gen(i._gen){};
  Indirect(t &&obj) : _obj(obj){};
  Indirect(t (&&gen)()) : _gen(std::function<t()>(gen)){};
  Indirect(const std::function<t()> &gen) : _gen(gen){};

  // template<std::enable_if_t<!std::is_copy_constructible_v<t>, bool>>
  // t& get() {
  //     if (_obj.has_value()) {
  //         std::optional<t> obj = std::nullopt;
  //         std::swap(obj, _obj);
  //         return std::move(obj.value());
  //     } else if (_gen.has_value()) {
  //         std::swap(_obj, std::make_optional(_gen.value()()));
  //         return std::move(_obj.value());
  //     } else {
  //         throw exception(std::string("Indirect of type ") + typeid(t).name() + " is unbound");
  //     }
  // };

  // template<std::enable_if_t<std::is_copy_constructible_v<t>, bool>>
  // t& get() {
  //     if (_obj.has_value()) {
  //         return _obj.value();
  //     } else if (_gen.has_value()) {
  //         _obj = _gen.value()();
  //         return _obj.value();
  //     } else {
  //         throw exception(std::string("Indirect of type ") + typeid(t).name() + " is unbound");
  //     }
  // };

  template <class = std::enable_if<!std::is_copy_constructible_v<t>>>
  t &&get() {
    if (_obj.has_value()) {
      std::optional<t> obj = std::nullopt;
      std::swap(obj, _obj);
      return std::move(obj.value());
    } else if (_gen.has_value()) {
      std::swap(_obj, std::make_optional(_gen.value()()));
      return std::move(_obj.value());
    } else {
      throw exception(std::string("Indirect of type ") + typeid(t).name() + " is unbound");
    }
  };

  template <class = std::enable_if<std::is_copy_constructible_v<t>>>
  t &&get() const {
    if (_obj.has_value()) {
      return _obj.value();
    } else if (_gen.has_value()) {
      _obj = _gen.value()();
      return _obj.value();
    } else {
      throw exception(std::string("Indirect of type ") + typeid(t).name() + " is unbound");
    }
  };

  operator t() const { return get(); }

  operator t &() const { return get(); }

  operator const t &() const { return get(); }

  bool has_value() const noexcept { return _obj.has_value() || _gen.has_value(); }

  protected:
  std::optional<t> _obj;
  std::optional<std::function<t()>> _gen;

  private:
};
