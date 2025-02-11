/*
Copyright (C) 2018 Andre Leiradella

This file is part of RALibretro.

RALibretro is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

RALibretro is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RALibretro.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "KeyBinds.h"

#include "Util.h"

#include "components/Dialog.h"
#include "components/Input.h"

#include <jsonsax/jsonsax.h>

#include <SDL_hints.h>
#include <SDL_keycode.h>

#define MODIFIERS (KMOD_MODE | KMOD_CTRL | KMOD_SHIFT | KMOD_ALT | KMOD_GUI)

enum
{
  // Joypad buttons
  kJoy0Up,
  kJoy0Down,
  kJoy0Left,
  kJoy0Right,
  kJoy0X,
  kJoy0Y,
  kJoy0A,
  kJoy0B,
  kJoy0L,
  kJoy0R,
  kJoy0L2,
  kJoy0R2,
  kJoy0L3,
  kJoy0R3,
  kJoy0Select,
  kJoy0Start,
  kJoy0LeftAnalogLeft,
  kJoy0LeftAnalogRight,
  kJoy0LeftAnalogUp,
  kJoy0LeftAnalogDown,
  kJoy0RightAnalogLeft,
  kJoy0RightAnalogRight,
  kJoy0RightAnalogUp,
  kJoy0RightAnalogDown,

  kJoy1Up,
  kJoy1Down,
  kJoy1Left,
  kJoy1Right,
  kJoy1X,
  kJoy1Y,
  kJoy1A,
  kJoy1B,
  kJoy1L,
  kJoy1R,
  kJoy1L2,
  kJoy1R2,
  kJoy1L3,
  kJoy1R3,
  kJoy1Select,
  kJoy1Start,
  kJoy1LeftAnalogLeft,
  kJoy1LeftAnalogRight,
  kJoy1LeftAnalogUp,
  kJoy1LeftAnalogDown,
  kJoy1RightAnalogLeft,
  kJoy1RightAnalogRight,
  kJoy1RightAnalogUp,
  kJoy1RightAnalogDown,

  // State management
  kSaveState1,
  kSaveState2,
  kSaveState3,
  kSaveState4,
  kSaveState5,
  kSaveState6,
  kSaveState7,
  kSaveState8,
  kSaveState9,
  kSaveState10,
  kLoadState1,
  kLoadState2,
  kLoadState3,
  kLoadState4,
  kLoadState5,
  kLoadState6,
  kLoadState7,
  kLoadState8,
  kLoadState9,
  kLoadState10,
  kPreviousSlot,
  kNextSlot,
  kSetSlot1,
  kSetSlot2,
  kSetSlot3,
  kSetSlot4,
  kSetSlot5,
  kSetSlot6,
  kSetSlot7,
  kSetSlot8,
  kSetSlot9,
  kSetSlot10,
  kLoadCurrent,
  kSaveCurrent,

  // Window size
  kSetWindowSize1,
  kSetWindowSize2,
  kSetWindowSize3,
  kSetWindowSize4,
  kSetWindowSize5,
  kToggleFullscreen,
  kRotateRight,
  kRotateLeft,

  // Emulation speed
  kPauseToggle,
  kPauseToggleNoOvl,
  kFastForward,
  kFastForwardToggle,
  kStep,

  // Screenshot
  kScreenshot,

  // Reset
  kReset,

  // Game Focus
  kGameFocusToggle,

  kMaxBindings
};

static const char* bindingNames[] = {
  "J0_UP", "J0_DOWN", "J0_LEFT", "J0_RIGHT", "J0_X", "J0_Y", "J0_A", "J0_B",
  "J0_L", "J0_R", "J0_L2", "J0_R2", "J0_L3", "J0_R3", "J0_SELECT", "J0_START",
  "J0_LSTICK_LEFT", "J0_LSTICK_RIGHT", "J0_LSTICK_UP", "J0_LSTICK_DOWN",
  "J0_RSTICK_LEFT", "J0_RSTICK_RIGHT", "J0_RSTICK_UP", "J0_RSTICK_DOWN",

  "J1_UP", "J1_DOWN", "J1_LEFT", "J1_RIGHT", "J1_X", "J1_Y", "J1_A", "J1_B",
  "J1_L", "J1_R", "J1_L2", "J1_R2", "J1_L3", "J1_R3", "J1_SELECT", "J1_START",
  "J1_LSTICK_LEFT", "J1_LSTICK_RIGHT", "J1_LSTICK_UP", "J1_LSTICK_DOWN",
  "J1_RSTICK_LEFT", "J1_RSTICK_RIGHT", "J1_RSTICK_UP", "J1_RSTICK_DOWN",

  "SAVE1", "SAVE2", "SAVE3", "SAVE4", "SAVE5", "SAVE6", "SAVE7", "SAVE8", "SAVE9", "SAVE0",
  "LOAD1", "LOAD2", "LOAD3", "LOAD4", "LOAD5", "LOAD6", "LOAD7", "LOAD8", "LOAD9", "LOAD0",
  "NEXT_SLOT", "PREV_SLOT",
  "SLOT1", "SLOT2", "SLOT3", "SLOT4", "SLOT5", "SLOT6", "SLOT7", "SLOT8", "SLOT9", "SLOT0",
  "LOAD_SLOT", "SAVE_SLOT",

  "WINDOW_1X", "WINDOW_2X", "WINDOW_3X", "WINDOW_4X", "WINDOW_5X",
  "TOGGLE_FULLSCREEN", "ROTATE_RIGHT", "ROTATE_LEFT",

  "SHOW_OVERLAY", "PAUSE", "FAST_FORWARD", "FAST_FORWARD_TOGGLE", "FRAME_ADVANCE",

  "SCREENSHOT",

  "RESET",

  "GAME_FOCUS_TOGGLE"
};
static_assert(sizeof(bindingNames) / sizeof(bindingNames[0]) == kMaxBindings, "bindingNames does not contain an appropriate number of elements");

bool KeyBinds::init(Logger* logger)
{
  static_assert(sizeof(_bindings) / sizeof(_bindings[0]) == kMaxBindings, "BindingList does not contain an appropriate number of elements");

  _logger = logger;
  _slot = 1;
  _gameFocus = false;
  _axesHeld = 0;

  if (SDL_NumJoysticks() > 0)
  {
    _bindings[kJoy0Up] = { 0, SDL_CONTROLLER_BUTTON_DPAD_UP, Binding::Type::Button, 0 };
    _bindings[kJoy0Down] = { 0, SDL_CONTROLLER_BUTTON_DPAD_DOWN, Binding::Type::Button, 0 };
    _bindings[kJoy0Left] = { 0, SDL_CONTROLLER_BUTTON_DPAD_LEFT, Binding::Type::Button, 0 };
    _bindings[kJoy0Right] = { 0, SDL_CONTROLLER_BUTTON_DPAD_RIGHT, Binding::Type::Button, 0 };
    _bindings[kJoy0X] = { 0, SDL_CONTROLLER_BUTTON_Y, Binding::Type::Button, 0 };
    _bindings[kJoy0Y] = { 0, SDL_CONTROLLER_BUTTON_X, Binding::Type::Button, 0 };
    _bindings[kJoy0A] = { 0, SDL_CONTROLLER_BUTTON_B, Binding::Type::Button, 0 };
    _bindings[kJoy0B] = { 0, SDL_CONTROLLER_BUTTON_A, Binding::Type::Button, 0 };
    _bindings[kJoy0L] = { 0, SDL_CONTROLLER_BUTTON_LEFTSHOULDER, Binding::Type::Button, 0 };
    _bindings[kJoy0R] = { 0, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, Binding::Type::Button, 0 };
    _bindings[kJoy0L2] = { 0, SDL_CONTROLLER_AXIS_TRIGGERLEFT, Binding::Type::Axis, 0 };
    _bindings[kJoy0R2] = { 0, SDL_CONTROLLER_AXIS_TRIGGERRIGHT, Binding::Type::Axis, 0 };
    _bindings[kJoy0L3] = { 0, SDL_CONTROLLER_BUTTON_LEFTSTICK, Binding::Type::Button, 0 };
    _bindings[kJoy0R3] = { 0, SDL_CONTROLLER_BUTTON_RIGHTSTICK, Binding::Type::Button, 0 };
    _bindings[kJoy0LeftAnalogLeft] = { 0, SDL_CONTROLLER_AXIS_LEFTX, Binding::Type::Axis, 0xFF };
    _bindings[kJoy0LeftAnalogRight] = { 0, SDL_CONTROLLER_AXIS_LEFTX, Binding::Type::Axis, 1 };
    _bindings[kJoy0LeftAnalogUp] = { 0, SDL_CONTROLLER_AXIS_LEFTY, Binding::Type::Axis, 0xFF };
    _bindings[kJoy0LeftAnalogDown] = { 0, SDL_CONTROLLER_AXIS_LEFTY, Binding::Type::Axis, 1 };
    _bindings[kJoy0RightAnalogLeft] = { 0, SDL_CONTROLLER_AXIS_RIGHTX, Binding::Type::Axis, 0xFF };
    _bindings[kJoy0RightAnalogRight] = { 0, SDL_CONTROLLER_AXIS_RIGHTX, Binding::Type::Axis, 1 };
    _bindings[kJoy0RightAnalogUp] = { 0, SDL_CONTROLLER_AXIS_RIGHTY, Binding::Type::Axis, 0xFF };
    _bindings[kJoy0RightAnalogDown] = { 0, SDL_CONTROLLER_AXIS_RIGHTY, Binding::Type::Axis, 1 };
    _bindings[kJoy0Select] = { 0, SDL_CONTROLLER_BUTTON_BACK, Binding::Type::Button, 0 };
    _bindings[kJoy0Start] = { 0, SDL_CONTROLLER_BUTTON_START, Binding::Type::Button, 0 };
  }
  else
  {
    _bindings[kJoy0Up] = { 0, SDLK_UP, Binding::Type::Key, 0 };
    _bindings[kJoy0Down] = { 0, SDLK_DOWN, Binding::Type::Key, 0 };
    _bindings[kJoy0Left] = { 0, SDLK_LEFT, Binding::Type::Key, 0 };
    _bindings[kJoy0Right] = { 0, SDLK_RIGHT, Binding::Type::Key, 0 };
    _bindings[kJoy0X] = { 0, SDLK_s, Binding::Type::Key, 0 };
    _bindings[kJoy0Y] = { 0, SDLK_a, Binding::Type::Key, 0 };
    _bindings[kJoy0A] = { 0, SDLK_x, Binding::Type::Key, 0 };
    _bindings[kJoy0B] = { 0, SDLK_z, Binding::Type::Key, 0 };
    _bindings[kJoy0L] = { 0, SDLK_d, Binding::Type::Key, 0 };
    _bindings[kJoy0R] = { 0, SDLK_c, Binding::Type::Key, 0 };
    _bindings[kJoy0L2] = { 0, SDLK_f, Binding::Type::Key, 0 };
    _bindings[kJoy0R2] = { 0, SDLK_v, Binding::Type::Key, 0 };
    _bindings[kJoy0L3] = { 0, SDLK_g, Binding::Type::Key, 0 };
    _bindings[kJoy0R3] = { 0, SDLK_h, Binding::Type::Key, 0 };
    _bindings[kJoy0LeftAnalogLeft] = { 0, SDLK_j, Binding::Type::Key, 0 };
    _bindings[kJoy0LeftAnalogRight] = { 0, SDLK_l, Binding::Type::Key, 0 };
    _bindings[kJoy0LeftAnalogUp] = { 0, SDLK_i, Binding::Type::Key, 0 };
    _bindings[kJoy0LeftAnalogDown] = { 0, SDLK_k, Binding::Type::Key, 0 };
    _bindings[kJoy0RightAnalogLeft] = { 0, SDLK_DELETE, Binding::Type::Key, 0 };
    _bindings[kJoy0RightAnalogRight] = { 0, SDLK_PAGEDOWN, Binding::Type::Key, 0 };
    _bindings[kJoy0RightAnalogUp] = { 0, SDLK_HOME, Binding::Type::Key, 0 };
    _bindings[kJoy0RightAnalogDown] = { 0, SDLK_END, Binding::Type::Key, 0 };
    _bindings[kJoy0Select] = { 0, SDLK_TAB, Binding::Type::Key, 0 };
    _bindings[kJoy0Start] = { 0, SDLK_RETURN, Binding::Type::Key, 0 };
  }

  _bindings[kSaveState1] = { 0, SDLK_F1, Binding::Type::Key, KMOD_SHIFT };
  _bindings[kSaveState2] = { 0, SDLK_F2, Binding::Type::Key, KMOD_SHIFT };
  _bindings[kSaveState3] = { 0, SDLK_F3, Binding::Type::Key, KMOD_SHIFT };
  _bindings[kSaveState4] = { 0, SDLK_F4, Binding::Type::Key, KMOD_SHIFT };
  _bindings[kSaveState5] = { 0, SDLK_F5, Binding::Type::Key, KMOD_SHIFT };
  _bindings[kSaveState6] = { 0, SDLK_F6, Binding::Type::Key, KMOD_SHIFT };
  _bindings[kSaveState7] = { 0, SDLK_F7, Binding::Type::Key, KMOD_SHIFT };
  _bindings[kSaveState8] = { 0, SDLK_F8, Binding::Type::Key, KMOD_SHIFT };
  _bindings[kSaveState9] = { 0, SDLK_F9, Binding::Type::Key, KMOD_SHIFT };
  _bindings[kSaveState10] = { 0, SDLK_F10, Binding::Type::Key, KMOD_SHIFT };
  _bindings[kLoadState1] = { 0, SDLK_F1, Binding::Type::Key, 0 };
  _bindings[kLoadState2] = { 0, SDLK_F2, Binding::Type::Key, 0 };
  _bindings[kLoadState3] = { 0, SDLK_F3, Binding::Type::Key, 0 };
  _bindings[kLoadState4] = { 0, SDLK_F4, Binding::Type::Key, 0 };
  _bindings[kLoadState5] = { 0, SDLK_F5, Binding::Type::Key, 0 };
  _bindings[kLoadState6] = { 0, SDLK_F6, Binding::Type::Key, 0 };
  _bindings[kLoadState7] = { 0, SDLK_F7, Binding::Type::Key, 0 };
  _bindings[kLoadState8] = { 0, SDLK_F8, Binding::Type::Key, 0 };
  _bindings[kLoadState9] = { 0, SDLK_F9, Binding::Type::Key, 0 };
  _bindings[kLoadState10] = { 0, SDLK_F10, Binding::Type::Key, 0 };
  _bindings[kPreviousSlot] = { 0, SDLK_MINUS, Binding::Type::Key, KMOD_SHIFT };
  _bindings[kNextSlot] = { 0, SDLK_EQUALS, Binding::Type::Key, KMOD_SHIFT };
  _bindings[kSetSlot1] = { 0, SDLK_1, Binding::Type::Key, 0 };
  _bindings[kSetSlot2] = { 0, SDLK_2, Binding::Type::Key, 0 };
  _bindings[kSetSlot3] = { 0, SDLK_3, Binding::Type::Key, 0 };
  _bindings[kSetSlot4] = { 0, SDLK_4, Binding::Type::Key, 0 };
  _bindings[kSetSlot5] = { 0, SDLK_5, Binding::Type::Key, 0 };
  _bindings[kSetSlot6] = { 0, SDLK_6, Binding::Type::Key, 0 };
  _bindings[kSetSlot7] = { 0, SDLK_7, Binding::Type::Key, 0 };
  _bindings[kSetSlot8] = { 0, SDLK_8, Binding::Type::Key, 0 };
  _bindings[kSetSlot9] = { 0, SDLK_9, Binding::Type::Key, 0 };
  _bindings[kSetSlot10] = { 0, SDLK_0, Binding::Type::Key, 0 };
  _bindings[kLoadCurrent] = { 0, SDLK_F11, Binding::Type::Key, 0 };
  _bindings[kSaveCurrent] = { 0, SDLK_F12, Binding::Type::Key, 0 };

  _bindings[kSetWindowSize1] = { 0, SDLK_1, Binding::Type::Key, KMOD_ALT };
  _bindings[kSetWindowSize2] = { 0, SDLK_2, Binding::Type::Key, KMOD_ALT };
  _bindings[kSetWindowSize3] = { 0, SDLK_3, Binding::Type::Key, KMOD_ALT };
  _bindings[kSetWindowSize4] = { 0, SDLK_4, Binding::Type::Key, KMOD_ALT };
  _bindings[kSetWindowSize5] = { 0, SDLK_5, Binding::Type::Key, KMOD_ALT };
  _bindings[kToggleFullscreen] = { 0, SDLK_RETURN, Binding::Type::Key, KMOD_ALT };
  _bindings[kRotateRight] = { 0, SDLK_r, Binding::Type::Key, KMOD_CTRL };
  _bindings[kRotateLeft] = { 0, SDLK_r, Binding::Type::Key, KMOD_CTRL | KMOD_SHIFT };

  _bindings[kPauseToggle] = { 0, SDLK_ESCAPE, Binding::Type::Key, 0 };
  _bindings[kPauseToggleNoOvl] = { 0, SDLK_p, Binding::Type::Key, 0 };
  _bindings[kFastForward] = { 0, SDLK_EQUALS, Binding::Type::Key, 0 };
  _bindings[kFastForwardToggle] = { 0, SDLK_MINUS, Binding::Type::Key, 0 };
  _bindings[kStep] = { 0, SDLK_SEMICOLON, Binding::Type::Key, 0 };

  _bindings[kReset] = { 0, 0, Binding::Type::None, 0 };

  _bindings[kScreenshot] = { 0, SDLK_PRINTSCREEN, Binding::Type::Key, 0 };

  _bindings[kGameFocusToggle] = { 0, SDLK_SCROLLLOCK, Binding::Type::Key, 0 };

   // TODO: load persisted

  return true;
}

// Flags for _axesHeld
enum {
  kJ0LL = 1<<0,
  kJ0LR = 1<<1,
  kJ0LU = 1<<2,
  kJ0LD = 1<<3,
  kJ0RL = 1<<4,
  kJ0RR = 1<<5,
  kJ0RU = 1<<6,
  kJ0RD = 1<<7,
  kJ1LL = 1<<8,
  kJ1LR = 1<<9,
  kJ1LU = 1<<10,
  kJ1LD = 1<<11,
  kJ1RL = 1<<12,
  kJ1RR = 1<<13,
  kJ1RU = 1<<14,
  kJ1RD = 1<<15,
  kNegative = 0x5555,
};

#define JOY_EXTRA(port, pressed) ((port << 8) | pressed)
#define AXIS_EXTRA(controller, value) ((controller << 16) | (value))
#define AXIS_EXTRA_UNSET(controller, axis) AXIS_EXTRA(controller, _axesHeld & axis ? (axis & kNegative ? 0x8001 : 0x7FFF) : 0)

KeyBinds::Action KeyBinds::translateButtonPress(int button, unsigned* extra)
{
  switch (button)
  {
    // Joypad buttons
    case kJoy0Up:       *extra = JOY_EXTRA(0, 1); return Action::kButtonUp;
    case kJoy0Down:     *extra = JOY_EXTRA(0, 1); return Action::kButtonDown;
    case kJoy0Left:     *extra = JOY_EXTRA(0, 1); return Action::kButtonLeft;
    case kJoy0Right:    *extra = JOY_EXTRA(0, 1); return Action::kButtonRight;
    case kJoy0X:        *extra = JOY_EXTRA(0, 1); return Action::kButtonX;
    case kJoy0Y:        *extra = JOY_EXTRA(0, 1); return Action::kButtonY;
    case kJoy0A:        *extra = JOY_EXTRA(0, 1); return Action::kButtonA;
    case kJoy0B:        *extra = JOY_EXTRA(0, 1); return Action::kButtonB;
    case kJoy0L:        *extra = JOY_EXTRA(0, 1); return Action::kButtonL;
    case kJoy0R:        *extra = JOY_EXTRA(0, 1); return Action::kButtonR;
    case kJoy0L2:       *extra = JOY_EXTRA(0, 1); return Action::kButtonL2;
    case kJoy0R2:       *extra = JOY_EXTRA(0, 1); return Action::kButtonR2;
    case kJoy0L3:       *extra = JOY_EXTRA(0, 1); return Action::kButtonL3;
    case kJoy0R3:       *extra = JOY_EXTRA(0, 1); return Action::kButtonR3;
    case kJoy0Select:   *extra = JOY_EXTRA(0, 1); return Action::kButtonSelect;
    case kJoy0Start:    *extra = JOY_EXTRA(0, 1); return Action::kButtonStart;

    case kJoy0LeftAnalogLeft:   _axesHeld |= kJ0LL; *extra = AXIS_EXTRA(0, 0x8001); return Action::kAxisLeftX;
    case kJoy0LeftAnalogRight:  _axesHeld |= kJ0LR; *extra = AXIS_EXTRA(0, 0x7FFF); return Action::kAxisLeftX;
    case kJoy0LeftAnalogUp:     _axesHeld |= kJ0LU; *extra = AXIS_EXTRA(0, 0x8001); return Action::kAxisLeftY;
    case kJoy0LeftAnalogDown:   _axesHeld |= kJ0LD; *extra = AXIS_EXTRA(0, 0x7FFF); return Action::kAxisLeftY;
    case kJoy0RightAnalogLeft:  _axesHeld |= kJ0RL; *extra = AXIS_EXTRA(0, 0x8001); return Action::kAxisRightX;
    case kJoy0RightAnalogRight: _axesHeld |= kJ0RR; *extra = AXIS_EXTRA(0, 0x7FFF); return Action::kAxisRightX;
    case kJoy0RightAnalogUp:    _axesHeld |= kJ0RU; *extra = AXIS_EXTRA(0, 0x8001); return Action::kAxisRightY;
    case kJoy0RightAnalogDown:  _axesHeld |= kJ0RD; *extra = AXIS_EXTRA(0, 0x7FFF); return Action::kAxisRightY;

    case kJoy1Up:       *extra = JOY_EXTRA(1, 1); return Action::kButtonUp;
    case kJoy1Down:     *extra = JOY_EXTRA(1, 1); return Action::kButtonDown;
    case kJoy1Left:     *extra = JOY_EXTRA(1, 1); return Action::kButtonLeft;
    case kJoy1Right:    *extra = JOY_EXTRA(1, 1); return Action::kButtonRight;
    case kJoy1X:        *extra = JOY_EXTRA(1, 1); return Action::kButtonX;
    case kJoy1Y:        *extra = JOY_EXTRA(1, 1); return Action::kButtonY;
    case kJoy1A:        *extra = JOY_EXTRA(1, 1); return Action::kButtonA;
    case kJoy1B:        *extra = JOY_EXTRA(1, 1); return Action::kButtonB;
    case kJoy1L:        *extra = JOY_EXTRA(1, 1); return Action::kButtonL;
    case kJoy1R:        *extra = JOY_EXTRA(1, 1); return Action::kButtonR;
    case kJoy1L2:       *extra = JOY_EXTRA(1, 1); return Action::kButtonL2;
    case kJoy1R2:       *extra = JOY_EXTRA(1, 1); return Action::kButtonR2;
    case kJoy1L3:       *extra = JOY_EXTRA(1, 1); return Action::kButtonL3;
    case kJoy1R3:       *extra = JOY_EXTRA(1, 1); return Action::kButtonR3;
    case kJoy1Select:   *extra = JOY_EXTRA(1, 1); return Action::kButtonSelect;
    case kJoy1Start:    *extra = JOY_EXTRA(1, 1); return Action::kButtonStart;

    case kJoy1LeftAnalogLeft:   _axesHeld |= kJ1LL; *extra = AXIS_EXTRA(1, 0x8001); return Action::kAxisLeftX;
    case kJoy1LeftAnalogRight:  _axesHeld |= kJ1LR; *extra = AXIS_EXTRA(1, 0x7FFF); return Action::kAxisLeftX;
    case kJoy1LeftAnalogUp:     _axesHeld |= kJ1LU; *extra = AXIS_EXTRA(1, 0x8001); return Action::kAxisLeftY;
    case kJoy1LeftAnalogDown:   _axesHeld |= kJ1LD; *extra = AXIS_EXTRA(1, 0x7FFF); return Action::kAxisLeftY;
    case kJoy1RightAnalogLeft:  _axesHeld |= kJ1RL; *extra = AXIS_EXTRA(1, 0x8001); return Action::kAxisRightX;
    case kJoy1RightAnalogRight: _axesHeld |= kJ1RR; *extra = AXIS_EXTRA(1, 0x7FFF); return Action::kAxisRightX;
    case kJoy1RightAnalogUp:    _axesHeld |= kJ1RU; *extra = AXIS_EXTRA(1, 0x8001); return Action::kAxisRightY;
    case kJoy1RightAnalogDown:  _axesHeld |= kJ1RD; *extra = AXIS_EXTRA(1, 0x7FFF); return Action::kAxisRightY;

    // State state management
    case kSaveState1:   *extra = 1; return Action::kSaveState;
    case kSaveState2:   *extra = 2; return Action::kSaveState;
    case kSaveState3:   *extra = 3; return Action::kSaveState;
    case kSaveState4:   *extra = 4; return Action::kSaveState;
    case kSaveState5:   *extra = 5; return Action::kSaveState;
    case kSaveState6:   *extra = 6; return Action::kSaveState;
    case kSaveState7:   *extra = 7; return Action::kSaveState;
    case kSaveState8:   *extra = 8; return Action::kSaveState;
    case kSaveState9:   *extra = 9; return Action::kSaveState;
    case kSaveState10:  *extra = 10; return Action::kSaveState;
    case kLoadState1:   *extra = 1; return Action::kLoadState;
    case kLoadState2:   *extra = 2; return Action::kLoadState;
    case kLoadState3:   *extra = 3; return Action::kLoadState;
    case kLoadState4:   *extra = 4; return Action::kLoadState;
    case kLoadState5:   *extra = 5; return Action::kLoadState;
    case kLoadState6:   *extra = 6; return Action::kLoadState;
    case kLoadState7:   *extra = 7; return Action::kLoadState;
    case kLoadState8:   *extra = 8; return Action::kLoadState;
    case kLoadState9:   *extra = 9; return Action::kLoadState;
    case kLoadState10:  *extra = 10; return Action::kLoadState;
    case kPreviousSlot: _slot = (_slot == 1) ? 10 : _slot - 1; return Action::kNothing;
    case kNextSlot:     _slot = (_slot == 10) ? 1 : _slot + 1; return Action::kNothing;
    case kSetSlot1:     _slot = 1; return Action::kNothing;
    case kSetSlot2:     _slot = 2; return Action::kNothing;
    case kSetSlot3:     _slot = 3; return Action::kNothing;
    case kSetSlot4:     _slot = 4; return Action::kNothing;
    case kSetSlot5:     _slot = 5; return Action::kNothing;
    case kSetSlot6:     _slot = 6; return Action::kNothing;
    case kSetSlot7:     _slot = 7; return Action::kNothing;
    case kSetSlot8:     _slot = 8; return Action::kNothing;
    case kSetSlot9:     _slot = 9; return Action::kNothing;
    case kSetSlot10:    _slot = 10; return Action::kNothing;
    case kLoadCurrent:  *extra = _slot; return Action::kLoadState;
    case kSaveCurrent:  *extra = _slot; return Action::kSaveState;

    // Window size
    case kSetWindowSize1:    return Action::kSetWindowSize1;
    case kSetWindowSize2:    return Action::kSetWindowSize2;
    case kSetWindowSize3:    return Action::kSetWindowSize3;
    case kSetWindowSize4:    return Action::kSetWindowSize4;
    case kSetWindowSize5:    return Action::kSetWindowSize5;
    case kToggleFullscreen:  return Action::kToggleFullscreen;
    case kRotateRight:       return Action::kRotateRight;
    case kRotateLeft:        return Action::kRotateLeft;

    // Emulation speed
    case kPauseToggleNoOvl:  return Action::kPauseToggleNoOvl;
    case kPauseToggle:       return Action::kPauseToggle;
    case kFastForward:       *extra = 1; return Action::kFastForward;
    case kFastForwardToggle: *extra = 2; return Action::kFastForward;
    case kStep:              return Action::kStep;

    // Reset
    case kReset:             return Action::kReset;

    // Screenshot
    case kScreenshot:        return Action::kScreenshot;

    // Game Focus
    case kGameFocusToggle:   _gameFocus = !_gameFocus; return Action::kGameFocusToggle;

    default:                 return Action::kNothing;
  }
}

KeyBinds::Action KeyBinds::translateButtonReleased(int button, unsigned* extra)
{
  switch (button)
  {
    // Joypad buttons
    case kJoy0Up:       *extra = JOY_EXTRA(0, 0); return Action::kButtonUp;
    case kJoy0Down:     *extra = JOY_EXTRA(0, 0); return Action::kButtonDown;
    case kJoy0Left:     *extra = JOY_EXTRA(0, 0); return Action::kButtonLeft;
    case kJoy0Right:    *extra = JOY_EXTRA(0, 0); return Action::kButtonRight;
    case kJoy0X:        *extra = JOY_EXTRA(0, 0); return Action::kButtonX;
    case kJoy0Y:        *extra = JOY_EXTRA(0, 0); return Action::kButtonY;
    case kJoy0A:        *extra = JOY_EXTRA(0, 0); return Action::kButtonA;
    case kJoy0B:        *extra = JOY_EXTRA(0, 0); return Action::kButtonB;
    case kJoy0L:        *extra = JOY_EXTRA(0, 0); return Action::kButtonL;
    case kJoy0R:        *extra = JOY_EXTRA(0, 0); return Action::kButtonR;
    case kJoy0L2:       *extra = JOY_EXTRA(0, 0); return Action::kButtonL2;
    case kJoy0R2:       *extra = JOY_EXTRA(0, 0); return Action::kButtonR2;
    case kJoy0L3:       *extra = JOY_EXTRA(0, 0); return Action::kButtonL3;
    case kJoy0R3:       *extra = JOY_EXTRA(0, 0); return Action::kButtonR3;
    case kJoy0Select:   *extra = JOY_EXTRA(0, 0); return Action::kButtonSelect;
    case kJoy0Start:    *extra = JOY_EXTRA(0, 0); return Action::kButtonStart;

    case kJoy0LeftAnalogLeft:   _axesHeld &= ~kJ0LL; *extra = AXIS_EXTRA_UNSET(0, kJ0LR); return Action::kAxisLeftX;
    case kJoy0LeftAnalogRight:  _axesHeld &= ~kJ0LR; *extra = AXIS_EXTRA_UNSET(0, kJ0LL); return Action::kAxisLeftX;
    case kJoy0LeftAnalogUp:     _axesHeld &= ~kJ0LU; *extra = AXIS_EXTRA_UNSET(0, kJ0LD); return Action::kAxisLeftY;
    case kJoy0LeftAnalogDown:   _axesHeld &= ~kJ0LD; *extra = AXIS_EXTRA_UNSET(0, kJ0LU); return Action::kAxisLeftY;
    case kJoy0RightAnalogLeft:  _axesHeld &= ~kJ0RL; *extra = AXIS_EXTRA_UNSET(0, kJ0RR); return Action::kAxisRightX;
    case kJoy0RightAnalogRight: _axesHeld &= ~kJ0RR; *extra = AXIS_EXTRA_UNSET(0, kJ0RL); return Action::kAxisRightX;
    case kJoy0RightAnalogUp:    _axesHeld &= ~kJ0RU; *extra = AXIS_EXTRA_UNSET(0, kJ0RD); return Action::kAxisRightY;
    case kJoy0RightAnalogDown:  _axesHeld &= ~kJ0RD; *extra = AXIS_EXTRA_UNSET(0, kJ0RU); return Action::kAxisRightY;

    case kJoy1Up:       *extra = JOY_EXTRA(1, 0); return Action::kButtonUp;
    case kJoy1Down:     *extra = JOY_EXTRA(1, 0); return Action::kButtonDown;
    case kJoy1Left:     *extra = JOY_EXTRA(1, 0); return Action::kButtonLeft;
    case kJoy1Right:    *extra = JOY_EXTRA(1, 0); return Action::kButtonRight;
    case kJoy1X:        *extra = JOY_EXTRA(1, 0); return Action::kButtonX;
    case kJoy1Y:        *extra = JOY_EXTRA(1, 0); return Action::kButtonY;
    case kJoy1A:        *extra = JOY_EXTRA(1, 0); return Action::kButtonA;
    case kJoy1B:        *extra = JOY_EXTRA(1, 0); return Action::kButtonB;
    case kJoy1L:        *extra = JOY_EXTRA(1, 0); return Action::kButtonL;
    case kJoy1R:        *extra = JOY_EXTRA(1, 0); return Action::kButtonR;
    case kJoy1L2:       *extra = JOY_EXTRA(1, 0); return Action::kButtonL2;
    case kJoy1R2:       *extra = JOY_EXTRA(1, 0); return Action::kButtonR2;
    case kJoy1L3:       *extra = JOY_EXTRA(1, 0); return Action::kButtonL3;
    case kJoy1R3:       *extra = JOY_EXTRA(1, 0); return Action::kButtonR3;
    case kJoy1Select:   *extra = JOY_EXTRA(1, 0); return Action::kButtonSelect;
    case kJoy1Start:    *extra = JOY_EXTRA(1, 0); return Action::kButtonStart;

    case kJoy1LeftAnalogLeft:   _axesHeld &= ~kJ1LL; *extra = AXIS_EXTRA_UNSET(1, kJ1LR); return Action::kAxisLeftX;
    case kJoy1LeftAnalogRight:  _axesHeld &= ~kJ1LR; *extra = AXIS_EXTRA_UNSET(1, kJ1LL); return Action::kAxisLeftX;
    case kJoy1LeftAnalogUp:     _axesHeld &= ~kJ1LU; *extra = AXIS_EXTRA_UNSET(1, kJ1LD); return Action::kAxisLeftY;
    case kJoy1LeftAnalogDown:   _axesHeld &= ~kJ1LD; *extra = AXIS_EXTRA_UNSET(1, kJ1LU); return Action::kAxisLeftY;
    case kJoy1RightAnalogLeft:  _axesHeld &= ~kJ1RL; *extra = AXIS_EXTRA_UNSET(1, kJ1RR); return Action::kAxisRightX;
    case kJoy1RightAnalogRight: _axesHeld &= ~kJ1RR; *extra = AXIS_EXTRA_UNSET(1, kJ1RL); return Action::kAxisRightX;
    case kJoy1RightAnalogUp:    _axesHeld &= ~kJ1RU; *extra = AXIS_EXTRA_UNSET(1, kJ1RD); return Action::kAxisRightY;
    case kJoy1RightAnalogDown:  _axesHeld &= ~kJ1RD; *extra = AXIS_EXTRA_UNSET(1, kJ1RU); return Action::kAxisRightY;

    // Emulation speed
    case kFastForward:  *extra = 0; return Action::kFastForward;

    default: return Action::kNothing;
  }
}

KeyBinds::Action KeyBinds::translateAnalog(int button, Sint16 value, unsigned* extra)
{
  KeyBinds::Action action;
  unsigned controller;

  switch (button)
  {
    case kJoy0LeftAnalogLeft:
    case kJoy0LeftAnalogRight:  action = Action::kAxisLeftX;  controller = 0; break;
    case kJoy0LeftAnalogUp:
    case kJoy0LeftAnalogDown:   action = Action::kAxisLeftY;  controller = 0; break;
    case kJoy0RightAnalogLeft:
    case kJoy0RightAnalogRight: action = Action::kAxisRightX; controller = 0; break;
    case kJoy0RightAnalogUp:
    case kJoy0RightAnalogDown:  action = Action::kAxisRightY; controller = 0; break;
    case kJoy1LeftAnalogLeft:
    case kJoy1LeftAnalogRight:  action = Action::kAxisLeftX;  controller = 1; break;
    case kJoy1LeftAnalogUp:
    case kJoy1LeftAnalogDown:   action = Action::kAxisLeftY;  controller = 1; break;
    case kJoy1RightAnalogLeft:
    case kJoy1RightAnalogRight: action = Action::kAxisRightX; controller = 1; break;
    case kJoy1RightAnalogUp:
    case kJoy1RightAnalogDown:  action = Action::kAxisRightY; controller = 1; break;

    default:
      return Action::kNothing;
  }

  /* if we pass -32768, it sometimes causes an overflow and acts like a positive value */
  if (value == -32768)
    value = -32767;

  *extra = AXIS_EXTRA(controller, ((unsigned)value) & 0xFFFF);
  return action;
}

