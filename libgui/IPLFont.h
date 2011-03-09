/**
 * Wii64 - IPLFont.h
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

#ifndef IPLFONT_H
#define IPLFONT_H

#include "GuiTypes.h"

#ifndef __GX__
#include <vectormath/cpp/vectormath_aos.h>
using namespace Vectormath::Aos;

#include "combined_shader_vpo.h"
#include "combined_shader_fpo.h"
#endif //!__GX__

namespace menu {

class IplFont
{
public:
	void setVmode(GXRModeObj *rmode);
	void drawInit(GXColor fontColor);
	void setColor(GXColor fontColor);
	void setColor(GXColor* fontColorPtr);
	void drawString(int x, int y, char *string, float scale, bool centered);
	int drawStringWrap(int x, int y, char *string, float scale, bool centered, int maxWidth, int lineSpacing);
	void drawStringAtOrigin(char *string, float scale);
	int getStringWidth(char *string, float scale);
	int getStringHeight(char *string, float scale);
	static IplFont& getInstance()
	{
		static IplFont obj;
		return obj;
	}

private:
	IplFont();
	~IplFont();
	void initFont();
	void decodeYay0(void *src, void *dst);
#ifdef __GX__
	void setIplConfig(unsigned char c);
	void convertI2toI4(void *dst, void *src, int xres, int yres);
#else //__GX__
	void convertI2toI8(void *dst, void *src, int xres, int yres);
#endif //!__GX__

	typedef struct {
		u16 s[256], t[256], font_size[256], fheight;
	} CHAR_INFO;

	u16 frameWidth;
	CHAR_INFO fontChars;
	GXRModeObj *vmode;
	GXColor fontColor;

#ifdef __GX__
#ifdef HW_RVL
	unsigned char *fontFont;
#else //GC
	unsigned char fontFont[ 0x40000 ] __attribute__((aligned(32)));
#endif
	GXTexObj fontTexObj;
#else //__GX__
	gcmTexture texobj;
	u32 rsx_texture_offset;
	u32 *rsx_texture_buffer;

	//Graphics environment variables... Move?
	u32 fpsize;
	u32 fp_offset;
	u32 *fp_buffer;

	s32 projMatrix_id;
	s32 modelViewMatrix_id;
	s32 vertexPosition_id;
	s32 vertexColor0_id;
	s32 vertexTexcoord_id;
	s32 textureUnit_id;
	s32 mode_id;
	f32 shader_mode;

	void *vp_ucode;
	rsxVertexProgram *vpo;
	void *fp_ucode;
	rsxFragmentProgram *fpo;

	Matrix4 projMatrix, modelViewMatrix;
#endif //!__GX__
};

} //namespace menu 

#endif
