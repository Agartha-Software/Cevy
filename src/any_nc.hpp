// <any_nc> -*- C++ -*-

// This file is a derivative of GCC's library implementation of std::any_nc
// It has been created for the express purpose of allowing a different usage
// of the core design of std::any_nc, namely to remove the requirement of copy constructability
// This alteration has been made in 2025
//
// The file has been modified in the following ways:
//  - compiler definitions indicating a core library file have been removed
//  - include guard has been changed to _CUSTOM_ANY_NC
//  - utility definitions in (std::) have been omitted and are served by an inclusion of <any>
//  - the eponymous class name has been changed to 'any_nc', meaning 'any non-copy'
//  - copy constructors and assignments have been deleted
//  - internal manager clone operator removed
//  - constructability template has been altered to require move constructability instead of copy
//
// This file does not provide any aliasing definitions, this file is intended to be used
// without obstructing the use of std::any
// this file provides std::make_any_nc as its equivalent to std::make_any
// this file provides these overloads and specifications to standard definitions:
//  - void std::swap(any_nc&, any_nc&)
//  - T std::any_cast<T>(const any_nc&)
//  - T std::any_cast<T>(any_nc&)
//  - T std::any_cast<T>(any_nc&&)
//  - void* __any_caster(const any_nc*)
//  - T* std::any_cast<T>(const any_nc*)
//  - T* std::any_cast<T>(any_nc*)
//  - std::__detail::__variant::_Never_valueless_alt<any_nc>
//
// Note that using an any_nc prevents copying any_nc, but allows copying out of
// any_nc with any_cast when using a CopyConstructible type
//
// The decision to remain in std::, as well as any other decision is not motivated
// for any particular reason other than conveniance.

// This modified file is licenced by the Free Software Foundation,
// the following is the original notice

// Copyright (C) 2014-2024 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 3, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Under Section 7 of GPL version 3, you are granted additional
// permissions described in the GCC Runtime Library Exception, version
// 3.1, as published by the Free Software Foundation.

// You should have received a copy of the GNU General Public License and
// a copy of the GCC Runtime Library Exception along with this program;
// see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
// <http://www.gnu.org/licenses/>.

#include <any>

#ifndef _CUSTOM_ANY_NC
#define _CUSTOM_ANY_NC 1

// #pragma GCC system_header

#define __glibcxx_want_any
// #include <bits/version.h>

#ifdef __cpp_lib_any // C++ >= 17

#include <initializer_list>
#include <new>
#include <type_traits>
#include <typeinfo>
#include <utility> // in_place_type_t

namespace std {

/**
 *  @addtogroup utilities
 *  @{
 */

//   /**
//    *  @brief Exception class thrown by a failed @c any_cast
//    *  @ingroup exceptions
//    */
//   class bad_any_cast : public bad_cast
//   {
//   public:
//     virtual const char* what() const noexcept { return "bad any_cast"; }
//   };

//   [[gnu::noreturn]] inline void __throw_bad_any_cast()
//   {
// #if __cpp_exceptions
//     throw bad_any_cast{};
// #else
//     __builtin_abort();
// #endif
//   }

/**
 *  @brief A type-safe container of any type.
 *
 *  An `any_nc` object's state is either empty or it stores a contained object
 *  of MoveConstructible type.
 *
 *  @since C++17
 */
using namespace std;
class any_nc {
  template <typename T>
  struct is_in_place_type : false_type {}; /// not provided by standard

  template <typename T>
  struct is_in_place_type<in_place_type_t<T>> : std::true_type {};

  template <typename _Tp>
  using remove_cvref_t = typename remove_cv<typename remove_reference<_Tp>::type>::type;

  // Holds either pointer to a heap object or the contained object itself.
  union _Storage {
    constexpr _Storage() : _M_ptr {nullptr} {}

    // Prevent trivial copies of this type, buffer might hold a non-POD.
    _Storage(const _Storage &) = delete;
    _Storage &operator=(const _Storage &) = delete;

    void *_M_ptr;
    aligned_storage<sizeof(_M_ptr), alignof(void *)>::type _M_buffer;
  };

  template <typename _Tp, typename _Safe = is_nothrow_move_constructible<_Tp>,
            bool _Fits = (sizeof(_Tp) <= sizeof(_Storage)) && (alignof(_Tp) <= alignof(_Storage))>
  using _Internal = std::integral_constant<bool, _Safe::value && _Fits>;

  template <typename _Tp>
  struct _Manager_internal; // uses small-object optimization

  template <typename _Tp>
  struct _Manager_external; // creates contained object on the heap