KeyBinds::Action KeyBinds::translateKeyboardInput(SDL_Keycode kcode, bool pressed, unsigned* extra) const
{
  enum retro_key rkey = RETROK_UNKNOWN;
  switch (kcode)
  {
    case SDLK_BACKSPACE:    rkey = RETROK_BACKSPACE; break;
    case SDLK_TAB:          rkey = RETROK_TAB; break;
    case SDLK_CLEAR:        rkey = RETROK_CLEAR; break;
    case SDLK_RETURN:       rkey = RETROK_RETURN; break;
    case SDLK_PAUSE:        rkey = RETROK_PAUSE; break;
    case SDLK_ESCAPE:       rkey = RETROK_ESCAPE; break;
    case SDLK_SPACE:        rkey = RETROK_SPACE; break;
    case SDLK_EXCLAIM:      rkey = RETROK_EXCLAIM; break;
    case SDLK_QUOTEDBL:     rkey = RETROK_QUOTEDBL; break;
    case SDLK_HASH:         rkey = RETROK_HASH; break;
    case SDLK_DOLLAR:       rkey = RETROK_DOLLAR; break;
    case SDLK_AMPERSAND:    rkey = RETROK_AMPERSAND; break;
    case SDLK_QUOTE:        rkey = RETROK_QUOTE; break;
    case SDLK_LEFTPAREN:    rkey = RETROK_LEFTPAREN; break;
    case SDLK_RIGHTPAREN:   rkey = RETROK_RIGHTPAREN; break;
    case SDLK_ASTERISK:     rkey = RETROK_ASTERISK; break;
    case SDLK_PLUS:         rkey = RETROK_PLUS; break;
    case SDLK_COMMA:        rkey = RETROK_COMMA; break;
    case SDLK_MINUS:        rkey = RETROK_MINUS; break;
    case SDLK_PERIOD:       rkey = RETROK_PERIOD; break;
    case SDLK_SLASH:        rkey = RETROK_SLASH; break;
    case SDLK_0:            rkey = RETROK_0; break;
    case SDLK_1:            rkey = RETROK_1; break;
    case SDLK_2:            rkey = RETROK_2; break;
    case SDLK_3:            rkey = RETROK_3; break;
    case SDLK_4:            rkey = RETROK_4; break;
    case SDLK_5:            rkey = RETROK_5; break;
    case SDLK_6:            rkey = RETROK_6; break;
    case SDLK_7:            rkey = RETROK_7; break;
    case SDLK_8:            rkey = RETROK_8; break;
    case SDLK_9:            rkey = RETROK_9; break;
    case SDLK_COLON:        rkey = RETROK_COLON; break;
    case SDLK_SEMICOLON:    rkey = RETROK_SEMICOLON; break;
    case SDLK_LESS:         rkey = RETROK_OEM_102; break;
    case SDLK_EQUALS:       rkey = RETROK_EQUALS; break;
    case SDLK_GREATER:      rkey = RETROK_GREATER; break;
    case SDLK_QUESTION:     rkey = RETROK_QUESTION; break;
    case SDLK_AT:           rkey = RETROK_AT; break;
    case SDLK_LEFTBRACKET:  rkey = RETROK_LEFTBRACKET; break;
    case SDLK_BACKSLASH:    rkey = RETROK_BACKSLASH; break;
    case SDLK_RIGHTBRACKET: rkey = RETROK_RIGHTBRACKET; break;
    case SDLK_CARET:        rkey = RETROK_CARET; break;
    case SDLK_UNDERSCORE:   rkey = RETROK_UNDERSCORE; break;
    case SDLK_BACKQUOTE:    rkey = RETROK_BACKQUOTE; break;
    case SDLK_a:            rkey = RETROK_a; break;
    case SDLK_b:            rkey = RETROK_b; break;
    case SDLK_c:            rkey = RETROK_c; break;
    case SDLK_d:            rkey = RETROK_d; break;
    case SDLK_e:            rkey = RETROK_e; break;
    case SDLK_f:            rkey = RETROK_f; break;
    case SDLK_g:            rkey = RETROK_g; break;
    case SDLK_h:            rkey = RETROK_h; break;
    case SDLK_i:            rkey = RETROK_i; break;
    case SDLK_j:            rkey = RETROK_j; break;
    case SDLK_k:            rkey = RETROK_k; break;
    case SDLK_l:            rkey = RETROK_l; break;
    case SDLK_m:            rkey = RETROK_m; break;
    case SDLK_n:            rkey = RETROK_n; break;
    case SDLK_o:            rkey = RETROK_o; break;
    case SDLK_p:            rkey = RETROK_p; break;
    case SDLK_q:            rkey = RETROK_q; break;
    case SDLK_r:            rkey = RETROK_r; break;
    case SDLK_s:            rkey = RETROK_s; break;
    case SDLK_t:            rkey = RETROK_t; break;
    case SDLK_u:            rkey = RETROK_u; break;
    case SDLK_v:            rkey = RETROK_v; break;
    case SDLK_w:            rkey = RETROK_w; break;
    case SDLK_x:            rkey = RETROK_x; break;
    case SDLK_y:            rkey = RETROK_y; break;
    case SDLK_z:            rkey = RETROK_z; break;
    case SDLK_DELETE:       rkey = RETROK_DELETE; break;

    case SDLK_KP_0:         rkey = RETROK_KP0; break;
    case SDLK_KP_1:         rkey = RETROK_KP1; break;
    case SDLK_KP_2:         rkey = RETROK_KP2; break;
    case SDLK_KP_3:         rkey = RETROK_KP3; break;
    case SDLK_KP_4:         rkey = RETROK_KP4; break;
    case SDLK_KP_5:         rkey = RETROK_KP5; break;
    case SDLK_KP_6:         rkey = RETROK_KP6; break;
    case SDLK_KP_7:         rkey = RETROK_KP7; break;
    case SDLK_KP_8:         rkey = RETROK_KP8; break;
    case SDLK_KP_9:         rkey = RETROK_KP9; break;
    case SDLK_KP_PERIOD:    rkey = RETROK_KP_PERIOD; break;
    case SDLK_KP_DIVIDE:    rkey = RETROK_KP_DIVIDE; break;
    case SDLK_KP_MULTIPLY:  rkey = RETROK_KP_MULTIPLY; break;
    case SDLK_KP_MINUS:     rkey = RETROK_KP_MINUS; break;
    case SDLK_KP_PLUS:      rkey = RETROK_KP_PLUS; break;
    case SDLK_KP_ENTER:     rkey = RETROK_KP_ENTER; break;
    case SDLK_KP_EQUALS:    rkey = RETROK_KP_EQUALS; break;

    case SDLK_UP:           rkey = RETROK_UP; break;
    case SDLK_DOWN:         rkey = RETROK_DOWN; break;
    case SDLK_RIGHT:        rkey = RETROK_RIGHT; break;
    case SDLK_LEFT:         rkey = RETROK_LEFT; break;
    case SDLK_INSERT:       rkey = RETROK_INSERT; break;
    case SDLK_HOME:         rkey = RETROK_HOME; break;
    case SDLK_END:          rkey = RETROK_END; break;
    case SDLK_PAGEUP:       rkey = RETROK_PAGEUP; break;
    case SDLK_PAGEDOWN:     rkey = RETROK_PAGEDOWN; break;

    case SDLK_F1:           rkey = RETROK_F1; break;
    case SDLK_F2:           rkey = RETROK_F2; break;
    case SDLK_F3:           rkey = RETROK_F3; break;
    case SDLK_F4:           rkey = RETROK_F4; break;
    case SDLK_F5:           rkey = RETROK_F5; break;
    case SDLK_F6:           rkey = RETROK_F6; break;
    case SDLK_F7:           rkey = RETROK_F7; break;
    case SDLK_F8:           rkey = RETROK_F8; break;
    case SDLK_F9:           rkey = RETROK_F9; break;
    case SDLK_F10:          rkey = RETROK_F10; break;
    case SDLK_F11:          rkey = RETROK_F11; break;
    case SDLK_F12:          rkey = RETROK_F12; break;
    case SDLK_F13:          rkey = RETROK_F13; break;
    case SDLK_F14:          rkey = RETROK_F14; break;
    case SDLK_F15:          rkey = RETROK_F15; break;

    case SDLK_NUMLOCKCLEAR: rkey = RETROK_NUMLOCK; break;
    case SDLK_CAPSLOCK:     rkey = RETROK_CAPSLOCK; break;
    case SDLK_SCROLLLOCK:   rkey = RETROK_SCROLLOCK; break;
    case SDLK_RSHIFT:       rkey = RETROK_RSHIFT; break;
    case SDLK_LSHIFT:       rkey = RETROK_LSHIFT; break;
    case SDLK_RCTRL:        rkey = RETROK_RCTRL; break;
    case SDLK_LCTRL:        rkey = RETROK_LCTRL; break;
    case SDLK_RALT:         rkey = RETROK_RALT; break;
    case SDLK_LALT:         rkey = RETROK_LALT; break;
    // case ?:              rkey = RETROK_RMETA; break;
    // case ?:              rkey = RETROK_LMETA; break;
    case SDLK_LGUI:         rkey = RETROK_LSUPER; break;
    case SDLK_RGUI:         rkey = RETROK_RSUPER; break;
    case SDLK_MODE:         rkey = RETROK_MODE; break;
    // case SDLK_COMPOSE:   rkey = RETROK_COMPOSE; break;

    case SDLK_HELP:         rkey = RETROK_HELP; break;
    case SDLK_PRINTSCREEN:  rkey = RETROK_PRINT; break;
    case SDLK_SYSREQ:       rkey = RETROK_SYSREQ; break;
    // case SDLK_BREAK:     rkey = RETROK_BREAK; break;
    case SDLK_MENU:         rkey = RETROK_MENU; break;
    case SDLK_POWER:        rkey = RETROK_POWER; break;
    // case SDLK_EURO:      rkey = RETROK_EURO; break;
    case SDLK_UNDO:         rkey = RETROK_UNDO; break;
  };

  *extra = static_cast<unsigned>((rkey << 8) | static_cast<char>(pressed));
  return Action::kKeyboardInput;
}

