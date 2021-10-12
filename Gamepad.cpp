#include "Gamepad.h"

#include <SFML/Graphics.hpp>
#include <string>
#include <fstream>
#include <iostream>

/// XInput
#include "Windows.h"
#include "Xinput.h"
#pragma comment(lib,"XInput.lib") // include libraries
#pragma comment(lib,"Xinput9_1_0.lib") // include libraries

Gamepad::Gamepad(int number, sf::Joystick::Identification data) : gamepadNumber(number)
{
	bool success = false;

	/// XINPUT
	DWORD dwResult;
	XINPUT_STATE state;
	ZeroMemory(&state, sizeof(XINPUT_STATE));
	dwResult = XInputGetState(number, &state);

	if (dwResult == ERROR_SUCCESS) { // XBOX Controller is connected
		isXInput = true;
		success = true;
		std::cout << "XInput device found.";
	}
	/// SDL DATABASE
	else // XBOX Controller is not connected -> check database
	{
		isXInput = false;
		success = true;

		// open db
		std::ifstream gamepads("resources/gamecontrollerdb.txt");
		if (!gamepads.is_open()) {
			std::cout << "Gamepad database not found!";
			return;
		}

		// Windows section
		std::string line = "";
		while (line != "# Windows") std::getline(gamepads, line);

		// Get vendorID and productID from GUID
		/* EXAMPLE:
			030000005e040000d102000000000000
					\__/    \__/
				   vendor  product
		   vendor: 5e04 -> 0x045e Hex -> 1118 decimal (Microsoft)
		   product: d102 -> 0x02d1 Hex -> 721 decimal (Xbox One Controller)
		*/
		unsigned int vID = 0;
		unsigned int pID = 0;
		while (!(vID == data.vendorId && pID == data.productId) || gamepads.eof()) {
			std::getline(gamepads, line); // entire line
			vID = std::stoul(line.substr(10, 2) + line.substr(8, 2), nullptr, 16); // hex string to unsigned int
			pID = std::stoul(line.substr(18, 2) + line.substr(16, 2), nullptr, 16);
		}
		if (!gamepads.eof()) {
			std::cout << data.name.toAnsiString() << " found.";
			LoadData(line);
		}

		// close db
		gamepads.close();
	}

	if (!success)
		std::cout << "Gamepad not recognized!";
		// TO DO: manual configuration by user
}

bool Gamepad::isButtonPressed(GAMEPAD_BUTTON btn)
{
	if (isXInput) {
		XINPUT_STATE state;
		ZeroMemory(&state, sizeof(XINPUT_STATE));
		XInputGetState(gamepadNumber, &state);

		if (state.Gamepad.wButtons == enumToButton(btn)) return true;
	}
	
	return (sf::Joystick::isButtonPressed(gamepadNumber, enumToButton(btn)));
}

float Gamepad::getAxisPosition(GAMEPAD_AXIS axis)
{
	XINPUT_STATE state;
	if (isXInput) {
		ZeroMemory(&state, sizeof(XINPUT_STATE));
		XInputGetState(gamepadNumber, &state);
	}
	GamePadAxis ax;

	switch (axis)
	{
		case GAMEPAD_AXIS::leftStick_X: {
			if (isXInput) return shrinkValue(state.Gamepad.sThumbLX, false);
			else ax = LSTICK_X;
		} break;
		case GAMEPAD_AXIS::leftStick_Y: {
			if (isXInput) return shrinkValue(-state.Gamepad.sThumbLY, false);
			else ax = LSTICK_Y;
		} break;
		case GAMEPAD_AXIS::rightStick_X: {
			if (isXInput) return shrinkValue(state.Gamepad.sThumbRX, false);
			else ax = RSTICK_X;
		} break;
		case GAMEPAD_AXIS::rightStick_Y: {
			if (isXInput) return shrinkValue(-state.Gamepad.sThumbRY, false);
			else ax = RSTICK_Y;
		} break;
	}

	float p = sf::Joystick::getAxisPosition(gamepadNumber, ax.Axis);
	if (ax.inverted) p = -p;
	return p;
}

