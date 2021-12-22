#include "GamepadMgr.h"

GamepadMgr::GamepadMgr()
{
	_gamepadOne = nullptr;
	_gamepadTwo = nullptr;
}

bool GamepadMgr::checkIfVendorIsMicrosoftOnDB(int number)
{
	sf::Joystick::Identification data = sf::Joystick::getIdentification(number);

	return data.vendorId == 1118;

	// open db
	std::ifstream gamepads("resources/gamecontrollerdb.txt");
	if (!gamepads.is_open()) {
		std::cout << "Gamepad database not found!";
		return false;
	}

	// Windows section
	std::string line = "";
	while (line != "# Windows") std::getline(gamepads, line);

	// Get vendorID and productID from GUID
	/*
	Microsoft: 5e04 -> 0x045e Hex -> 1118 decimal
	*/
	unsigned int vID = 0;
	unsigned int pID = 0;
	while (!(vID == data.vendorId && pID == data.productId) || gamepads.eof()) {
		std::getline(gamepads, line); // entire line
		vID = std::stoul(line.substr(10, 2) + line.substr(8, 2), nullptr, 16); // hex string to unsigned int
		pID = std::stoul(line.substr(18, 2) + line.substr(16, 2), nullptr, 16);
	}
	if (!gamepads.eof()) return true;

	return false;
}

void GamepadMgr::Initialize()
{
	// Determine connected joysticks and if they are XInput or not
	/*
	Example: 2 gamepads connected, one XInput and another not XInput
	The XInput device will be number 0 for XInput, but for SFML it can be 0 or 1
	- We need to determine how many XInput devices are connected and assign their
	numbers accordingly.
	*/

	int totalJoysticksConnected = 0;
	int numberOfXInputDevices = 0;
	for (int i = 0; i < 2; i++) {
		if (sf::Joystick::isConnected(i)) totalJoysticksConnected++;

		/// XINPUT
		DWORD dwResult;
		XINPUT_STATE state;
		ZeroMemory(&state, sizeof(XINPUT_STATE));
		dwResult = XInputGetState(i, &state);

		if (dwResult == ERROR_SUCCESS) // XBOX Controller is connected
			numberOfXInputDevices++;
	}

	if (totalJoysticksConnected == 0) return;
	if (totalJoysticksConnected == 1 && numberOfXInputDevices == 0) {
		_gamepadOne = new Gamepad(0, false);
	}
	else if (totalJoysticksConnected == 1 && numberOfXInputDevices == 1) {
		_gamepadOne = new Gamepad(0, true);
	}
	else if (totalJoysticksConnected == 2 && numberOfXInputDevices == 2) {
		_gamepadOne = new Gamepad(0, true);
		_gamepadTwo = new Gamepad(1, true);
	}
	else if (totalJoysticksConnected == 2 && numberOfXInputDevices == 0) {
		_gamepadOne = new Gamepad(0, false);
		_gamepadTwo = new Gamepad(1, false);
	}
	else if (totalJoysticksConnected == 2 && numberOfXInputDevices == 1) {
		// The XInput device will be number 0 for XInput, but we don't know the number
		// assigned for SFML, could be 0 or 1.

		// Check db, if vendorID is Microsoft (5e04 -> 0x045e Hex -> 1118 decimal)
		// then that's XInput. It will leave out XBOX controllers by other manufacturers,
		// but they will be handled by SDL's DB

		if (checkIfVendorIsMicrosoftOnDB(0)) { // For the XInput Gamepad XInput number and SFML number coincide = 0
			_gamepadOne = new Gamepad(0, true); // XInput Gamepad
			_gamepadTwo = new Gamepad(1, false); // The other Gamepad
		}
		else { // For the XInput Gamepad XInput number is 0 and SFML number is 1
			_gamepadOne = new Gamepad(0, false); // The other Gamepad
			_gamepadTwo = new Gamepad(0, true); // XInput Gamepad
		}
	}
}