KeyBinds::Action KeyBinds::translate(const SDL_KeyboardEvent* key, unsigned* extra)
{
  if (key->repeat)
  {
    return Action::kNothing;
  }

  // Accept both left and right modifiers
  Uint16 mod = key->keysym.mod & MODIFIERS;
  if (mod & KMOD_SHIFT) mod |= KMOD_SHIFT;
  if (mod & KMOD_CTRL)  mod |= KMOD_CTRL;
  if (mod & KMOD_ALT)   mod |= KMOD_ALT;
  if (mod & KMOD_GUI)   mod |= KMOD_GUI;

  if (_gameFocus)
  {
    // in game focus mode all keyboard-bound hotkeys are disabled
    // with the exception of the game focus toggle itself
    if (_bindings[kGameFocusToggle].type == Binding::Type::Key
      && (uint32_t)key->keysym.sym == _bindings[kGameFocusToggle].button
      && mod == _bindings[kGameFocusToggle].modifiers)
    {
      if (key->state == SDL_PRESSED)
        return translateButtonPress(kGameFocusToggle, extra);

      return Action::kNothing;
    }
  }
  else
  {
    for (size_t i = 0; i < kMaxBindings; i++)
    {
      if (_bindings[i].type == Binding::Type::Key)
      {
        if ((uint32_t)key->keysym.sym == _bindings[i].button && mod == _bindings[i].modifiers)
        {
          if (key->state == SDL_PRESSED)
            return translateButtonPress(i, extra);

          if (key->state == SDL_RELEASED)
            return translateButtonReleased(i, extra);

          return Action::kNothing;
        }
      }
    }
  }

  // if the key is not consumed as an hotkey, handle it as keyboard input
  return translateKeyboardInput(key->keysym.sym, key->state == SDL_PRESSED, extra);
}