float Gamepad::getTriggerValue(GAMEPAD_TRIGGER tgr)
{
	if (isXInput) {
		XINPUT_STATE state;
		ZeroMemory(&state, sizeof(XINPUT_STATE));
		XInputGetState(gamepadNumber, &state);

		if (tgr == GAMEPAD_TRIGGER::leftTrigger) return shrinkValue(state.Gamepad.bLeftTrigger, true);
		else return shrinkValue(state.Gamepad.bRightTrigger, true);
	}
	else {
		if (tgr == GAMEPAD_TRIGGER::leftTrigger) {
			// as button
			if (sf::Joystick::isButtonPressed(gamepadNumber, LTRIGGER.button)) return 100.f;
			// as axis
			else if (LTRIGGER.isAxis) {
				float p = sf::Joystick::getAxisPosition(gamepadNumber, LTRIGGER.Axis.Axis);
				if (LTRIGGER.Axis.inverted) p = -p;
				return p;
			}
			else return 0.f;
		}
		else {
			// as button
			if (sf::Joystick::isButtonPressed(gamepadNumber, RTRIGGER.button)) return 100.f;
			// as axis
			else if (RTRIGGER.isAxis) {
				float p = sf::Joystick::getAxisPosition(gamepadNumber, RTRIGGER.Axis.Axis);
				if (RTRIGGER.Axis.inverted) p = -p;
				return p;
			}
			else return 0.f;
		}
	}
}

void Gamepad::LoadData(std::string line)
{
	// EXAMPLE:
	/* 030000005e040000d102000000000000,Xbox One Controller,
		a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,
		leftshoulder:b4,leftstick:b8,lefttrigger:a2,leftx:a0,lefty:a1,
		rightshoulder:b5,rightstick:b9,righttrigger:a5,rightx:a3,
		righty:a4,start:b7,x:b2,y:b3,platform:Windows,
	*/
	int pos = line.find(",");
	line = line.substr(pos + 1); // clear GUID "Xbox One Controller,a:b0,b..."
	pos = line.find(",");
	line = line.substr(pos + 1); // clear name "a:b0,b:b1,back:b6,d..."

	while (line.substr(0, 8) != "platform") {
		// extract name and data of current button/axis
		pos = line.find(":");
		int endPos = line.find(",");
		std::string name = line.substr(0, pos); // "a"
		std::string data = line.substr(pos + 1, endPos - pos - 1); // "b0"

		if (data[0] == 'b') // button (TO DO: DPAD)
			assert(assignButton(name, data) && "Button not found!");
		else if (data[0] == 'a' || data[0] == '-' || data[0] == '+') // axis
			assert(assignAxis(name, data) && "Axis not found!");

		// next button or axis
		pos = line.find(",");
		line = line.substr(pos + 1);
	}
}

bool Gamepad::assignButton(std::string n, std::string d)
{
	int b = std::stoi(d.substr(1, d.size() - 1)); // just number -> "b0" -> "0"

	if (n == "a") A = b;
	else if (n == "b") B = b;
	else if (n == "x") X = b;
	else if (n == "y") Y = b;
	else if (n == "back") BACK = b;
	else if (n == "start") START = b;
	else if (n == "leftshoulder") LB = b;
	else if (n == "rightshoulder") RB = b;
	else if (n == "leftstick") LSTICK = b;
	else if (n == "rightstick") RSTICK = b;
	else if (n == "lefttrigger") {
		LTRIGGER.isAxis = false;
		LTRIGGER.button = b;
	}
	else if (n == "righttrigger") {
		RTRIGGER.isAxis = false;
		RTRIGGER.button = b;
	}
	else if (n == "guide") return true; // ignore button "guide"
	else return false;

	return true;
}

