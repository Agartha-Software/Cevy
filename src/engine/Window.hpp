/*
** Agartha-Software, 2024
** C++evy
** File description:
** generic window handling
*/

#pragma once

#include "Plugin.hpp"
#include "cursor.hpp"
#include <glm/glm.hpp>
#include <memory>
#include <type_traits>

namespace cevy::engine {
class Window {
  public:
  class GenericWindow {
    public:
    glm::vec<2, int>  windowSize;
    glm::vec<2, int>  renderSize;
    bool fullscreen;
    GenericWindow(int width, int height, bool fullscreen) : windowSize(width, height), renderSize(width, height), fullscreen(fullscreen) {};

    virtual void setWindowSize(int width, int height) = 0;
    virtual void setRenderSize(int width, int height) = 0;
    virtual void setFullscreen(bool fullscreen) = 0;
    virtual void setCursorState(CursorState state) = 0;

    using Plugin = ecs::NullPlugin;
  };
  template <typename Windower>
  Window(Windower &&win) {
    this->window = std::make_shared<Windower>(std::forward<Windower>(win));
  }

  template <typename Windower,
            std::enable_if_t<std::is_base_of_v<GenericWindow, Windower>>>
  Window(int width, int height) {
    this->window = std::make_shared<Windower>(width, height);
  }

  template <typename Windower>
  Windower &get_handler() {
    return dynamic_cast<Windower &>(*this->window);
  }

  const glm::vec<2, int> &windowSize() const { return this->window->windowSize; }
  const glm::vec<2, int> &renderSize() const { return this->window->renderSize; }
  bool fullscreen() const { return this->window->fullscreen; }
  void setWindowSize(int width, int height) { this->window->setWindowSize(width, height); }
  void setRenderSize(int width, int height) { this->window->setRenderSize(width, height); }
  void setFullscreen(bool fullscreen) { this->window->setFullscreen(fullscreen); }
  void setCursorState(CursorState state) { return this->window->setCursorState(state); }

  GenericWindow *operator->() { return window.get(); }

  protected:
  std::shared_ptr<GenericWindow> window;
};
} // namespace cevy::engine