KeyBinds::Action KeyBinds::translate(const SDL_ControllerButtonEvent* cbutton, unsigned* extra)
{
  SDL_JoystickID bindingID = getBindingID(cbutton->which);

  for (size_t i = 0; i < kMaxBindings; i++)
  {
    if (_bindings[i].type == Binding::Type::Button)
    {
      if (cbutton->button == _bindings[i].button && bindingID == _bindings[i].joystick_id)
      {
        if (cbutton->state == SDL_PRESSED)
          return translateButtonPress(i, extra);

        if (cbutton->state == SDL_RELEASED)
          return translateButtonReleased(i, extra);

        break;
      }
    }
  }

  return Action::kNothing;
}

unsigned KeyBinds::getNavigationPort(SDL_JoystickID joystickID)
{
  joystickID = getBindingID(joystickID);

  if (_bindings[kJoy0Right].type == Binding::Type::Button && _bindings[kJoy0Right].joystick_id == joystickID)
    return 0;
  if (_bindings[kJoy1Right].type == Binding::Type::Button && _bindings[kJoy1Right].joystick_id == joystickID)
    return 1;

  if (_bindings[kJoy0LeftAnalogLeft].type == Binding::Type::Axis && _bindings[kJoy0LeftAnalogLeft].joystick_id == joystickID)
    return 0;
  if (_bindings[kJoy1LeftAnalogLeft].type == Binding::Type::Axis && _bindings[kJoy1LeftAnalogLeft].joystick_id == joystickID)
    return 1;

  if (_bindings[kJoy0LeftAnalogLeft].type == Binding::Type::Button && _bindings[kJoy0LeftAnalogLeft].joystick_id == joystickID)
    return 0;
  if (_bindings[kJoy1LeftAnalogLeft].type == Binding::Type::Button && _bindings[kJoy1LeftAnalogLeft].joystick_id == joystickID)
    return 1;

  return 0xFFFFFFFF;
}

