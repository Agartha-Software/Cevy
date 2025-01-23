/*
** Agartha-Software, 2023
** C++evy
** File description:
** Handle
*/
#pragma once

#include "cevy.hpp"
#include <memory>

namespace cevy::engine {

template <typename Type>
class Handle {
  using value = Type;

  std::shared_ptr<Type> _ref;

  public:
  Handle(Type &ref) : _ref(std::make_shared<Type>(ref)){};
  Handle(Type &&ref) : _ref(std::make_shared<Type>(std::forward<Type>(ref))){};

  const Type* operator->() const { return _ref.get(); }
  Type* operator->() { return _ref.get(); }
  std::shared_ptr<Type> get() { return _ref; }
};

} // namespace cevy::engine
