/**
 * Wii64 - FocusManager.h
 * Copyright (C) 2009 sepp256
 *
 * Wii64 homepage: http://www.emulatemii.com
 * email address: sepp256@gmail.com
 *
 *
 * This program is free software; you can redistribute it and/
 * or modify it under the terms of the GNU General Public Li-
 * cence as published by the Free Software Foundation; either
 * version 2 of the Licence, or any later version.
 *
 * This program is distributed in the hope that it will be use-
 * ful, but WITHOUT ANY WARRANTY; without even the implied war-
 * ranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public Licence for more details.
 *
**/

#ifndef FOCUSMANAGER_H
#define FOCUSMANAGER_H

#include "GuiTypes.h"

//Custom Defines for PS3 buttons
// 0: UP, 1: DOWN
#define PS3_BTN_LEFT		(1<<15)
#define PS3_BTN_DOWN		(1<<14)
#define PS3_BTN_RIGHT		(1<<13)
#define PS3_BTN_UP			(1<<12)
#define PS3_BTN_START		(1<<11)
#define PS3_BTN_R3			(1<<10)
#define PS3_BTN_L3			(1<<9)
#define PS3_BTN_SELECT		(1<<8)
#define PS3_BTN_SQUARE		(1<<7)
#define PS3_BTN_CROSS		(1<<6)
#define PS3_BTN_CIRCLE		(1<<5)
#define PS3_BTN_TRIANGLE	(1<<4)
#define PS3_BTN_R1			(1<<3)
#define PS3_BTN_L1			(1<<2)
#define PS3_BTN_R2			(1<<1)
#define PS3_BTN_L2			(1<<0)

namespace menu {

class Focus
{
public:
	void updateFocus();
	void addComponent(Component* component);
	void removeComponent(Component* component);
	Frame* getCurrentFrame();
	void setCurrentFrame(Frame* frame);
	void setFocusActive(bool active);
	void clearInputData();
	void clearPrimaryFocus();
	void setFreezeAction(bool freezeAction);
	enum FocusDirection
	{
		DIRECTION_NONE=0,
		DIRECTION_LEFT,
		DIRECTION_RIGHT,
		DIRECTION_DOWN,
		DIRECTION_UP
	};
	enum FocusAction
	{
		ACTION_SELECT=1,
		ACTION_BACK=2
	};

	static Focus& getInstance()
	{
		static Focus obj;
		return obj;
	}

private:
	Focus();
	~Focus();
	bool focusActive, pressed, frameSwitch, clearInput, freezeAction;
	int buttonsPressed, previousButtonsPressed;
	u32 previousButtonsWii[4];
	u16 previousButtonsGC[4];
	u16 previousButtonsPS3[7];
	ComponentList focusList;
	Component *primaryFocusOwner, *secondaryFocusOwner;
	Frame *currentFrame;

};

} //namespace menu 

#endif