void KeyBinds::mapDevice(SDL_JoystickID originalID, SDL_JoystickID newID)
{
  for (auto pair = _bindingMap.begin(); pair != _bindingMap.end(); ++pair)
  {
    if (pair->second == originalID)
    {
      _bindingMap.erase(pair);
      break;
    }
  }

  _bindingMap.insert(std::make_pair(newID, originalID));
}

SDL_JoystickID KeyBinds::getBindingID(SDL_JoystickID id) const
{
  auto iter = _bindingMap.find(id);
  if (iter != _bindingMap.end())
    return iter->second;

  return id;
}

static bool IsAnalog(int button)
{
  switch (button)
  {
    case kJoy0LeftAnalogLeft:
    case kJoy0LeftAnalogRight:
    case kJoy0LeftAnalogUp:
    case kJoy0LeftAnalogDown:
    case kJoy0RightAnalogLeft:
    case kJoy0RightAnalogRight:
    case kJoy0RightAnalogUp:
    case kJoy0RightAnalogDown:
    case kJoy1LeftAnalogLeft:
    case kJoy1LeftAnalogRight:
    case kJoy1LeftAnalogUp:
    case kJoy1LeftAnalogDown:
    case kJoy1RightAnalogLeft:
    case kJoy1RightAnalogRight:
    case kJoy1RightAnalogUp:
    case kJoy1RightAnalogDown:
      return true;

    default:
      return false;
  }
}

void KeyBinds::translate(const SDL_ControllerAxisEvent* caxis, Input& input,
  Action* action1, unsigned* extra1, Action* action2, unsigned* extra2)
{
  SDL_JoystickID bindingID = getBindingID(caxis->which);
  *action1 = *action2 = Action::kNothing;

  int threshold = static_cast<int>(32767 * input.getJoystickSensitivity(caxis->which));
  int analogThreshold = threshold / 4;
  for (size_t i = 0; i < kMaxBindings; i++)
  {
    if (_bindings[i].type == Binding::Type::Axis)
    {
      if (caxis->axis == _bindings[i].button && bindingID == _bindings[i].joystick_id)
      {
        if (IsAnalog(i))
        {
          if (caxis->value > analogThreshold || caxis->value < -analogThreshold)
            *action1 = translateAnalog(i, caxis->value, extra1);
          else
            *action1 = translateAnalog(i, 0, extra1);

          break;
        }
        else if ((_bindings[i].modifiers & 0xFF) == 0xFF) // negative axis
        {
          if (caxis->value < -threshold)
            *action1 = translateButtonPress(i, extra1);
          else
            *action1 = translateButtonReleased(i, extra1);
        }
        else // positive axis
        {
          if (caxis->value > threshold)
            *action2 = translateButtonPress(i, extra2);
          else
            *action2 = translateButtonReleased(i, extra2);
        }
      }
    }
  }
}

void KeyBinds::getBindingString(char buffer[32], const KeyBinds::Binding& binding)
{
  switch (binding.type)
  {
    case KeyBinds::Binding::Type::None:
      snprintf(buffer, 32, "none");
      break;

    case KeyBinds::Binding::Type::Button:
      snprintf(buffer, 32, "J%d %s", binding.joystick_id, SDL_GameControllerGetStringForButton(static_cast<SDL_GameControllerButton>(binding.button)));
      break;

    case KeyBinds::Binding::Type::Axis:
      switch (binding.modifiers)
      {
        case 0xFF:
          snprintf(buffer, 32, "J%d -%s", binding.joystick_id, SDL_GameControllerGetStringForAxis(static_cast<SDL_GameControllerAxis>(binding.button)));
          break;
        case 0:
          snprintf(buffer, 32, "J%d %s", binding.joystick_id, SDL_GameControllerGetStringForAxis(static_cast<SDL_GameControllerAxis>(binding.button)));
          break;
        case 1:
          snprintf(buffer, 32, "J%d +%s", binding.joystick_id, SDL_GameControllerGetStringForAxis(static_cast<SDL_GameControllerAxis>(binding.button)));
          break;
      }
      break;

    case KeyBinds::Binding::Type::Key:
      snprintf(buffer, 32, "%s%s%s%s",
        (binding.modifiers & KMOD_CTRL) ? "Ctrl+" : "",
        (binding.modifiers & KMOD_ALT) ? "Alt+" : "",
        (binding.modifiers & KMOD_SHIFT) ? "Shift+" : "",
        SDL_GetKeyName(static_cast<SDL_Keycode>(binding.button)));
      break;
  }
}

