# SFML_GamepadSupport
Gamepad support for SFML based on XInput and the game controller DB for SDL

# Installation
1. Download the latest gamecontrollerdb.txt from https://github.com/gabomdq/SDL_GameControllerDB and place it on your resources directory in your project
2. Download and place "Gamepad.h" and "Gamepad.cpp" on your project wherever you like (could be hdr and src directories)
3. Add them to your project

# Usage
1. Create a Gamepad
```
if (sf::Joystick::isConnected(0))
  _gamepadOne = new Gamepad(0, sf::Joystick::getIdentification(0));  
```
2. Retrieve data:
```
  // button
  bool shoot = _gamepadOne->isButtonPressed(Gamepad::GAMEPAD_BUTTON::btn_a);
  // axis
  float x = _gamepadOne->getAxisPosition(Gamepad::GAMEPAD_AXIS::leftStick_X);
  float y = _gamepadOne->getAxisPosition(Gamepad::GAMEPAD_AXIS::leftStick_Y);
  // trigger
  float brake = _gamepadOne->getTriggerValue(Gamepad::GAMEPAD_TRIGGER::leftTrigger);
  float accelerate = _gamepadOne->getTriggerValue(Gamepad::GAMEPAD_TRIGGER::rightTrigger);
```
3. Check for an event:
```
if (_currentEvent.type == sf::Event::EventType::JoystickButtonPressed && _currentEvent.joystickButton.joystickId == 0 && _gamepadOne->isButtonPressed(Gamepad::GAMEPAD_BUTTON::btn_start))
  /// do something
```
4. That's it!

# Notes
- Since the majority of gamepads that are used on PC are XBOX controllers the Gamepad contructor checks if the controller is an XInput controller, if not, it then checks SDL database.
- Inverted axes are supported seamlessly, it always returns SFML convention (left and bottom negative, right and up positive)
- Triggers as buttons or Axes are supported seamlessly, it always returns a value from 0 to 100

# To do
1. Add DPAD support: it can be button, 0.x values or axes
2. Handle MacOs and Linux gamepads
