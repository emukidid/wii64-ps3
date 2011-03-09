/**
 * Wii64 - GuiTypes.h
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

#ifndef GUIDEFS_H
#define GUIDEFS_H

#ifdef __GX__
#include <gccore.h>
# ifdef HW_RVL
# include <wiiuse/wpad.h>
# endif
#else //__GX__
#include "../main/winlnxdefs.h"
//#include <ppu-types.h>
#include <io/pad.h>
#include <rsx/rsx.h>
#include <sysutil/video.h>
#include "../main/rsxutil.h"
typedef struct _gx_color {
 	u8 r;			/*!< Red color component. */
 	u8 g;			/*!< Green color component. */
 	u8 b;			/*!< Blue alpha component. */
	u8 a;			/*!< Alpha component. If a function does not use the alpha value, it is safely ignored. */
} GXColor;
#define GXRModeObj void
#define GX_MODULATE			0			/*!< <i>Cv</i>=<i>CrCt</i>; <i>Av</i>=<i>ArAt</i> */
#define GX_REPLACE			3			/*!< <i>Cv=<i>Ct</i>; <i>Ar=<i>At</i> */
#define GX_PASSCLR			4			/*!< <i>Cv=<i>Cr</i>; <i>Av=<i>Ar</i> */
#define GX_FALSE	0
#define GX_TRUE		1
extern s32 globalTextureUnit_id;

#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif
#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif

extern "C" void dbg_printf(const char *fmt,...);

#endif //!__GX__

#include <vector>
#include <stack>
#include <string>
#include <malloc.h>
#include <algorithm>

extern "C" {
#include <string.h>
#include <stdio.h>
}

namespace menu {

class Graphics;
class Component;
class Frame;
class Button;
class Input;
class Cursor;
class Focus;
class Image;
class IplFont;

typedef	std::vector<Frame*> FrameList;
typedef std::vector<Component*> ComponentList;
typedef std::stack<float> FloatStack;
//typedef std::stack<Mtx> MatrixStack;

} //namespace menu 

#endif