static bool parseBindingString(const std::string& str, KeyBinds::Binding& binding)
{
  const char* ptr = str.c_str();
  int button = 0;

  if (str == "none")
  {
    binding.type = KeyBinds::Binding::Type::None;
    binding.joystick_id = 0;
    binding.button = 0;
    binding.modifiers = 0;
    return true;
  }

  if (ptr[0] == 'J' && isdigit(ptr[1]))
  {
    int joystick_id = (ptr[1] - '0');
    ptr += 3;

    if (*ptr == '-')
    {
      button = SDL_GameControllerGetAxisFromString(++ptr);
      if (button != SDL_CONTROLLER_AXIS_INVALID)
      {
        binding.type = KeyBinds::Binding::Type::Axis;
        binding.joystick_id = joystick_id;
        binding.button = button;
        binding.modifiers = 0xFF;
        return true;
      }
    }
    else if (*ptr == '+')
    {
      button = SDL_GameControllerGetAxisFromString(++ptr);
      if (button != SDL_CONTROLLER_AXIS_INVALID)
      {
        binding.type = KeyBinds::Binding::Type::Axis;
        binding.joystick_id = joystick_id;
        binding.button = button;
        binding.modifiers = 1;
        return true;
      }
    }
    else
    {
      button = SDL_GameControllerGetButtonFromString(ptr);
      if (button != SDL_CONTROLLER_BUTTON_INVALID)
      {
        binding.type = KeyBinds::Binding::Type::Button;
        binding.joystick_id = joystick_id;
        binding.button = button;
        binding.modifiers = 0;
        return true;
      }

      button = SDL_GameControllerGetAxisFromString(ptr);
      if (button != SDL_CONTROLLER_AXIS_INVALID)
      {
        binding.type = KeyBinds::Binding::Type::Axis;
        binding.joystick_id = joystick_id;
        binding.button = button;
        binding.modifiers = 0;
        return true;
      }
    }
  }
  else
  {
    int mods = 0;

    if (strncmp(ptr, "Ctrl+", 5) == 0)
    {
      mods |= KMOD_CTRL;
      ptr += 5;
    }

    if (strncmp(ptr, "Alt+", 4) == 0)
    {
      mods |= KMOD_ALT;
      ptr += 4;
    }

    if (strncmp(ptr, "Shift+", 6) == 0)
    {
      mods |= KMOD_SHIFT;
      ptr += 6;
    }

    button = SDL_GetKeyFromName(ptr);
    if (button != SDLK_UNKNOWN)
    {
      binding.type = KeyBinds::Binding::Type::Key;
      binding.joystick_id = 0;
      binding.button = button;
      binding.modifiers = mods;
      return true;
    }
  }

  return false;
}

static void remapBinding(KeyBinds::BindingList& keyBinds, const std::string& key, KeyBinds::Binding& binding)
{
  int bindIndex = (key[10] == 'Y') ? kJoy0LeftAnalogUp : kJoy0LeftAnalogLeft;
  if (key[3] == 'R')
    bindIndex += (kJoy0RightAnalogLeft - kJoy0LeftAnalogLeft);
  if (key[1] == '1')
    bindIndex += (kJoy1LeftAnalogLeft - kJoy0LeftAnalogLeft);

  keyBinds[bindIndex] = binding;
  keyBinds[bindIndex].modifiers = 0xFF;

  keyBinds[bindIndex + 1] = binding;
  keyBinds[bindIndex + 1].modifiers = 1;
}

std::string KeyBinds::serializeBindings() const
{
  char binding[32];
  std::string bindings = "{";
  bindings.reserve(kMaxBindings * 16);

  for (int i = 0; i < kMaxBindings; ++i)
  {
    if (i != 0)
      bindings += ",";

    bindings += "\"";
    bindings += bindingNames[i];
    bindings += "\":\"";

    getBindingString(binding, _bindings[i]);
    bindings += util::jsonEscape(binding);
    bindings += "\"";
  }

  bindings += "}";
  return bindings;
}

bool KeyBinds::deserializeBindings(const char* json)
{
  struct Deserialize
  {
    KeyBinds* self;
    std::string key;
  };
  Deserialize ud;
  ud.self = this;

  jsonsax_result_t res = jsonsax_parse((char*)json, &ud, [](void* udata, jsonsax_event_t event, const char* str, size_t num)
  {
    auto* ud = (Deserialize*)udata;

    if (event == JSONSAX_KEY)
    {
      ud->key = std::string(str, num);
    }
    else if (event == JSONSAX_STRING)
    {
      if (ud->key.length() == 11 && ud->key[0] == 'J' && ud->key[4] == 'S' && ud->key[5] == 'T' && ud->key[6] == 'I' && ud->key[9] == '_')
      {
        // Jn_xSTICK_d | n=0/1 x=L/R d=X/Y
        Binding binding;
        parseBindingString(util::jsonUnescape(std::string(str, num)), binding);
        remapBinding(ud->self->_bindings, ud->key, binding);
      }

      for (int i = 0; i < kMaxBindings; ++i)
      {
        if (ud->key == bindingNames[i])
        {
          parseBindingString(util::jsonUnescape(std::string(str, num)), ud->self->_bindings[i]);
        }
      }
    }

    return 0;
  });

  return (res == JSONSAX_OK);
}

// SDL defines a function to convert a virtual key to a SDL scancode, but it's not public.
// * SDL_Scancode WindowsScanCodeToSDLScanCode(LPARAM lParam, WPARAM wParam);
//
// we don't actually care about the extended keys, so we can get by just duplicating the raw keys
// from http://hg.libsdl.org/SDL/file/131ea7dcc225/src/events/scancodes_windows.h
static const SDL_Scancode windows_scancode_table[] =
{
  /*	0						1							2							3							4						5							6							7 */
  /*	8						9							A							B							C						D							E							F */
  SDL_SCANCODE_UNKNOWN,		SDL_SCANCODE_ESCAPE,		SDL_SCANCODE_1,				SDL_SCANCODE_2,				SDL_SCANCODE_3,			SDL_SCANCODE_4,				SDL_SCANCODE_5,				SDL_SCANCODE_6,			/* 0 */
  SDL_SCANCODE_7,				SDL_SCANCODE_8,				SDL_SCANCODE_9,				SDL_SCANCODE_0,				SDL_SCANCODE_MINUS,		SDL_SCANCODE_EQUALS,		SDL_SCANCODE_BACKSPACE,		SDL_SCANCODE_TAB,		/* 0 */

  SDL_SCANCODE_Q,				SDL_SCANCODE_W,				SDL_SCANCODE_E,				SDL_SCANCODE_R,				SDL_SCANCODE_T,			SDL_SCANCODE_Y,				SDL_SCANCODE_U,				SDL_SCANCODE_I,			/* 1 */
  SDL_SCANCODE_O,				SDL_SCANCODE_P,				SDL_SCANCODE_LEFTBRACKET,	SDL_SCANCODE_RIGHTBRACKET,	SDL_SCANCODE_RETURN,	SDL_SCANCODE_LCTRL,			SDL_SCANCODE_A,				SDL_SCANCODE_S,			/* 1 */

  SDL_SCANCODE_D,				SDL_SCANCODE_F,				SDL_SCANCODE_G,				SDL_SCANCODE_H,				SDL_SCANCODE_J,			SDL_SCANCODE_K,				SDL_SCANCODE_L,				SDL_SCANCODE_SEMICOLON,	/* 2 */
  SDL_SCANCODE_APOSTROPHE,	SDL_SCANCODE_GRAVE,			SDL_SCANCODE_LSHIFT,		SDL_SCANCODE_BACKSLASH,		SDL_SCANCODE_Z,			SDL_SCANCODE_X,				SDL_SCANCODE_C,				SDL_SCANCODE_V,			/* 2 */

  SDL_SCANCODE_B,				SDL_SCANCODE_N,				SDL_SCANCODE_M,				SDL_SCANCODE_COMMA,			SDL_SCANCODE_PERIOD,	SDL_SCANCODE_SLASH,			SDL_SCANCODE_RSHIFT,		SDL_SCANCODE_PRINTSCREEN,/* 3 */
  SDL_SCANCODE_LALT,			SDL_SCANCODE_SPACE,			SDL_SCANCODE_CAPSLOCK,		SDL_SCANCODE_F1,			SDL_SCANCODE_F2,		SDL_SCANCODE_F3,			SDL_SCANCODE_F4,			SDL_SCANCODE_F5,		/* 3 */

  SDL_SCANCODE_F6,			SDL_SCANCODE_F7,			SDL_SCANCODE_F8,			SDL_SCANCODE_F9,			SDL_SCANCODE_F10,		SDL_SCANCODE_NUMLOCKCLEAR,	SDL_SCANCODE_SCROLLLOCK,	SDL_SCANCODE_HOME,		/* 4 */
  SDL_SCANCODE_UP,			SDL_SCANCODE_PAGEUP,		SDL_SCANCODE_KP_MINUS,		SDL_SCANCODE_LEFT,			SDL_SCANCODE_KP_5,		SDL_SCANCODE_RIGHT,			SDL_SCANCODE_KP_PLUS,		SDL_SCANCODE_END,		/* 4 */

  SDL_SCANCODE_DOWN,			SDL_SCANCODE_PAGEDOWN,		SDL_SCANCODE_INSERT,		SDL_SCANCODE_DELETE,		SDL_SCANCODE_UNKNOWN,	SDL_SCANCODE_UNKNOWN,		SDL_SCANCODE_NONUSBACKSLASH,SDL_SCANCODE_F11,		/* 5 */
  SDL_SCANCODE_F12,			SDL_SCANCODE_PAUSE,			SDL_SCANCODE_UNKNOWN,		SDL_SCANCODE_LGUI,			SDL_SCANCODE_RGUI,		SDL_SCANCODE_APPLICATION,	SDL_SCANCODE_UNKNOWN,		SDL_SCANCODE_UNKNOWN,	/* 5 */

  SDL_SCANCODE_UNKNOWN,		SDL_SCANCODE_UNKNOWN,		SDL_SCANCODE_UNKNOWN,		SDL_SCANCODE_UNKNOWN,		SDL_SCANCODE_F13,		SDL_SCANCODE_F14,			SDL_SCANCODE_F15,			SDL_SCANCODE_F16,		/* 6 */
  SDL_SCANCODE_F17,			SDL_SCANCODE_F18,			SDL_SCANCODE_F19,			SDL_SCANCODE_UNKNOWN,		SDL_SCANCODE_UNKNOWN,	SDL_SCANCODE_UNKNOWN,		SDL_SCANCODE_UNKNOWN,		SDL_SCANCODE_UNKNOWN,	/* 6 */

  SDL_SCANCODE_INTERNATIONAL2,		SDL_SCANCODE_UNKNOWN,		SDL_SCANCODE_UNKNOWN,		SDL_SCANCODE_INTERNATIONAL1,		SDL_SCANCODE_UNKNOWN,	SDL_SCANCODE_UNKNOWN,		SDL_SCANCODE_UNKNOWN,		SDL_SCANCODE_UNKNOWN,	/* 7 */
  SDL_SCANCODE_UNKNOWN,		SDL_SCANCODE_INTERNATIONAL4,		SDL_SCANCODE_UNKNOWN,		SDL_SCANCODE_INTERNATIONAL5,		SDL_SCANCODE_UNKNOWN,	SDL_SCANCODE_INTERNATIONAL3,		SDL_SCANCODE_UNKNOWN,		SDL_SCANCODE_UNKNOWN	/* 7 */
};

