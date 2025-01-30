/*
** AgarthaSoftware, 2024
** Cevy
** File description:
** Input State
*/

#pragma once

#include <GLFW/glfw3.h>
#include <cstdint>
#include <unordered_set>

namespace cevy {
namespace input {

enum class KeyCode: unsigned int {
  Space = GLFW_KEY_SPACE,
  Apostriophe = GLFW_KEY_APOSTROPHE,
  Comma = GLFW_KEY_COMMA, /* , */
  Minus = GLFW_KEY_MINUS, /* - */
  Period = GLFW_KEY_PERIOD, /* . */
  Dot = GLFW_KEY_PERIOD, /* . */
  Slash = GLFW_KEY_SLASH, /* / */
  K0 = GLFW_KEY_0,
  K1 = GLFW_KEY_1,
  K2 = GLFW_KEY_2,
  K3 = GLFW_KEY_3,
  K4 = GLFW_KEY_4,
  K5 = GLFW_KEY_5,
  K6 = GLFW_KEY_6,
  K7 = GLFW_KEY_7,
  K8 = GLFW_KEY_8,
  K9 = GLFW_KEY_9,
  Semicolon = GLFW_KEY_SEMICOLON, /* ; */
  Equal = GLFW_KEY_EQUAL, /* = */
  A = GLFW_KEY_A,
  B = GLFW_KEY_B,
  C = GLFW_KEY_C,
  D = GLFW_KEY_D,
  E = GLFW_KEY_E,
  F = GLFW_KEY_F,
  G = GLFW_KEY_G,
  H = GLFW_KEY_H,
  I = GLFW_KEY_I,
  J = GLFW_KEY_J,
  K = GLFW_KEY_K,
  L = GLFW_KEY_L,
  M = GLFW_KEY_M,
  N = GLFW_KEY_N,
  O = GLFW_KEY_O,
  P = GLFW_KEY_P,
  Q = GLFW_KEY_Q,
  R = GLFW_KEY_R,
  S = GLFW_KEY_S,
  T = GLFW_KEY_T,
  U = GLFW_KEY_U,
  V = GLFW_KEY_V,
  W = GLFW_KEY_W,
  X = GLFW_KEY_X,
  Y = GLFW_KEY_Y,
  Z = GLFW_KEY_Z,
  LeftBracket = GLFW_KEY_LEFT_BRACKET, /* [ */
  BackSlash = GLFW_KEY_BACKSLASH, /* \ */
  RightBracket = GLFW_KEY_RIGHT_BRACKET, /* ] */
  GraveAccent = GLFW_KEY_GRAVE_ACCENT, /* ` */
  NonUS1 = GLFW_KEY_WORLD_1, /* non-US #1 */
  NonUS2 = GLFW_KEY_WORLD_1, /* non-US #2 */
  Escape = GLFW_KEY_ESCAPE,
  Enter = GLFW_KEY_ENTER,
  Tab = GLFW_KEY_TAB,
  Backspace = GLFW_KEY_BACKSPACE,
  Insert = GLFW_KEY_INSERT,
  Delete = GLFW_KEY_DELETE,
  Right = GLFW_KEY_RIGHT,
  RightArrow = GLFW_KEY_RIGHT,
  Left = GLFW_KEY_LEFT,
  LeftArrow = GLFW_KEY_LEFT,
  Down = GLFW_KEY_DOWN,
  DownArrow = GLFW_KEY_DOWN,
  Up = GLFW_KEY_UP,
  UpArrow = GLFW_KEY_UP,
  PageUp = GLFW_KEY_PAGE_UP,
  PageDown = GLFW_KEY_PAGE_DOWN,
  Home = GLFW_KEY_HOME,
  End = GLFW_KEY_END,
  CapsLock = GLFW_KEY_CAPS_LOCK,
  ScrollLock = GLFW_KEY_SCROLL_LOCK,
  NumLock = GLFW_KEY_NUM_LOCK,
  PrintScreen = GLFW_KEY_PRINT_SCREEN,
  Pause = GLFW_KEY_PAUSE,
  F1 = GLFW_KEY_F1,
  F2 = GLFW_KEY_F2,
  F3 = GLFW_KEY_F3,
  F4 = GLFW_KEY_F4,
  F5 = GLFW_KEY_F5,
  F6 = GLFW_KEY_F6,
  F7 = GLFW_KEY_F7,
  F8 = GLFW_KEY_F8,
  F9 = GLFW_KEY_F9,
  F10 = GLFW_KEY_F10,
  F11 = GLFW_KEY_F11,
  F12 = GLFW_KEY_F12,
  F13 = GLFW_KEY_F13,
  F14 = GLFW_KEY_F14,
  F15 = GLFW_KEY_F15,
  F16 = GLFW_KEY_F16,
  F17 = GLFW_KEY_F17,
  F18 = GLFW_KEY_F18,
  F19 = GLFW_KEY_F19,
  F20 = GLFW_KEY_F20,
  F21 = GLFW_KEY_F21,
  F22 = GLFW_KEY_F23,
  F24 = GLFW_KEY_F24,
  F25 = GLFW_KEY_F25,
  KP0 = GLFW_KEY_KP_0,
  KP1 = GLFW_KEY_KP_1,
  KP2 = GLFW_KEY_KP_2,
  KP3 = GLFW_KEY_KP_3,
  KP4 = GLFW_KEY_KP_4,
  KP5 = GLFW_KEY_KP_5,
  KP6 = GLFW_KEY_KP_6,
  KP7 = GLFW_KEY_KP_7,
  KP8 = GLFW_KEY_KP_8,
  KP9 = GLFW_KEY_KP_9,
  KPDecimal = GLFW_KEY_KP_DECIMAL,
  KPDivide = GLFW_KEY_KP_DIVIDE,
  KPMultiply = GLFW_KEY_KP_MULTIPLY,
  KPSubstract = GLFW_KEY_KP_SUBTRACT,
  KPAdd = GLFW_KEY_KP_ADD,
  KPEnter = GLFW_KEY_KP_ENTER,
  KPEqual = GLFW_KEY_KP_EQUAL,
  LeftShift = GLFW_KEY_LEFT_SHIFT,
  Shift = GLFW_KEY_LEFT_SHIFT,
  LeftControl = GLFW_KEY_LEFT_CONTROL,
  Control = GLFW_KEY_LEFT_CONTROL,
  LeftAlt = GLFW_KEY_LEFT_ALT,
  Alt = GLFW_KEY_LEFT_ALT,
  LeftSuper = GLFW_KEY_LEFT_SUPER,
  Super = GLFW_KEY_LEFT_SUPER,
  RightShift = GLFW_KEY_RIGHT_SHIFT,
  RightControl = GLFW_KEY_RIGHT_CONTROL,
  RightAlt = GLFW_KEY_RIGHT_ALT,
  RightSuper = GLFW_KEY_RIGHT_SUPER,
  Menu = GLFW_KEY_MENU,
  Last = GLFW_KEY_LAST
};

enum class ButtonState : uint8_t {
  Released = 0b000,
  // Invalid = 0b001,
  JustPressedAndJustReleased = 0b010,
  JustPressed = 0b011,
  JustReleased = 0b100,
  Pressed = 0b101,
  JustReleasedAndPressedAndRelease = 0b110,
  JustReleasedAndPressed = 0b111,
};

typedef struct keyPressed_s {
  KeyCode keycode;
} keyPressed;

typedef struct keyReleased_s {
  KeyCode keycode;
} keyReleased;

template <typename InputType, std::size_t N = 0, typename std::enable_if<std::is_enum<InputType>::value>::type* = nullptr>
class ButtonInput {
  public:
  void press(InputType button) {
    this->pressed.insert(button);
    this->just_pressed.insert(button);
  }

  void release(InputType button) {
    this->pressed.erase(button);
    this->just_released.insert(button);
  }

  bool is_pressed(InputType button) {
    return this->pressed.count(button);
  }

  bool is_released(InputType button) {
    return !this->pressed.count(button);
  }

  bool is_just_pressed(InputType button) {
    return this->just_pressed.count(button);
  }

  bool is_just_released(InputType button) {
    return this->just_released.count(button);
  }

  void clear() {
    just_pressed.clear();
    just_released.clear();
  }

  protected:
  /// A collection of every button that has been pressed.
  std::unordered_set<InputType> pressed;
  /// A collection of every button that has just been pressed.
  std::unordered_set<InputType> just_pressed;
  /// A collection of every button that has just been released.
  std::unordered_set<InputType> just_released;
};

// Not Pressed, Just Pressed Pressed, Just Released
}};