  template <typename _Tp>
  using _Manager =
      conditional_t<_Internal<_Tp>::value, _Manager_internal<_Tp>, _Manager_external<_Tp>>;

  template <typename _Tp, typename _VTp = decay_t<_Tp>>
  using _Decay_if_not_any = enable_if_t<!is_same_v<_VTp, any_nc>, _VTp>;

  /// Emplace with an object created from @p __args as the contained object.
  template <typename _Tp, typename... _Args, typename _Mgr = _Manager<_Tp>>
  void __do_emplace(_Args &&...__args) {
    reset();
    _Mgr::_S_create(_M_storage, std::forward<_Args>(__args)...);
    _M_manager = &_Mgr::_S_manage;
  }

  /// Emplace with an object created from @p __il and @p __args as
  /// the contained object.
  template <typename _Tp, typename _Up, typename... _Args, typename _Mgr = _Manager<_Tp>>
  void __do_emplace(initializer_list<_Up> __il, _Args &&...__args) {
    reset();
    _Mgr::_S_create(_M_storage, __il, std::forward<_Args>(__args)...);
    _M_manager = &_Mgr::_S_manage;
  }

  template <typename _Res, typename _Tp, typename... _Args>
  using __any_constructible =
      enable_if<conjunction<is_move_constructible<_Tp>, is_constructible<_Tp, _Args...>>::value,
                _Res>;

  template <typename _Tp, typename... _Args>
  using __any_constructible_t = typename __any_constructible<bool, _Tp, _Args...>::type;

  template <typename _VTp, typename... _Args>
  using __emplace_t = typename __any_constructible<_VTp &, _VTp, _Args...>::type;

  public:
  // construct/destruct

  /// Default constructor, creates an empty object.
  constexpr any_nc() noexcept : _M_manager(nullptr) {}

  // /// Copy constructor, copies the state of @p __other
  // any_nc(const any_nc& __other)
  // {
  //   if (!__other.has_value())
  // _M_manager = nullptr;
  //   else
  // {
  //   _Arg __arg;
  //   __arg._M_any = this;
  //   __other._M_manager(_Op_clone, &__other, &__arg);
  // }
  // }
  any_nc(const any_nc &__other) = delete;

  /**
   * @brief Move constructor, transfer the state from @p __other
   *
   * @post @c !__other.has_value() (this postcondition is a GNU extension)
   */
  any_nc(any_nc &&__other) noexcept {
    if (!__other.has_value())
      _M_manager = nullptr;
    else {
      _Arg __arg;
      __arg._M_any = this;
      __other._M_manager(_Op_xfer, &__other, &__arg);
    }
  }

  /// Construct with a copy of @p __value as the contained object.
  template <
      typename _Tp, typename _VTp = _Decay_if_not_any<_Tp>, typename _Mgr = _Manager<_VTp>,
      enable_if_t<is_copy_constructible_v<_VTp> && !is_in_place_type<_VTp>::value, bool> = true>
  any_nc(_Tp &&__value) : _M_manager(&_Mgr::_S_manage) {
    _Mgr::_S_create(_M_storage, std::forward<_Tp>(__value));
  }

  /// Construct with an object created from @p __args as the contained object.
  template <typename _Tp, typename... _Args, typename _VTp = decay_t<_Tp>,
            typename _Mgr = _Manager<_VTp>, __any_constructible_t<_VTp, _Args &&...> = false>
  explicit any_nc(in_place_type_t<_Tp>, _Args &&...__args) : _M_manager(&_Mgr::_S_manage) {
    _Mgr::_S_create(_M_storage, std::forward<_Args>(__args)...);
  }

  /// Construct with an object created from @p __il and @p __args as
  /// the contained object.
  template <typename _Tp, typename _Up, typename... _Args, typename _VTp = decay_t<_Tp>,
            typename _Mgr = _Manager<_VTp>,
            __any_constructible_t<_VTp, initializer_list<_Up> &, _Args &&...> = false>
  explicit any_nc(in_place_type_t<_Tp>, initializer_list<_Up> __il, _Args &&...__args)
      : _M_manager(&_Mgr::_S_manage) {
    _Mgr::_S_create(_M_storage, __il, std::forward<_Args>(__args)...);
  }

  /// Destructor, calls @c reset()
  ~any_nc() { reset(); }

  // assignments

  /// Copy the state of another object.
  // any_nc&
  // operator=(const any_nc& __rhs)
  // {
  //   *this = any_nc(__rhs);
  //   return *this;
  // }
  any_nc &operator=(const any_nc &__rhs) = delete;