bool Gamepad::assignAxis(std::string n, std::string d)
{
	// possible input: "a0" , "-a1", +a3"

	bool inverted = (d[0] == '-');
	int a;
	((d[0] == '-' || d[0] == '+'))
		? a = std::stoi(d.substr(2)) // "-a2" -> "2"
		: a = std::stoi(d.substr(1)); // "a2" -> "2"

	if (n == "leftx") {
		LSTICK_X.Axis = extractAxis(a);
		LSTICK_X.inverted = inverted;
	}
	else if (n == "lefty") {
		LSTICK_Y.Axis = extractAxis(a);
		LSTICK_Y.inverted = inverted;
	}
	else if (n == "rightx") {
		RSTICK_X.Axis = extractAxis(a);
		RSTICK_X.inverted = inverted;
	}
	else if (n == "righty") {
		RSTICK_Y.Axis = extractAxis(a);
		RSTICK_Y.inverted = inverted;
	}
	else if (n == "lefttrigger") {
		LTRIGGER.isAxis = true;
		LTRIGGER.Axis.Axis = extractAxis(a);
		LTRIGGER.Axis.inverted = inverted;
	}
	else if (n == "righttrigger") {
		RTRIGGER.isAxis = true;
		RTRIGGER.Axis.Axis = extractAxis(a);
		RTRIGGER.Axis.inverted = inverted;
	}
	else return false;

	return true;
}

sf::Joystick::Axis Gamepad::extractAxis(int axisNumber)
{
	switch (axisNumber)
	{
		case 0:	return sf::Joystick::Axis::X;
		case 1: return sf::Joystick::Axis::Y;
		case 2:	return sf::Joystick::Axis::Z;
		case 3: return sf::Joystick::Axis::R; 
		case 4:	return sf::Joystick::Axis::U;
		case 5: return sf::Joystick::Axis::V;
		case 6:	return sf::Joystick::Axis::PovX;
		case 7: return sf::Joystick::Axis::PovY;
	}

	assert(false && "Axis Number out of range");
}

int Gamepad::enumToButton(GAMEPAD_BUTTON btn)
{
	switch (btn)
	{
	case GAMEPAD_BUTTON::btn_a: if (isXInput) return XINPUT_GAMEPAD_A; return A;
	case GAMEPAD_BUTTON::btn_b: if (isXInput) return XINPUT_GAMEPAD_B; return B;
	case GAMEPAD_BUTTON::btn_x: if (isXInput) return XINPUT_GAMEPAD_X; return X;
	case GAMEPAD_BUTTON::btn_y: if (isXInput) return XINPUT_GAMEPAD_Y; return Y;
	case GAMEPAD_BUTTON::btn_leftStick: if (isXInput) return XINPUT_GAMEPAD_LEFT_THUMB; return LSTICK;
	case GAMEPAD_BUTTON::btn_rightStick: if (isXInput) return XINPUT_GAMEPAD_RIGHT_THUMB; return RSTICK;
	case GAMEPAD_BUTTON::btn_back: if (isXInput) return XINPUT_GAMEPAD_BACK; return BACK;
	case GAMEPAD_BUTTON::btn_start: if (isXInput) return XINPUT_GAMEPAD_START; return START;
	case GAMEPAD_BUTTON::btn_lb: if (isXInput) return XINPUT_GAMEPAD_LEFT_SHOULDER; return LB;
	case GAMEPAD_BUTTON::btn_rb: if (isXInput) return XINPUT_GAMEPAD_RIGHT_SHOULDER; return RB;
	}
}
float Gamepad::shrinkValue(float f, bool trigger)
{
	/*
	Triggers: 
		old: 0 to 255
		new: 0 to 100
	Thumbs:
		old: -32768 to 32767
		new: -100 to 100
	*/
	float oldMin = (f < 0) ? -32768 : 0;
	float oldMax = (trigger) ? 255 : 32767;
	float newMin = (f < 0) ? -100 : 0;
	float newMax = 100;

	float oldPercent = (f - oldMin) / (oldMax - oldMin);
	return ((newMax - newMin) * oldPercent + newMin);
}