// subset of method here: http://hg.libsdl.org/SDL/file/131ea7dcc225/src/video/windows/SDL_windowsevents.c
static SDL_Scancode WindowsScanCodeToSDLScanCode(LPARAM lparam, WPARAM wparam)
{
  switch (wparam)
  {
    case VK_PAUSE: return SDL_SCANCODE_PAUSE;
    case VK_NUMLOCK: return SDL_SCANCODE_NUMLOCKCLEAR;
  }

  int nScanCode = (lparam >> 16) & 0xFF;
  SDL_Scancode code = (nScanCode <= 127) ? windows_scancode_table[nScanCode] : SDL_SCANCODE_UNKNOWN;

  if (lparam & (1 << 24))
  {
    switch (code) {
      case SDL_SCANCODE_RETURN:
        code = SDL_SCANCODE_KP_ENTER;
        break;
      case SDL_SCANCODE_LALT:
        code = SDL_SCANCODE_RALT;
        break;
      case SDL_SCANCODE_LCTRL:
        code = SDL_SCANCODE_RCTRL;
        break;
      case SDL_SCANCODE_SLASH:
        code = SDL_SCANCODE_KP_DIVIDE;
        break;
      case SDL_SCANCODE_CAPSLOCK:
        code = SDL_SCANCODE_KP_PLUS;
        break;
      default:
        break;
    }
  }
  else
  {
    switch (code) {
      case SDL_SCANCODE_HOME:
        code = SDL_SCANCODE_KP_7;
        break;
      case SDL_SCANCODE_UP:
        code = SDL_SCANCODE_KP_8;
        break;
      case SDL_SCANCODE_PAGEUP:
        code = SDL_SCANCODE_KP_9;
        break;
      case SDL_SCANCODE_LEFT:
        code = SDL_SCANCODE_KP_4;
        break;
      case SDL_SCANCODE_RIGHT:
        code = SDL_SCANCODE_KP_6;
        break;
      case SDL_SCANCODE_END:
        code = SDL_SCANCODE_KP_1;
        break;
      case SDL_SCANCODE_DOWN:
        code = SDL_SCANCODE_KP_2;
        break;
      case SDL_SCANCODE_PAGEDOWN:
        code = SDL_SCANCODE_KP_3;
        break;
      case SDL_SCANCODE_INSERT:
        code = SDL_SCANCODE_KP_0;
        break;
      case SDL_SCANCODE_DELETE:
        code = SDL_SCANCODE_KP_PERIOD;
        break;
      case SDL_SCANCODE_PRINTSCREEN:
        code = SDL_SCANCODE_KP_MULTIPLY;
        break;
      default:
        break;
    }
  }

  return code;
}

class ChangeInputDialog : public Dialog
{
public:
  static const WORD ID_LABEL = 40000;
  static const WORD IDC_CLEAR = 50000;
  static const WORD IDT_TIMER = 40500;

  const KeyBinds::Binding getButtonDescriptor() const { return _buttonDescriptor; }

  Input* _input = nullptr;
  std::map<SDL_JoystickID, SDL_JoystickID>* _bindingMap = nullptr;
  KeyBinds::BindingList* _bindings = nullptr;
  KeyBinds::Action _button = KeyBinds::Action::kNothing;
  bool _isOpen = false;

  bool show(HWND hParent)
  {
    _updated = false;

    auto hwnd = CreateDialogIndirectParam(NULL, (LPCDLGTEMPLATE)_template, hParent, s_dialogProc, (LPARAM)this);
    EnableWindow(hParent, 0);

    _buttonDescriptor = _bindings->at(static_cast<int>(_button));
    updateBindingString(hwnd);

    ShowWindow(hwnd, 1);

    _isOpen = true;
    while(_isOpen)
    {
      MSG msg;
      while(PeekMessage(&msg, 0, 0, 0, PM_NOREMOVE))
      {
        if(GetMessage(&msg, 0, 0, 0) > 0)
        {
          switch (msg.message)
          {
            case WM_KEYDOWN:
            case WM_SYSKEYDOWN:
            {
              const auto code = WindowsScanCodeToSDLScanCode(msg.lParam, msg.wParam);
              const auto sdlKey = SDL_GetKeyFromScancode(code);
              switch (sdlKey)
              {
                case SDLK_UNKNOWN:
                case SDLK_LALT:
                case SDLK_RALT:
                case SDLK_LCTRL:
                case SDLK_RCTRL:
                case SDLK_LSHIFT:
                case SDLK_RSHIFT:
                  // don't allow modifiers by themselves
                  break;

                default:
                  _buttonDescriptor.type = KeyBinds::Binding::Key;
                  _buttonDescriptor.joystick_id = -1;
                  _buttonDescriptor.button = sdlKey;
                  _buttonDescriptor.modifiers = 0;

                  if (GetKeyState(VK_CONTROL) & 0x8000)
                    _buttonDescriptor.modifiers |= KMOD_CTRL;
                  if (GetKeyState(VK_MENU) & 0x8000)
                    _buttonDescriptor.modifiers |= KMOD_ALT;
                  if (GetKeyState(VK_SHIFT) & 0x8000)
                    _buttonDescriptor.modifiers |= KMOD_SHIFT;

                  updateBindingString(hwnd);
                  break;
              }
            }

            case WM_SYSCOMMAND:
              break;

            default:
              if(!IsDialogMessage(hwnd, &msg))
              {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
              }
              break;
          }
        }
      }
      Sleep(10);
    }

    EnableWindow(hParent, 1);

    return _updated;
  }

protected:
  KeyBinds::Binding _buttonDescriptor;

  void initControls(HWND hwnd) override
  {
    Dialog::initControls(hwnd);

    SetTimer(hwnd, IDT_TIMER, 100, (TIMERPROC)NULL);
  }

  void retrieveData(HWND hwnd) override
  {
    Dialog::retrieveData(hwnd);

    // _updated is normally only set if a field changes, since this dialog has no fields,
    // always return "modified" if the user didn't cancel the dialog.
    _updated = true;
  }

  void markClosed(HWND hwnd) override
  {
    Dialog::markClosed(hwnd);
    _isOpen = false;
  }

  INT_PTR dialogProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) override
  {
    switch (msg)
    {
      case WM_COMMAND:
      {
        int controlId = LOWORD(wparam);
        if (controlId == IDC_CLEAR)
        {
          _buttonDescriptor.joystick_id = 0;
          _buttonDescriptor.type = KeyBinds::Binding::Type::None;
          _buttonDescriptor.button = 0;
          _buttonDescriptor.modifiers = 0;

          updateBindingString(hwnd);
        }
        break;
      }

      case WM_TIMER:
        if (wparam == IDT_TIMER)
        {
          KeyBinds::Binding button = _input->captureButtonPress();
          if (button.type != KeyBinds::Binding::Type::None)
          {
            _buttonDescriptor = button;

            auto pair = _bindingMap->find(button.joystick_id);
            if (pair != _bindingMap->end())
              _buttonDescriptor.joystick_id = pair->second;

            updateBindingString(hwnd);
          }
        }
        break;

      case WM_ACTIVATE:
        if (wparam != WA_INACTIVE)
        {
          updateBindingString(hwnd);
        }
        else
        {
          std::string sMessage;
          sMessage.resize(128);
          int nChars = GetDlgItemText(hwnd, ID_LABEL, (LPSTR)sMessage.data(), sMessage.capacity());
          sMessage.resize(nChars);
          size_t nIndex = sMessage.find_first_of('\n');
          if (nIndex != std::string::npos)
            sMessage.resize(nIndex);
          sMessage.append("\nFocus this window to enter new input");
          SetDlgItemText(hwnd, ID_LABEL, sMessage.c_str());
        }
        break;
    }

    return Dialog::dialogProc(hwnd, msg, wparam, lparam);
  }

  void updateBindingString(HWND hwnd)
  {
    char buffer[32];
    KeyBinds::getBindingString(buffer, _buttonDescriptor);

    if (_buttonDescriptor.type != KeyBinds::Binding::Type::None)
    {
      for (size_t i = 0; i < _bindings->size(); ++i)
      {
        if (i == static_cast<size_t>(_button))
          continue;

        const KeyBinds::Binding& binding = _bindings->at(i);
        if (binding.button == _buttonDescriptor.button &&
          binding.modifiers == _buttonDescriptor.modifiers &&
          binding.type == _buttonDescriptor.type)
        {
          if (binding.type == KeyBinds::Binding::Type::Key || binding.joystick_id == _buttonDescriptor.joystick_id)
          {
            std::string sMessage = buffer;
            sMessage.append("\nWARNING: Multiple bindings");
            SetDlgItemText(hwnd, ID_LABEL, sMessage.c_str());
            return;
          }
        }
      }
    }

    SetDlgItemText(hwnd, ID_LABEL, buffer);
  }
};