  /**
   * @brief Move assignment operator
   *
   * @post @c !__rhs.has_value() (not guaranteed for other implementations)
   */
  any_nc &operator=(any_nc &&__rhs) noexcept {
    if (!__rhs.has_value())
      reset();
    else if (this != &__rhs) {
      reset();
      _Arg __arg;
      __arg._M_any = this;
      __rhs._M_manager(_Op_xfer, &__rhs, &__arg);
    }
    return *this;
  }

  /// Store a copy of @p __rhs as the contained object.
  template <typename _Tp>
  enable_if_t<is_copy_constructible<_Decay_if_not_any<_Tp>>::value, any_nc &>
  operator=(_Tp &&__rhs) {
    *this = any_nc(std::forward<_Tp>(__rhs));
    return *this;
  }

  /// Emplace with an object created from @p __args as the contained object.
  template <typename _Tp, typename... _Args>
  __emplace_t<decay_t<_Tp>, _Args...> emplace(_Args &&...__args) {
    using _VTp = decay_t<_Tp>;
    __do_emplace<_VTp>(std::forward<_Args>(__args)...);
    return *any_nc::_Manager<_VTp>::_S_access(_M_storage);
  }

  /// Emplace with an object created from @p __il and @p __args as
  /// the contained object.
  template <typename _Tp, typename _Up, typename... _Args>
  __emplace_t<decay_t<_Tp>, initializer_list<_Up> &, _Args &&...>
  emplace(initializer_list<_Up> __il, _Args &&...__args) {
    using _VTp = decay_t<_Tp>;
    __do_emplace<_VTp, _Up>(__il, std::forward<_Args>(__args)...);
    return *any_nc::_Manager<_VTp>::_S_access(_M_storage);
  }

  // modifiers

  /// If not empty, destroy the contained object.
  void reset() noexcept {
    if (has_value()) {
      _M_manager(_Op_destroy, this, nullptr);
      _M_manager = nullptr;
    }
  }

  /// Exchange state with another object.
  void swap(any_nc &__rhs) noexcept {
    if (!has_value() && !__rhs.has_value())
      return;

    if (has_value() && __rhs.has_value()) {
      if (this == &__rhs)
        return;

      any_nc __tmp;
      _Arg __arg;
      __arg._M_any = &__tmp;
      __rhs._M_manager(_Op_xfer, &__rhs, &__arg);
      __arg._M_any = &__rhs;
      _M_manager(_Op_xfer, this, &__arg);
      __arg._M_any = this;
      __tmp._M_manager(_Op_xfer, &__tmp, &__arg);
    } else {
      any_nc *__empty = !has_value() ? this : &__rhs;
      any_nc *__full = !has_value() ? &__rhs : this;
      _Arg __arg;
      __arg._M_any = __empty;
      __full->_M_manager(_Op_xfer, __full, &__arg);
    }
  }

  // observers

  /// Reports whether there is a contained object or not.
  bool has_value() const noexcept { return _M_manager != nullptr; }

#if __cpp_rtti
  /// The @c typeid of the contained object, or @c typeid(void) if empty.
  const type_info &type() const noexcept {
    if (!has_value())
      return typeid(void);
    _Arg __arg;
    _M_manager(_Op_get_type_info, this, &__arg);
    return *__arg._M_typeinfo;
  }
#endif

  /// @cond undocumented
  template <typename _Tp>
  static constexpr bool __is_valid_cast() {
    return disjunction<is_reference<_Tp>, is_copy_constructible<_Tp>>::value;
  }
  /// @endcond

  private:
  enum _Op { _Op_access, _Op_get_type_info, /* _Op_clone, */ _Op_destroy, _Op_xfer };

  union _Arg {
    void *_M_obj;
    const std::type_info *_M_typeinfo;
    any_nc *_M_any;
  };

  void (*_M_manager)(_Op, const any_nc *, _Arg *);
  _Storage _M_storage;

  /// @cond undocumented
  template <typename _Tp>
  friend void *__any_caster(const any_nc *__any);
  /// @endcond

  // Manage in-place contained object.
  template <typename _Tp>
  struct _Manager_internal {
    static void _S_manage(_Op __which, const any_nc *__anyp, _Arg *__arg);

    template <typename _Up>
    static void _S_create(_Storage &__storage, _Up &&__value) {
      void *__addr = &__storage._M_buffer;
      ::new (__addr) _Tp(std::forward<_Up>(__value));
    }

    template <typename... _Args>
    static void _S_create(_Storage &__storage, _Args &&...__args) {
      void *__addr = &__storage._M_buffer;
      ::new (__addr) _Tp(std::forward<_Args>(__args)...);
    }

