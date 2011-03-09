/**
 * Wii64 - MenuContext.cpp
 * Copyright (C) 2009, 2010 sepp256
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

#include "MenuContext.h"
#include "../libgui/FocusManager.h"
#include "../libgui/CursorManager.h"

MenuContext *pMenuContext;
extern "C" void dbg_printf(const char *fmt,...);

MenuContext::MenuContext(GXRModeObj *vmode)
		: currentActiveFrame(0),
		  mainFrame(0),
		  loadRomFrame(0),
		  fileBrowserFrame(0),
		  currentRomFrame(0),
		  loadSaveFrame(0),
		  saveGameFrame(0),
		  settingsFrame(0),
		  selectCPUFrame(0),
		  configureInputFrame(0),
		  configurePaksFrame(0),
		  configureButtonsFrame(0)
{
	pMenuContext = this;
//	dbg_printf("Initialize MenuContext\r\n");

	menu::Gui::getInstance().setVmode(vmode);
//	dbg_printf("MenuContext - setVmode done\r\n");

	mainFrame = new MainFrame();
//	dbg_printf("MenuContext - new MainFrame\r\n");
	loadRomFrame = new LoadRomFrame();
	fileBrowserFrame = new FileBrowserFrame();
	currentRomFrame = new CurrentRomFrame();
	loadSaveFrame = new LoadSaveFrame();
	saveGameFrame = new SaveGameFrame();
	settingsFrame = new SettingsFrame();
	selectCPUFrame = new SelectCPUFrame();
	configureInputFrame = new ConfigureInputFrame();
	configurePaksFrame = new ConfigurePaksFrame();
	configureButtonsFrame = new ConfigureButtonsFrame();

	menu::Gui::getInstance().addFrame(mainFrame);
//	dbg_printf("MenuContext - add MainFrame\r\n");
	menu::Gui::getInstance().addFrame(loadRomFrame);
	menu::Gui::getInstance().addFrame(fileBrowserFrame);
	menu::Gui::getInstance().addFrame(currentRomFrame);
	menu::Gui::getInstance().addFrame(loadSaveFrame);
	menu::Gui::getInstance().addFrame(saveGameFrame);
	menu::Gui::getInstance().addFrame(settingsFrame);
	menu::Gui::getInstance().addFrame(selectCPUFrame);
	menu::Gui::getInstance().addFrame(configureInputFrame);
	menu::Gui::getInstance().addFrame(configurePaksFrame);
	menu::Gui::getInstance().addFrame(configureButtonsFrame);

	menu::Focus::getInstance().setFocusActive(true);
//	dbg_printf("MenuContext - setFocusActive\r\n");
	setActiveFrame(FRAME_MAIN);
//	dbg_printf("Initialized MenuContext\r\n");
}

MenuContext::~MenuContext()
{
	delete configureButtonsFrame;
	delete configurePaksFrame;
	delete configureInputFrame;
	delete selectCPUFrame;
	delete settingsFrame;
	delete saveGameFrame;
	delete loadSaveFrame;
	delete currentRomFrame;
	delete fileBrowserFrame;
	delete loadRomFrame;
	delete mainFrame;
	pMenuContext = NULL;
}

bool MenuContext::isRunning()
{
	bool isRunning = true;
//	dbg_printf("MenuContext isRunning\r\n");
//	printf("MenuContext isRunning\n");
	draw();

/*	PADStatus* gcPad = menu::Input::getInstance().getPad();
	if(gcPad[0].button & PAD_BUTTON_START)
		isRunning = false;*/
	
	return isRunning;
}

void MenuContext::setActiveFrame(int frameIndex)
{
	if(currentActiveFrame)
		currentActiveFrame->hideFrame();

	switch(frameIndex) {
	case FRAME_MAIN:
		currentActiveFrame = mainFrame;
		break;
	case FRAME_LOADROM:
		currentActiveFrame = loadRomFrame;
		break;
	case FRAME_FILEBROWSER:
		currentActiveFrame = fileBrowserFrame;
		break;
	case FRAME_CURRENTROM:
		currentActiveFrame = currentRomFrame;
		break;
	case FRAME_LOADSAVE:
		currentActiveFrame = loadSaveFrame;
		break;
	case FRAME_SAVEGAME:
		currentActiveFrame = saveGameFrame;
		break;
	case FRAME_SETTINGS:
		currentActiveFrame = settingsFrame;
		break;
	case FRAME_SELECTCPU:
		currentActiveFrame = selectCPUFrame;
		break;
	case FRAME_CONFIGUREINPUT:
		currentActiveFrame = configureInputFrame;
		break;
	case FRAME_CONFIGUREPAKS:
		currentActiveFrame = configurePaksFrame;
		break;
	case FRAME_CONFIGUREBUTTONS:
		currentActiveFrame = configureButtonsFrame;
		break;
	}

	if(currentActiveFrame)
	{
		currentActiveFrame->showFrame();
		menu::Focus::getInstance().setCurrentFrame(currentActiveFrame);
		menu::Cursor::getInstance().setCurrentFrame(currentActiveFrame);
	}
}

void MenuContext::setActiveFrame(int frameIndex, int submenu)
{
	setActiveFrame(frameIndex);
	if(currentActiveFrame) currentActiveFrame->activateSubmenu(submenu);
}

menu::Frame* MenuContext::getFrame(int frameIndex)
{
	menu::Frame* pFrame = NULL;
	switch(frameIndex) {
	case FRAME_MAIN:
		pFrame = mainFrame;
		break;
	case FRAME_LOADROM:
		pFrame = loadRomFrame;
		break;
	case FRAME_FILEBROWSER:
		pFrame = fileBrowserFrame;
		break;
	case FRAME_CURRENTROM:
		pFrame = currentRomFrame;
		break;
	case FRAME_LOADSAVE:
		pFrame = loadSaveFrame;
		break;
	case FRAME_SAVEGAME:
		pFrame = saveGameFrame;
		break;
	case FRAME_SETTINGS:
		pFrame = settingsFrame;
		break;
	case FRAME_SELECTCPU:
		pFrame = selectCPUFrame;
		break;
	case FRAME_CONFIGUREINPUT:
		pFrame = configureInputFrame;
		break;
	case FRAME_CONFIGUREBUTTONS:
		pFrame = configureButtonsFrame;
		break;
	}

	return pFrame;
}

void MenuContext::draw()
{
	menu::Gui::getInstance().draw();
}
