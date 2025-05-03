/*
** Agartha-Software, 2023
** C++evy
** File description:
** Handle
*/
#pragma once

#include "cevy.hpp"
#include <any>
#include <functional>
#include <memory>
#include <optional>
#include <type_traits>
#include <typeindex>

namespace cevy::engine {
struct AssetId {
  size_t id;
  operator const size_t &() const { return id; }
};

struct ErasedAssetId {
  AssetId id;
  std::type_index type;
};

template <typename>
class Assets;
template <typename>
class AssetsData;
class ErasedHandle;

template <typename A>
class Handle : protected std::shared_ptr<std::optional<A>> {
  friend class Assets<A>;
  friend class AssetsData<A>;

  public:
  using value = A;
  using shared = std::shared_ptr<std::optional<A>>;

  // explicit Handle() : shared(nullptr) {};
  protected:
  AssetId id;
  Handle &replace(A &&asset) {
    shared::get()->emplace(std::forward<A>(asset));
    return *this;
  }
  template <typename... Args>
  Handle &emplace(Args &&...args) {
    shared::get()->emplace(std::forward<A>(std::forward<A>(args)...));
    return *this;
  }
  // const A *ptr() const { return shared().get(); }

  public:
  Handle(const shared &ptr, AssetId id) : shared(ptr), id(id) {};
  Handle(const Handle &rhs) : shared(rhs), id(rhs.id) {};
  Handle(Handle &&rhs) : shared(std::forward<shared>(rhs)), id(rhs.id) {};
  // Handle(A &&ref) : shared(std::make_shared<typename
  // shared::element_type>(std::make_optional(std::forward<A>(ref)))) {}; template <typename...
  // Args> Handle(Args &&...arg) : shared(std::make_shared<A>(std::forward<Args>(arg)...)) {};

  const A *operator->() const { return &shared::get()->value(); }
  A *operator->() { return &shared::get()->value(); }
  A &get() { return shared::get()->value(); }
  const A &get() const { return shared::get()->value(); }
  const shared &share() const { return *this; }

  operator AssetId() const { return id; }

  operator ErasedAssetId() const { return {id, typeid(A)}; }

  bool operator==(const Handle<A> rhs) const { return shared::get() == rhs.shared::get(); }

  Handle &operator=(Handle<A> &&rhs) {
    shared::operator=(std::forward<shared>(shared(rhs)));
    return *this;
  }

  Handle &operator=(const Handle<A> &rhs) {
    shared::operator=(rhs.share());
    return *this;
  }

  template <typename A2, class = std::enable_if_t<std::conjunction_v<
                             std::is_same<A, A2>, std::is_copy_constructible<A2>>>>
  static Handle Clone(const Handle<A2> source) {
    return Handle(std::make_shared<typename shared::element_type>(source.get()),
                  AssetId {size_t(-1l)});
  }
};

class ErasedHandle {
  cevy::any handle;
  std::function<cevy::any(const cevy::any &)> copy;

  public:
  std::type_index type;
  AssetId id;
  template <typename A>
  ErasedHandle(Handle<A> &&handle)
      : handle(cevy::make_any<Handle<A>>(handle)), type(typeid(A)), id(handle) {
    this->copy = [](const cevy::any &erased) {
      return cevy::make_any<Handle<A>>(std::any_cast<const Handle<A> &>(erased));
    };
  };
  ErasedHandle(ErasedHandle &&rhs)
      : handle(std::forward<cevy::any>(rhs.handle)), copy(rhs.copy), type(rhs.type), id(rhs.id) {};

  ErasedHandle(const ErasedHandle &rhs) : copy(rhs.copy), type(rhs.type), id(rhs.id) {
    this->handle = this->copy(rhs.handle);
  }
  template <typename A>
  operator Handle<A> &&() && {
    return std::move(*this).cast<A>();
  }
  template <typename A>
  Handle<A> &&cast() && {
    return std::any_cast<Handle<A> &&>(std::move(this->handle));
  }
  template <typename A>
  Handle<A> &cast() & {
    return std::any_cast<Handle<A> &>(this->handle);
  }
  template <typename A>
  const Handle<A> &cast() const & {
    return std::any_cast<const Handle<A> &>(this->handle);
  }
};

} // namespace cevy::engine

namespace std {
template <typename T>
struct hash<cevy::engine::Handle<T>> {
  std::size_t operator()(const cevy::engine::Handle<T> &handle) const noexcept {
    return std::hash<void *> {}(handle.share().get());
    // return std::hash<void *> {}(static_cast<void *>(handle._M_ptr));
  }
};
// template <typename A>
// class optional<cevy::engine::Handle<A>> : public cevy::engine::Handle<A> {
//   public:
//   optional(std::nullopt_t) {}
//   // optional<cevy::engine::Handle<A>>(std::nullopt_t) {}
//   operator bool() const { return *this != nullptr; }

//   bool operator==(nullptr_t) const { return this->ptr() == nullptr; }
//   bool operator!=(nullptr_t) const { return this->ptr() != nullptr; }
// };
}; // namespace std
