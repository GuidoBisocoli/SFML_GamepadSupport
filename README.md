# SFML_GamepadSupport
Gamepad support for SFML based on XInput and the game controller DB for SDL

# Installation
1. Download the latest gamecontrollerdb.txt from https://github.com/gabomdq/SDL_GameControllerDB and place it on your resources directory in your project
2. Download and place "Gamepad.h", "Gamepad.cpp", "GamepadMgr.h" and "GamepadMgr.cpp" on your project wherever you like (could be hdr and src directories)
3. Add them to your project

# Usage
1. Initialize Manager, the Manager will take care of initializing and populating every Gamepad whether it is XInput or other.
```
  GamepadMgr::Instance().Initialize();
```
2. Call them through the manager like this:
```
  GamepadMgr::Instance().GamepadOne()->...
  GamepadMgr::Instance().GamepadTwo()->...
```
3. Retrieve data:
```
  // button
  bool shoot = GamepadMgr::Instance().GamepadOne()->isButtonPressed(Gamepad::GAMEPAD_BUTTON::btn_a);
  // axis
  float x = GamepadMgr::Instance().GamepadOne()->getAxisPosition(Gamepad::GAMEPAD_AXIS::leftStick_X);
  float y = GamepadMgr::Instance().GamepadOne()->getAxisPosition(Gamepad::GAMEPAD_AXIS::leftStick_Y);
  // trigger
  float brake = GamepadMgr::Instance().GamepadOne()->getTriggerValue(Gamepad::GAMEPAD_TRIGGER::leftTrigger);
  float accelerate = GamepadMgr::Instance().GamepadOne()->getTriggerValue(Gamepad::GAMEPAD_TRIGGER::rightTrigger);
```
4. Check for an event:
```
if (_currentEvent.type == sf::Event::EventType::JoystickButtonPressed && _currentEvent.joystickButton.joystickId == 0 && GamepadMgr::Instance().GamepadOne()->isButtonPressed(Gamepad::GAMEPAD_BUTTON::btn_start))
  /// do something
```
5. That's it!

# Notes
- Since the majority of gamepads that are used on PC are XBOX controllers the Gamepad Manager checks if the controller is an XInput controller, if not, it then checks SDL database.
- XInput gamepad number is not always the same as the number provided by sf::Joystick, this is determined and handled properly by the Gamepad Manager.
- Inverted axes are supported seamlessly, it always returns SFML convention (left and bottom negative, right and up positive).
- Triggers as buttons or Axes are supported seamlessly, it always returns a value from 0 to 100.

# To dos and limitations
1. Add DPAD support: it can be button, 0.x values or axes.
2. Handle MacOs and Linux gamepads.
3. If there is another XInput device connected to the computer (a wheel for example) at the moment of execution, it will detect it as a Gamepad. Fix this by ignoring XInput devices that are not Gamepads.
4. Connecting and disconnecting at runtime is not handled.
5. Maximum of 2 gamepads limitation.