    static _Tp *_S_access(const _Storage &__storage) {
      // The contained object is in __storage._M_buffer
      const void *__addr = &__storage._M_buffer;
      return static_cast<_Tp *>(const_cast<void *>(__addr));
    }
  };

  // Manage external contained object.
  template <typename _Tp>
  struct _Manager_external {
    static void _S_manage(_Op __which, const any_nc *__anyp, _Arg *__arg);

    template <typename _Up>
    static void _S_create(_Storage &__storage, _Up &&__value) {
      __storage._M_ptr = new _Tp(std::forward<_Up>(__value));
    }
    template <typename... _Args>
    static void _S_create(_Storage &__storage, _Args &&...__args) {
      __storage._M_ptr = new _Tp(std::forward<_Args>(__args)...);
    }
    static _Tp *_S_access(const _Storage &__storage) {
      // The contained object is in *__storage._M_ptr
      return static_cast<_Tp *>(__storage._M_ptr);
    }
  };
};

/// Exchange the states of two @c any_nc objects.
inline void swap(any_nc &__x, any_nc &__y) noexcept { __x.swap(__y); }

/// Create an `any_nc` holding a `_Tp` constructed from `__args...`.
template <typename _Tp, typename... _Args>
inline enable_if_t<is_constructible_v<any_nc, in_place_type_t<_Tp>, _Args...>, any_nc>
make_any_nc(_Args &&...__args) {
  return any_nc(in_place_type<_Tp>, std::forward<_Args>(__args)...);
}

/// Create an `any_nc` holding a `_Tp` constructed from `__il` and `__args...`.
template <typename _Tp, typename _Up, typename... _Args>
inline enable_if_t<
    is_constructible_v<any_nc, in_place_type_t<_Tp>, initializer_list<_Up> &, _Args...>, any_nc>
make_any_nc(initializer_list<_Up> __il, _Args &&...__args) {
  return any_nc(in_place_type<_Tp>, __il, std::forward<_Args>(__args)...);
}

/**
 * @brief Access the contained object.
 *
 * @tparam  _ValueType  A const-reference or CopyConstructible type.
 * @param   __any       The object to access.
 * @return  The contained object.
 * @throw   bad_any_cast If <code>
 *          __any.type() != typeid(remove_reference_t<_ValueType>)
 *          </code>
 */
template <typename _ValueType>
inline _ValueType any_cast(const any_nc &__any) {
  using _Up = std::remove_cv_t<std::remove_reference_t<_ValueType>>;
  static_assert(any_nc::__is_valid_cast<_ValueType>(),
                "Template argument must be a reference or CopyConstructible type");
  static_assert(is_constructible_v<_ValueType, const _Up &>,
                "Template argument must be constructible from a const value.");
  auto __p = any_cast<_Up>(&__any);
  if (__p)
    return static_cast<_ValueType>(*__p);
  throw std::bad_any_cast();
}

/**
 * @brief Access the contained object.
 *
 * @tparam  _ValueType  A reference or CopyConstructible type.
 * @param   __any       The object to access.
 * @return  The contained object.
 * @throw   bad_any_cast If <code>
 *          __any.type() != typeid(remove_reference_t<_ValueType>)
 *          </code>
 *
 * @{
 */
template <typename _ValueType>
inline _ValueType any_cast(any_nc &__any) {
  using _Up = std::remove_cv_t<std::remove_reference_t<_ValueType>>;
  static_assert(any_nc::__is_valid_cast<_ValueType>(),
                "Template argument must be a reference or CopyConstructible type");
  //   static_assert(is_constructible_v<_ValueType, _Up&>,
  // "Template argument must be constructible from an lvalue.");
  auto __p = any_cast<_Up>(&__any);
  if (__p)
    return static_cast<_ValueType>(*__p);
  throw std::bad_any_cast();
}

template <typename _ValueType>
inline _ValueType any_cast(any_nc &&__any) {
  using _Up = std::remove_cv_t<std::remove_reference_t<_ValueType>>;
  static_assert(any_nc::__is_valid_cast<_ValueType>(),
                "Template argument must be a reference or CopyConstructible type");
  static_assert(is_constructible_v<_ValueType, _Up>,
                "Template argument must be constructible from an rvalue.");
  auto __p = any_cast<_Up>(&__any);
  if (__p)
    return static_cast<_ValueType>(std::move(*__p));
  throw std::bad_any_cast();
}
/// @}

/// @cond undocumented
template <typename _Tp>
void *__any_caster(const any_nc *__any) {
  // any_cast<T> returns non-null if __any->type() == typeid(T) and
  // typeid(T) ignores cv-qualifiers so remove them:
  using _Up = remove_cv_t<_Tp>;
  // The contained value has a decayed type, so if decay_t<U> is not U,
  // then it's not possible to have a contained value of type U:
  if constexpr (!is_same_v<decay_t<_Up>, _Up>)
    return nullptr;
  // Only copy constructible types can be used for contained values:
  //     else if constexpr (!is_copy_constructible_v<_Up>)
  // return nullptr;
  // First try comparing function addresses, which works without RTTI
  else if (__any->_M_manager == &any_nc::_Manager<_Up>::_S_manage
#if __cpp_rtti
           || __any->type() == typeid(_Tp)
#endif
  ) {
    return any_nc::_Manager<_Up>::_S_access(__any->_M_storage);
  }
  return nullptr;
}
/// @endcond

/**
 * @brief Access the contained object.
 *
 * @tparam  _ValueType  The type of the contained object.
 * @param   __any       A pointer to the object to access.
 * @return  The address of the contained object if <code>
 *          __any != nullptr && __any.type() == typeid(_ValueType)
 *          </code>, otherwise a null pointer.
 *
 * @{
 */
template <typename _ValueType>
inline const _ValueType *any_cast(const any_nc *__any) noexcept {
  // _GLIBCXX_RESOLVE_LIB_DEFECTS
  // 3305. any_cast<void>
  static_assert(!is_void_v<_ValueType>);

  // As an optimization, don't bother instantiating __any_caster for
  // function types, since std::any_nc can only hold objects.
  if constexpr (is_object_v<_ValueType>)
    if (__any)
      return static_cast<_ValueType *>(__any_caster<_ValueType>(__any));
  return nullptr;
}

template <typename _ValueType>
inline _ValueType *any_cast(any_nc *__any) noexcept {
  static_assert(!is_void_v<_ValueType>);

  if constexpr (is_object_v<_ValueType>)
    if (__any)
      return static_cast<_ValueType *>(__any_caster<_ValueType>(__any));
  return nullptr;
}
/// @}

template <typename _Tp>
void any_nc::_Manager_internal<_Tp>::_S_manage(_Op __which, const any_nc *__any, _Arg *__arg) {
  // The contained object is in _M_storage._M_buffer
  auto __ptr = reinterpret_cast<const _Tp *>(&__any->_M_storage._M_buffer);
  switch (__which) {
  case _Op_access:
    __arg->_M_obj = const_cast<_Tp *>(__ptr);
    break;
  case _Op_get_type_info:
#if __cpp_rtti
    __arg->_M_typeinfo = &typeid(_Tp);
#endif
    break;
    //     case _Op_clone:
    // ::new(&__arg->_M_any->_M_storage._M_buffer) _Tp(*__ptr);
    // __arg->_M_any->_M_manager = __any->_M_manager;
    break;
  case _Op_destroy:
    __ptr->~_Tp();
    break;
  case _Op_xfer:
    ::new (&__arg->_M_any->_M_storage._M_buffer) _Tp(std::move(*const_cast<_Tp *>(__ptr)));
    __ptr->~_Tp();
    __arg->_M_any->_M_manager = __any->_M_manager;
    const_cast<any_nc *>(__any)->_M_manager = nullptr;
    break;
  }
}

template <typename _Tp>
void any_nc::_Manager_external<_Tp>::_S_manage(_Op __which, const any_nc *__any, _Arg *__arg) {
  // The contained object is *_M_storage._M_ptr
  auto __ptr = static_cast<const _Tp *>(__any->_M_storage._M_ptr);
  switch (__which) {
  case _Op_access:
    __arg->_M_obj = const_cast<_Tp *>(__ptr);
    break;
  case _Op_get_type_info:
#if __cpp_rtti
    __arg->_M_typeinfo = &typeid(_Tp);
#endif
    break;
    //     case _Op_clone:
    // __arg->_M_any->_M_storage._M_ptr = new _Tp(*__ptr);
    // __arg->_M_any->_M_manager = __any->_M_manager;
    break;
  case _Op_destroy:
    delete __ptr;
    break;
  case _Op_xfer:
    __arg->_M_any->_M_storage._M_ptr = __any->_M_storage._M_ptr;
    __arg->_M_any->_M_manager = __any->_M_manager;
    const_cast<any_nc *>(__any)->_M_manager = nullptr;
    break;
  }
}

/// @}

namespace __detail::__variant {
template <typename>
struct _Never_valueless_alt; // see <variant>

// Provide the strong exception-safety guarantee when emplacing an
// any_nc into a variant.
template <>
struct _Never_valueless_alt<std::any_nc> : std::true_type {};
} // namespace __detail::__variant

} // namespace std

#endif // __cpp_lib_any
#endif // _CUSTOM_ANY_NC