class InputDialog : public Dialog
{
public:
  void initControllerButtons(const KeyBinds::BindingList& bindings, int base)
  {
    _bindings = bindings;
    _firstBinding = base;

    const WORD WIDTH = 478;
    const WORD HEIGHT = 220;

    addButtonInput(0, 1, "L2", kJoy0L2 + base);
    addButtonInput(0, 9, "R2", kJoy0R2 + base);
    addButtonInput(1, 1, "L1", kJoy0L + base);
    addButtonInput(1, 9, "R1", kJoy0R + base);
    addButtonInput(2, 1, "Up", kJoy0Up + base);
    addButtonInput(2, 4, "Select", kJoy0Select + base);
    addButtonInput(2, 6, "Start", kJoy0Start + base);
    addButtonInput(2, 9, "X", kJoy0X + base);
    addButtonInput(3, 0, "Left", kJoy0Left + base);
    addButtonInput(3, 2, "Right", kJoy0Right + base);
    addButtonInput(3, 8, "Y", kJoy0Y + base);
    addButtonInput(3, 10, "A", kJoy0A + base);
    addButtonInput(4, 1, "Down", kJoy0Down + base);
    addButtonInput(4, 9, "B", kJoy0B + base);

    addButtonInput(5, 1, "Left Analog Up", kJoy0LeftAnalogUp + base);
    addButtonInput(5, 4, "L3", kJoy0L3 + base);
    addButtonInput(5, 6, "R3", kJoy0R3 + base);
    addButtonInput(5, 9, "Right Analog Up", kJoy0RightAnalogUp + base);
    addButtonInput(6, 0, "Left Analog Left", kJoy0LeftAnalogLeft + base);
    addButtonInput(6, 2, "Left Analog Right", kJoy0LeftAnalogRight + base);
    addButtonInput(6, 8, "Right Analog Left", kJoy0RightAnalogLeft + base);
    addButtonInput(6, 10, "Right Analog Right", kJoy0RightAnalogRight + base);
    addButtonInput(7, 1, "Left Analog Down", kJoy0LeftAnalogDown + base);
    addButtonInput(7, 9, "Right Analog Down", kJoy0RightAnalogDown + base);

    addButton("Export", IDC_EXPORT, 0, HEIGHT - 14, 50, 14, false);
    addButton("Import", IDC_IMPORT, 55, HEIGHT - 14, 50, 14, false);

    addButton("OK", IDOK, WIDTH - 55 - 50, HEIGHT - 14, 50, 14, true);
    addButton("Cancel", IDCANCEL, WIDTH - 50, HEIGHT - 14, 50, 14, false);
  }

  void initHotKeyButtons(const KeyBinds::BindingList& bindings)
  {
    _bindings = bindings;

    const WORD WIDTH = 437;
    const WORD HEIGHT = 326;
    char label[32];

    addButtonInput(0, 0, "Reset", kReset);
    addButtonInput(1, 0, "Take Screenshot", kScreenshot);
    addButtonInput(2, 0, "Game Focus", kGameFocusToggle);
    addButtonInput(3, 0, "Pause", kPauseToggleNoOvl);
    addButtonInput(4, 0, "Frame Advance", kStep);
    addButtonInput(5, 0, "Fast Forward (Hold)", kFastForward);
    addButtonInput(6, 0, "Fast Forward (Toggle)", kFastForwardToggle);

    addButtonInput(0, 2, "Window Size 1x", kSetWindowSize1);
    addButtonInput(1, 2, "Window Size 2x", kSetWindowSize2);
    addButtonInput(2, 2, "Window Size 3x", kSetWindowSize3);
    addButtonInput(3, 2, "Window Size 4x", kSetWindowSize4);
    addButtonInput(4, 2, "Window Size 5x", kSetWindowSize5);
    addButtonInput(5, 2, "Rotate Right", kRotateRight);
    addButtonInput(6, 2, "Rotate Left", kRotateLeft);
    addButtonInput(7, 2, "Toggle Fullscreen", kToggleFullscreen);
    addButtonInput(8, 2, "Show Overlay", kPauseToggle);

    for (int i = 0; i < 10; ++i)
    {
      snprintf(label, sizeof(label), "Save State %d", i + 1);
      addButtonInput(i, 5, label, kSaveState1 + i);
    }
    addButtonInput(10, 5, "Save Current State", kSaveCurrent);

    for (int i = 0; i < 10; ++i)
    {
      snprintf(label, sizeof(label), "Load State %d", i + 1);
      addButtonInput(i, 7, label, kLoadState1 + i);
    }
    addButtonInput(10, 7, "Load Current State", kLoadCurrent);

    for (int i = 0; i < 10; ++i)
    {
      snprintf(label, sizeof(label), "Select State %d", i + 1);
      addButtonInput(i, 9, label, kSetSlot1 + i);
    }
    addButtonInput(10, 9, "Select Previous State", kPreviousSlot);
    addButtonInput(11, 9, "Select Next State", kNextSlot);

    addButton("OK", IDOK, WIDTH - 55 - 50, HEIGHT - 14, 50, 14, true);
    addButton("Cancel", IDCANCEL, WIDTH - 50, HEIGHT - 14, 50, 14, false);
  }

  Input* _input = nullptr;
  std::map<SDL_JoystickID, SDL_JoystickID>* _bindingMap = nullptr;
  Logger* _logger = nullptr;

  const KeyBinds::BindingList& getBindings() const { return _bindings; }

protected:
  KeyBinds::BindingList _bindings = {};
  int _firstBinding = kJoy0Up;
  std::array<char[32], kMaxBindings> _buttonLabels = {};

  static const WORD IDC_EXPORT = 20000 - 1;
  static const WORD IDC_IMPORT = IDC_EXPORT - 1;

  void retrieveData(HWND hwnd) override
  {
    Dialog::retrieveData(hwnd);

    // _updated is normally only set if a field changes, since this dialog has no fields,
    // always return "modified" if the user didn't cancel the dialog.
    _updated = true;
  }

  void updateButtonLabel(int button)
  {
    KeyBinds::Binding& desc = _bindings[button];
    KeyBinds::getBindingString(_buttonLabels[button], desc);
  }

  void addButtonInput(int row, int column, const char* label, int button)
  {
    const WORD ITEM_WIDTH = 80;
    const WORD ITEM_HEIGHT = 10;

    const WORD x = column * (ITEM_WIDTH / 2) + 3;
    const WORD y = row * (ITEM_HEIGHT * 2 + 6);

    addButton(label, 20000 + button, x, y, ITEM_WIDTH -6 , ITEM_HEIGHT, false);

    updateButtonLabel(button);
    addEditbox(10000 + button, x, y + ITEM_HEIGHT, ITEM_WIDTH - 6, ITEM_HEIGHT, 1, _buttonLabels[button], 16, true);
  }

  INT_PTR dialogProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) override
  {
    switch (msg)
    {
      case WM_COMMAND:
      {
        int controlId = LOWORD(wparam);
        if (controlId >= 20000 && controlId <= 20000 + kMaxBindings)
          updateButton(hwnd, controlId - 20000);
        else if (controlId == IDC_EXPORT)
          exportJson(hwnd);
        else if (controlId == IDC_IMPORT)
          importJson(hwnd);
        break;
      }
    }

    return Dialog::dialogProc(hwnd, msg, wparam, lparam);
  }

  void updateButton(HWND hwnd, int button)
  {
    char buffer[32];
    GetDlgItemText(hwnd, 20000 + button, buffer, sizeof(buffer));

    ChangeInputDialog db;
    db.init(buffer);
    db._input = _input;
    db._bindingMap = _bindingMap;
    db._bindings = &_bindings;
    db._button = static_cast<KeyBinds::Action>(button);

    GetDlgItemText(hwnd, 10000 + button, buffer, sizeof(buffer));
    db.addLabel(buffer, ChangeInputDialog::ID_LABEL, 0, 0, 200, 18);

    db.addButton("Clear", ChangeInputDialog::IDC_CLEAR, 0, 20, 50, 14, false);
    db.addButton("OK", IDOK, 80, 20, 50, 14, true);
    db.addButton("Cancel", IDCANCEL, 140, 20, 50, 14, false);

    // the joystick is normally ignored if the SDL window doesn't have focus
    // temporarily disable that behavior
    SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");

    if (db.show(hwnd))
    {
      _bindings[button] = db.getButtonDescriptor();
      updateButtonLabel(button);
      SetDlgItemText(hwnd, 10000 + button, _buttonLabels[button]);
    }

    SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "0");
  }

  void exportJson(HWND hwnd)
  {
    std::string extensions = "JSON Files (*.json)";
    extensions.append("\0", 1);
    extensions.append("*.json");
    extensions.append("\0", 2);
    std::string path = util::saveFileDialog(hwnd, extensions, "json");

    if (!path.empty())
    {
      const int numBindings = (kJoy1Up - kJoy0Up);
      char binding[32];
      std::string bindings = "{";
      bindings.reserve(numBindings * 16);

      for (int i = 0; i < numBindings; ++i)
      {
        if (i != 0)
          bindings += ",";

        bindings += "\"";
        bindings += bindingNames[kJoy0Up + i]; /* always write J0_ */
        bindings += "\":\"";

        KeyBinds::getBindingString(binding, _bindings[_firstBinding + i]);
        bindings += util::jsonEscape(binding);
        bindings += "\"";
      }

      bindings += "}";

      util::saveFile(_logger, path, bindings.data(), bindings.length());
    }
  }

  void importJson(HWND hwnd)
  {
    std::string extensions = "JSON Files (*.json)";
    extensions.append("\0", 1);
    extensions.append("*.json");
    extensions.append("\0", 2);
    std::string path = util::openFileDialog(hwnd, extensions);

    if (!path.empty())
    {
      std::string json = util::loadFile(_logger, path);
      struct Deserialize
      {
        InputDialog* dialog;
        HWND hwnd;
        std::string key;
      };
      Deserialize ud;
      ud.dialog = this;
      ud.hwnd = hwnd;

      jsonsax_result_t res = jsonsax_parse((char*)json.c_str(), &ud, [](void* udata, jsonsax_event_t event, const char* str, size_t num)
      {
        auto* ud = (Deserialize*)udata;

        if (event == JSONSAX_KEY)
        {
          ud->key = std::string(str, num);
        }
        else if (event == JSONSAX_STRING)
        {
          for (int i = kJoy0Up; i < kJoy1Up; ++i)
          {
            if (ud->key == bindingNames[i])
            {
              const int button = ud->dialog->_firstBinding + i;
              parseBindingString(util::jsonUnescape(std::string(str, num)), ud->dialog->_bindings[button]);
              ud->dialog->updateButtonLabel(button);
              SetDlgItemText(ud->hwnd, 10000 + button, ud->dialog->_buttonLabels[button]);
            }
          }
        }

        return 0;
      });
    }
  }
};

void KeyBinds::showControllerDialog(Input& input, int port)
{
  char label[32];
  snprintf(label, sizeof(label), "Controller %u", port + 1);

  InputDialog db;
  db.init(label);
  db._input = &input;
  db._bindingMap = &_bindingMap;
  db._logger = _logger;

  switch (port)
  {
    case 0:
      db.initControllerButtons(_bindings, kJoy0Up);
      break;
    case 1:
      db.initControllerButtons(_bindings, kJoy1Up);
      break;
  }

  if (db.show())
    _bindings = db.getBindings();
}

void KeyBinds::showHotKeyDialog(Input& input)
{
  InputDialog db;
  db.init("Hot Keys");
  db._input = &input;
  db._bindingMap = &_bindingMap;
  db._logger = _logger;

  db.initHotKeyButtons(_bindings);

  if (db.show())
    _bindings = db.getBindings();
}
