/**
 * Wii64 - controller-PS3.c
 * Copyright (C) 2007, 2008, 2009, 2010 Mike Slegeir
 * Copyright (C) 2007, 2008, 2009, 2010, 2011 sepp256
 * 
 * PS3 controller input module
 *
 * Wii64 homepage: http://www.emulatemii.com
 * email address: tehpola@gmail.com
 *                sepp256@gmail.com
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


#include <string.h>
#include <io/pad.h>
#include "controller.h"

enum {
	L_STICK_AS_ANALOG = 1, R_STICK_AS_ANALOG = 2,
};

enum {
	PS3_BTN_LEFT		= (1<<15),
	PS3_BTN_DOWN		= (1<<14),
	PS3_BTN_RIGHT		= (1<<13),
	PS3_BTN_UP			= (1<<12),
	PS3_BTN_START		= (1<<11),
	PS3_BTN_R3			= (1<<10),
	PS3_BTN_L3			= (1<<9),
	PS3_BTN_SELECT		= (1<<8),
	PS3_BTN_SQUARE		= (1<<7),
	PS3_BTN_CROSS		= (1<<6),
	PS3_BTN_CIRCLE		= (1<<5),
	PS3_BTN_TRIANGLE	= (1<<4),
	PS3_BTN_R1			= (1<<3),
	PS3_BTN_L1			= (1<<2),
	PS3_BTN_R2			= (1<<1),
	PS3_BTN_L2			= (1<<0),
	L_STICK_L			= 0x01 << 16,
	L_STICK_R			= 0x02 << 16,
	L_STICK_U			= 0x04 << 16,
	L_STICK_D			= 0x08 << 16,
	R_STICK_L			= 0x10 << 16,
	R_STICK_R			= 0x20 << 16,
	R_STICK_U			= 0x40 << 16,
	R_STICK_D			= 0x80 << 16,
};

static button_t buttons[] = {
	{  0, ~0,				"None" },
	{  1, PS3_BTN_UP,		"D-Up" },
	{  2, PS3_BTN_LEFT,		"D-Left" },
	{  3, PS3_BTN_RIGHT,	"D-Right" },
	{  4, PS3_BTN_DOWN,		"D-Down" },
	{  5, PS3_BTN_L1,		"L1" },
	{  6, PS3_BTN_L2,		"L2" },
	{  7, PS3_BTN_L3,		"L3" },
	{  8, PS3_BTN_R1,		"R1" },
	{  9, PS3_BTN_R2,		"R2" },
	{ 10, PS3_BTN_R3,		"R3" },
	{ 11, PS3_BTN_CROSS,	"Cross" },
	{ 12, PS3_BTN_SQUARE,	"Square" },
	{ 13, PS3_BTN_CIRCLE,	"Circle" },
	{ 14, PS3_BTN_TRIANGLE,	"Triangle" },
	{ 15, PS3_BTN_START,	"Start" },
	{ 16, PS3_BTN_SELECT,	"Select" },
	{ 17, R_STICK_U,		"RS-Up" },
	{ 18, R_STICK_L,		"RS-Left" },
	{ 19, R_STICK_R,		"RS-Right" },
	{ 20, R_STICK_D,		"RS-Down" },
	{ 21, L_STICK_U,		"LS-Up" },
	{ 22, L_STICK_L,		"LS-Left" },
	{ 23, L_STICK_R,		"LS-Right" },
	{ 24, L_STICK_D,		"LS-Down" },
};

static button_t analog_sources[] = {
	{ 0, L_STICK_AS_ANALOG,  "Left Stick" },
	{ 1, R_STICK_AS_ANALOG,  "Right Stick" },
};

static button_t menu_combos[] = {
	{ 0, PS3_BTN_SQUARE|PS3_BTN_TRIANGLE,	"Square+Triangle" },
	{ 1, PS3_BTN_START|PS3_BTN_SELECT,		"Start+Select" },
};

static u32 getButtons(u32 buttonsPS3, u32 analogPS3)
{
	//0xRH-RV-LH-LV 0x00 = Left/Up, 0xFF = Right/Down
	u32 b = buttonsPS3;
	s8 LstickX      = (s8) ((int)((analogPS3>>8) & 0xFF) - 128);
	s8 LstickY      = (s8) ((int)((analogPS3>>0) & 0xFF) - 128);
	s8 RstickX      = (s8) ((int)((analogPS3>>24) & 0xFF) - 128);
	s8 RstickY      = (s8) ((int)((analogPS3>>16) & 0xFF) - 128);
	
	if(LstickX    < -25) b |= L_STICK_L;
	if(LstickX    >  25) b |= L_STICK_R;
	if(LstickY    < -25) b |= L_STICK_U;
	if(LstickY    >  25) b |= L_STICK_D;
	
	if(RstickX    < -25) b |= R_STICK_L;
	if(RstickX    >  25) b |= R_STICK_R;
	if(RstickY    < -25) b |= R_STICK_U;
	if(RstickY    >  25) b |= R_STICK_D;
	
	return b;
}

padInfo padinfo;

static u32 previousButtonsPS3[4];
static u32 previousAnalogPS3[4];

static int _GetKeys(int Control, BUTTONS * Keys, controller_config_t* config)
{
	padData paddata;
	u32 buttonsPS3, analogPS3;
	BUTTONS* c = Keys;
	memset(c, 0, sizeof(BUTTONS));

	if (!controller_PS3.available[Control]) return 0;

	ioPadGetData(Control, &paddata);
	if (paddata.len)
	{
		buttonsPS3 = ((paddata.button[2]&0xFF)<<8) | (paddata.button[3]&0xFF);
		analogPS3 = ((paddata.button[4]&0xFF)<<24) | ((paddata.button[5]&0xFF)<<16) | 
					((paddata.button[6]&0xFF)<<8) | (paddata.button[7]&0xFF); 
		//0xRH-RV-LH-LV 0x00 = Left/Up, 0xFF = Right/Down
		previousButtonsPS3[Control] = buttonsPS3;
		previousAnalogPS3[Control] = analogPS3;
	}
	else
	{
		buttonsPS3 = previousButtonsPS3[Control];
		analogPS3 = previousAnalogPS3[Control];
	}

	u32 b = getButtons(buttonsPS3,analogPS3);
	inline int isHeld(button_tp button){
		return (b & button->mask) == button->mask;
	}
	
	c->R_DPAD       = isHeld(config->DR);
	c->L_DPAD       = isHeld(config->DL);
	c->D_DPAD       = isHeld(config->DD);
	c->U_DPAD       = isHeld(config->DU);
	
	c->START_BUTTON = isHeld(config->START);
	c->B_BUTTON     = isHeld(config->B);
	c->A_BUTTON     = isHeld(config->A);

	c->Z_TRIG       = isHeld(config->Z);
	c->R_TRIG       = isHeld(config->R);
	c->L_TRIG       = isHeld(config->L);

	c->R_CBUTTON    = isHeld(config->CR);
	c->L_CBUTTON    = isHeld(config->CL);
	c->D_CBUTTON    = isHeld(config->CD);
	c->U_CBUTTON    = isHeld(config->CU);

	if(config->analog->mask == L_STICK_AS_ANALOG){
		c->X_AXIS = (s8)  ((int)((analogPS3>>8) & 0xFF) - 128);
		c->Y_AXIS = (s8) -((int)((analogPS3>>0) & 0xFF) - 127);
	} else if(config->analog->mask == R_STICK_AS_ANALOG){
		c->X_AXIS = (s8)  ((int)((analogPS3>>24) & 0xFF) - 128);
		c->Y_AXIS = (s8) -((int)((analogPS3>>16) & 0xFF) - 127);
	}
	if(config->invertedY) c->Y_AXIS = -c->Y_AXIS;

	// Return whether the exit button(s) are pressed
	return isHeld(config->exit);
}

static void pause(int Control){
	//PAD_ControlMotor(Control, PAD_MOTOR_STOP);
}

static void resume(int Control){ }

static void rumble(int Control, int rumble){
	//PAD_ControlMotor(Control, rumble ? PAD_MOTOR_RUMBLE : PAD_MOTOR_STOP);
}

static void configure(int Control, controller_config_t* config){
	// Don't know how this should be integrated
}

static void assign(int p, int v){
	// Nothing to do here
}


static void refreshAvailable(void);

controller_t controller_PS3 =
	{ 'P',
	  _GetKeys,
	  configure,
	  assign,
	  pause,
	  resume,
	  rumble,
	  refreshAvailable,
	  {0, 0, 0, 0},
	  sizeof(buttons)/sizeof(buttons[0]),
	  buttons,
	  sizeof(analog_sources)/sizeof(analog_sources[0]),
	  analog_sources,
	  sizeof(menu_combos)/sizeof(menu_combos[0]),
	  menu_combos,
	  { .DU        = &buttons[1],  // D-Pad Up
	    .DL        = &buttons[2],  // D-Pad Left
	    .DR        = &buttons[3],  // D-Pad Right
	    .DD        = &buttons[4],  // D-Pad Down
	    .Z         = &buttons[6],  // L2
	    .L         = &buttons[5],  // L1
	    .R         = &buttons[8],  // R1
	    .A         = &buttons[11], // Cross
	    .B         = &buttons[13], // Circle
	    .START     = &buttons[15], // Start
	    .CU        = &buttons[17], // Right Stick Up
	    .CL        = &buttons[18], // Right Stick Left
	    .CR        = &buttons[19], // Right Stick Right
	    .CD        = &buttons[20], // Right Stick Down
	    .analog    = &analog_sources[0],
	    .exit      = &menu_combos[0],
	    .invertedY = 0,
	  }
	 };

static void refreshAvailable(void){
	//Note: 7 controllers can be connected to PS3. Maybe check up to 7 in the future?
	ioPadGetInfo(&padinfo);
	
	int i;
	for(i=0; i<4; ++i)
		controller_PS3.available[i] = padinfo.status[i];
}
