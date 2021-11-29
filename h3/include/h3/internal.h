#ifndef _H3_INTERNAL_H_
#define _H3_INTERNAL_H_

#include <h3/common.h>
#include <string>

#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <Windows.h>

#include <Xinput.h>

void               H3Internal_SetAssertMessage(const std::string& msg);
const std::string& H3Internal_GetAssertMessage();

void               H3Internal_HandleAssertPopup();

class Gamepad
{
private:
	int cId;
	XINPUT_STATE state;

	float deadzoneX;
	float deadzoneY;

public:
	Gamepad() : deadzoneX(0.05f), deadzoneY(0.02f) {}
	Gamepad(float dzX, float dzY) : deadzoneX(dzX), deadzoneY(dzY) {}

	float leftStickX;
	float leftStickY;
	float rightStickX;
	float rightStickY;
	float leftTrigger;
	float rightTrigger;

	int  GetPort();
	XINPUT_GAMEPAD* GetState();
	bool CheckConnection();
	bool Refresh();
	bool IsPressed(WORD);
};

#endif  /* _H3_INTERNAL_H_ */
