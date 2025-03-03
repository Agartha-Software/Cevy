/*
** Agartha-Software, 2024
** C++evy
** File description:
** generic window handling
*/

#pragma once

#include "Plugin.hpp"
#include <glm/glm.hpp>
#include <memory>
#include <type_traits>

namespace cevy::engine {
class Window {
  public:
  struct generic_window {
    generic_window(){};
    virtual bool open() = 0;
    virtual void pollEvents() = 0;

    virtual glm::vec<2, int> size() const = 0;
    virtual void setSize(int width, int height) = 0;
    virtual void setFullscreen(bool fullscreen) = 0;
    using Plugin = ecs::NullPlugin;
  };
  template <template <typename> typename Windower, typename Renderer>
  Window(Windower<Renderer> &&win) {
    this->window = std::make_shared<Windower<Renderer>>(std::forward<Windower<Renderer>>(win));
  }

  template <template <typename> typename Windower, typename Renderer,
            std::enable_if_t<std::is_base_of_v<generic_window, Windower<Renderer>>>>
  Window(int width, int height) {
    this->window = std::make_shared<Windower<Renderer>>(width, height);
  }
  template <template <typename> typename Windower, typename Renderer>
  Windower<Renderer> *operator->() {
    return dynamic_cast<Windower<Renderer> *>(this->window.get());
  }
  template <template <typename> typename Windower, typename Renderer>
  Windower<Renderer> *get_handler() {
    return dynamic_cast<Windower<Renderer> *>(this->window.get());
  }

  bool open() { return this->window->open(); }
  glm::vec<2, int> size() const { return this->window->size(); }
  void setSize(int width, int height) { this->window->setSize(width, height); }
  void setFullscreen(bool fullscreen) { this->window->setFullscreen(fullscreen); }
  generic_window *operator->() { return window.get(); }

  protected:
  std::shared_ptr<generic_window> window;
};
} // namespace cevy::engine
