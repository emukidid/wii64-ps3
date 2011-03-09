/**
 * Wii64 - Gui.cpp
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

#include "Gui.h"
#include "IPLFont.h"
#include "InputManager.h"
#include "CursorManager.h"
#include "FocusManager.h"
#include "MessageBox.h"
#include "LoadingBar.h"
#include "GuiResources.h"
#include "../main/wii64config.h"
//#include "../source/wii64config.h"

extern "C" {
#include "../gc_input/controller.h"
#ifdef WII
#include <di/di.h>
#endif 
}

extern char shutdownMenu;
extern unsigned int dvd_hard_init;
extern u32 running; //PS3 libgui-demo variable

namespace menu {

Gui::Gui()
	: fade(9)
{
	menuLogo = new Logo();
	menuLogo->setLocation(570.0, 70.0, -150.0);
	menuLogo->setVisible(true);
}

Gui::~Gui()
{
	delete menuLogo;
	delete gfx;
}

void Gui::setVmode(GXRModeObj *vmode)
{
	gfx = new Graphics(vmode);
	IplFont::getInstance().setVmode(vmode);
}

void Gui::addFrame(Frame *frame)
{
	frameList.push_back(frame);
}

void Gui::removeFrame(Frame *frame)
{
	frameList.erase(std::remove(frameList.begin(),frameList.end(),frame),frameList.end());
}

void Gui::draw()
{
//	gfx->setDepth(0.0);
//	dbg_printf("Gui draw\r\n");
//	printf("Gui draw\n");
	Input::getInstance().refreshInput();
	Cursor::getInstance().updateCursor();
	Focus::getInstance().updateFocus();
	if(padAutoAssign) auto_assign_controllers(); //for gc_input
	//Update time??
	//Get graphics framework and pass to Frame draw fns?
	gfx->drawInit();
	drawBackground();
	FrameList::const_iterator iteration;
	for (iteration = frameList.begin(); iteration != frameList.end(); iteration++)
	{
		(*iteration)->updateTime(0.0f); //TODO: Pass deltaTime
		(*iteration)->drawChildren(*gfx);
	}
//	menuLogo->drawComponent(*gfx);
	menuLogo->draw(*gfx);
	if (MessageBox::getInstance().getActive()) MessageBox::getInstance().drawMessageBox(*gfx);
	if (LoadingBar::getInstance().getActive()) LoadingBar::getInstance().drawLoadingBar(*gfx);
	Cursor::getInstance().drawCursor(*gfx);

	if(shutdownMenu)
	{
		Cursor::getInstance().setFreezeAction(true);
		Focus::getInstance().setFreezeAction(true);
		gfx->enableBlending(true);
		gfx->setTEV(GX_PASSCLR);
		gfx->setDepth(-10.0f);
		gfx->newModelView();
		gfx->loadModelView();
		gfx->loadOrthographic();

		gfx->setColor((GXColor){0, 0, 0, fade});
		if(screenMode)	gfx->fillRect(-104, 0, 848, 480);
		else			gfx->fillRect(0, 0, 640, 480);
		
		if(fade == 255)
		{
#ifdef __GX__
			VIDEO_SetBlack(true);
			VIDEO_Flush();
		 	VIDEO_WaitVSync();
			if(shutdownMenu==1)	//Power off System
				SYS_ResetSystem(SYS_POWEROFF, 0, 0);
			else			//Return to Loader
			{
#ifdef WII
        if(dvd_hard_init) {
				  DI_Close();
			  }
#endif
				void (*rld)() = (void (*)()) 0x80001800;
				rld();
			}
#else //__GX__
			//TODO: Shut down gui for PS3...
			running = 0;
#endif //!__GX__
		}

		char increment = 3;
		fade = fade +increment > 255 ? 255 : fade + increment;
	}

	gfx->swapBuffers();
}

void Gui::drawBackground()
{

	//Draw Menu Backdrop
	Resources::getInstance().getImage(Resources::IMAGE_MENU_BACKGROUND)->activateImage(GX_TEXMAP0);
#ifdef __GX__
//	gfx->setTEV(GX_REPLACE);
	GXColor muxCol = (GXColor){0,17,85,255};
	GX_SetTevColor(GX_TEVREG0,muxCol);
	GX_SetTevColorIn(GX_TEVSTAGE0,GX_CC_C0,GX_CC_ZERO,GX_CC_TEXC,GX_CC_TEXC);
	GX_SetTevColorOp(GX_TEVSTAGE0,GX_TEV_ADD,GX_TB_ZERO,GX_CS_SCALE_1,GX_TRUE,GX_TEVPREV);
#else //__GX__
	//TODO: change the combining here or change texture
//	gfx->setTEV(GX_REPLACE);
	gfx->setColor((GXColor){175,190,255,255});
	gfx->setTEV(GX_MODULATE);
#endif //!__GX__
	gfx->enableBlending(false);
	if(screenMode)	gfx->drawImage(0, -104, 0, 848, 480, 0, 1, 0, 1);
	else			gfx->drawImage(0, 0, 0, 640, 480, (848.0-640.0)/2/848.0, 1.0 - (848.0-640.0)/2/848.0, 0, 1);
	gfx->setTEV(GX_PASSCLR);

	//Demo RSX draw code
//	gfx->setTEV(GX_MODULATE);

/*	//init shader:
	u32 fpsize = 0;
	u32 fp_offset;
	u32 *fp_buffer = NULL;

	s32 projMatrix_id = -1;
	s32 modelViewMatrix_id = -1;
	s32 vertexPosition_id = -1;
	s32 vertexColor0_id = -1;
	s32 vertexTexcoord_id = -1;
	s32 textureUnit_id = -1;

	void *vp_ucode = NULL;
//	rsxVertexProgram *vpo = (rsxVertexProgram*)texture_shader_vpo;
	rsxVertexProgram *vpo = (rsxVertexProgram*)modulate_shader_vpo;

	void *fp_ucode = NULL;
//	rsxFragmentProgram *fpo = (rsxFragmentProgram*)texture_shader_fpo;
	rsxFragmentProgram *fpo = (rsxFragmentProgram*)modulate_shader_fpo;

	vp_ucode = rsxVertexProgramGetUCode(vpo);
	projMatrix_id = rsxVertexProgramGetConst(vpo,"projMatrix");
	modelViewMatrix_id = rsxVertexProgramGetConst(vpo,"modelViewMatrix");
	vertexPosition_id = rsxVertexProgramGetAttrib(vpo,"vertexPosition");
	vertexColor0_id = rsxVertexProgramGetAttrib(vpo,"vertexColor");
	vertexTexcoord_id = rsxVertexProgramGetAttrib(vpo,"vertexTexcoord");

	dbg_printf("IDs: P %d, MV %d, Pos %d, Col %d, Tex %d\r\n", projMatrix_id, 
		modelViewMatrix_id, vertexPosition_id, vertexColor0_id, vertexTexcoord_id);


	fp_ucode = rsxFragmentProgramGetUCode(fpo,&fpsize);
	fp_buffer = (u32*)rsxMemalign(64,fpsize);
	memcpy(fp_buffer,fp_ucode,fpsize);
	rsxAddressToOffset(fp_buffer,&fp_offset);

	textureUnit_id = rsxFragmentProgramGetAttrib(fpo,"texture");
*/
/*	int ind = 0;
	for (int j=0; j<480; j++)
	{
		for (int i=0; i<640; i++)
		{
//			FBtex[ind++] = 0x8000 | ((j/10)%0x1f); //Horizontal blue stripes
			FBtex[ind++] = 0x8000 | ((j/10)%0x1f) | (((i/10)%0x1f)<<5); //Horizontal blue+ vertical green stripes
		}
	}

	//setup texture
	u32 width = 640;
	u32 height = 480;
	u32 pitch = (width*2);
	gcmTexture texture;
	u32 texture_offset;
	rsxAddressToOffset(FBtex,&texture_offset);

	rsxInvalidateTextureCache(context,GCM_INVALIDATE_TEXTURE);

	texture.format		= (GCM_TEXTURE_FORMAT_A1R5G5B5 | GCM_TEXTURE_FORMAT_LIN); //CELL_GCM_TEXTURE_R5G5B5A1=(0x97)
	texture.mipmap		= 1;
	texture.dimension	= GCM_TEXTURE_DIMS_2D;
	texture.cubemap		= GCM_FALSE;
	texture.remap		= ((GCM_TEXTURE_REMAP_TYPE_REMAP << GCM_TEXTURE_REMAP_TYPE_B_SHIFT) |
						   (GCM_TEXTURE_REMAP_TYPE_REMAP << GCM_TEXTURE_REMAP_TYPE_G_SHIFT) |
						   (GCM_TEXTURE_REMAP_TYPE_REMAP << GCM_TEXTURE_REMAP_TYPE_R_SHIFT) |
						   (GCM_TEXTURE_REMAP_TYPE_REMAP << GCM_TEXTURE_REMAP_TYPE_A_SHIFT) |
						   (GCM_TEXTURE_REMAP_COLOR_B << GCM_TEXTURE_REMAP_COLOR_B_SHIFT) |
						   (GCM_TEXTURE_REMAP_COLOR_G << GCM_TEXTURE_REMAP_COLOR_G_SHIFT) |
						   (GCM_TEXTURE_REMAP_COLOR_R << GCM_TEXTURE_REMAP_COLOR_R_SHIFT) |
						   (GCM_TEXTURE_REMAP_COLOR_A << GCM_TEXTURE_REMAP_COLOR_A_SHIFT));
	texture.width		= width;
	texture.height		= height;
	texture.depth		= 1;
	texture.location	= GCM_LOCATION_RSX;
	texture.pitch		= pitch;
	texture.offset		= texture_offset;
//	rsxLoadTexture(context,textureUnit_id,&texture);globalTextureUnit_id
//	rsxTextureControl(context,textureUnit_id,GCM_TRUE,0<<8,12<<8,GCM_TEXTURE_MAX_ANISO_1);
//	rsxTextureFilter(context,textureUnit_id,GCM_TEXTURE_LINEAR,GCM_TEXTURE_LINEAR,GCM_TEXTURE_CONVOLUTION_QUINCUNX);
//	rsxTextureWrapMode(context,textureUnit_id,GCM_TEXTURE_CLAMP_TO_EDGE,GCM_TEXTURE_CLAMP_TO_EDGE,GCM_TEXTURE_CLAMP_TO_EDGE,0,GCM_TEXTURE_ZFUNC_LESS,0);
	rsxLoadTexture(context,globalTextureUnit_id,&texture);
	rsxTextureControl(context,globalTextureUnit_id,GCM_TRUE,0<<8,12<<8,GCM_TEXTURE_MAX_ANISO_1);
	rsxTextureFilter(context,globalTextureUnit_id,GCM_TEXTURE_LINEAR,GCM_TEXTURE_LINEAR,GCM_TEXTURE_CONVOLUTION_QUINCUNX);
	rsxTextureWrapMode(context,globalTextureUnit_id,GCM_TEXTURE_CLAMP_TO_EDGE,GCM_TEXTURE_CLAMP_TO_EDGE,GCM_TEXTURE_CLAMP_TO_EDGE,0,GCM_TEXTURE_ZFUNC_LESS,0);

	gfx->setDepth(-10.0f);

	gfx->setTEV(GX_REPLACE);
	gfx->drawImage(0, 0, 0, 50, 100, 0.0f, 1.0f, 0.0f, 1.0f);
	
	Resources::getInstance().getImage(Resources::IMAGE_DEFAULT_BUTTON)->activateImage(GX_TEXMAP0);
	gfx->drawImage(0, 50, 0, 50, 100, 0.0f, 1.0f, 0.0f, 1.0f);

	Resources::getInstance().getImage(Resources::IMAGE_DEFAULT_BUTTONFOCUS)->activateImage(GX_TEXMAP0);
	gfx->drawImage(0, 100, 0, 50, 100, 0.0f, 1.0f, 0.0f, 1.0f);

	Resources::getInstance().getImage(Resources::IMAGE_STYLEA_BUTTONSELECTOFF)->activateImage(GX_TEXMAP0);
	gfx->drawImage(0, 150, 0, 50, 100, 0.0f, 1.0f, 0.0f, 1.0f);

	Resources::getInstance().getImage(Resources::IMAGE_STYLEA_BUTTONSELECTOFFFOCUS)->activateImage(GX_TEXMAP0);
	gfx->drawImage(0, 200, 0, 50, 100, 0.0f, 1.0f, 0.0f, 1.0f);

	Resources::getInstance().getImage(Resources::IMAGE_STYLEA_BUTTONSELECTON)->activateImage(GX_TEXMAP0);
	gfx->drawImage(0, 250, 0, 50, 100, 0.0f, 1.0f, 0.0f, 1.0f);

	Resources::getInstance().getImage(Resources::IMAGE_STYLEA_BUTTONSELECTONFOCUS)->activateImage(GX_TEXMAP0);
	gfx->drawImage(0, 300, 0, 50, 100, 0.0f, 1.0f, 0.0f, 1.0f);

	Resources::getInstance().getImage(Resources::IMAGE_MENU_BACKGROUND)->activateImage(GX_TEXMAP0);
	gfx->drawImage(0, 350, 0, 50, 100, 0.0f, 1.0f, 0.0f, 1.0f);

	Resources::getInstance().getImage(Resources::IMAGE_LOGO)->activateImage(GX_TEXMAP0);
	gfx->drawImage(0, 400, 0, 50, 100, 0.0f, 1.0f, 0.0f, 1.0f);

	Resources::getInstance().getImage(Resources::IMAGE_CONTROLLER_EMPTY)->activateImage(GX_TEXMAP0);
	gfx->drawImage(0, 450, 0, 50, 100, 0.0f, 1.0f, 0.0f, 1.0f);

	Resources::getInstance().getImage(Resources::IMAGE_CONTROLLER_GAMECUBE)->activateImage(GX_TEXMAP0);
	gfx->drawImage(0, 500, 0, 50, 100, 0.0f, 1.0f, 0.0f, 1.0f);

	Resources::getInstance().getImage(Resources::IMAGE_CONTROLLER_CLASSIC)->activateImage(GX_TEXMAP0);
	gfx->drawImage(0, 550, 0, 50, 100, 0.0f, 1.0f, 0.0f, 1.0f);

	Resources::getInstance().getImage(Resources::IMAGE_CONTROLLER_WIIMOTENUNCHUCK)->activateImage(GX_TEXMAP0);
	gfx->drawImage(0, 0, 100, 50, 100, 0.0f, 1.0f, 0.0f, 1.0f);

	Resources::getInstance().getImage(Resources::IMAGE_CONTROLLER_WIIMOTE)->activateImage(GX_TEXMAP0);
	gfx->drawImage(0, 50, 100, 50, 100, 0.0f, 1.0f, 0.0f, 1.0f);

	Resources::getInstance().getImage(Resources::IMAGE_N64_CONTROLLER)->activateImage(GX_TEXMAP0);
	gfx->drawImage(0, 100, 100, 50, 100, 0.0f, 1.0f, 0.0f, 1.0f);

	//Try drawing a button
	Resources::getInstance().getImage(Resources::IMAGE_STYLEA_BUTTONSELECTONFOCUS)->activateImage(GX_TEXMAP0);
	gfx->drawImage(0, 50, 200, 100, 100, 0.0f, 2.0f, 0.0f, 1.0f);
	gfx->drawImage(0, 160, 200, 100, 100, 2.0f, 0.0f, 0.0f, 1.0f);

	gfx->setTEV(GX_MODULATE);
	Resources::getInstance().getImage(Resources::IMAGE_LOGO)->activateImage(GX_TEXMAP0);
	gfx->testPrim();
*/
/*	IMAGE_DEFAULT_BUTTON=1,
		IMAGE_DEFAULT_BUTTONFOCUS,
		IMAGE_STYLEA_BUTTON,
		IMAGE_STYLEA_BUTTONFOCUS,
		IMAGE_STYLEA_BUTTONSELECTOFF,
		IMAGE_STYLEA_BUTTONSELECTOFFFOCUS,
		IMAGE_STYLEA_BUTTONSELECTON,
		IMAGE_STYLEA_BUTTONSELECTONFOCUS,
		IMAGE_MENU_BACKGROUND,
		IMAGE_LOGO,
		IMAGE_CONTROLLER_EMPTY,
		IMAGE_CONTROLLER_GAMECUBE,
		IMAGE_CONTROLLER_CLASSIC,
		IMAGE_CONTROLLER_WIIMOTENUNCHUCK,
		IMAGE_CONTROLLER_WIIMOTE,
		IMAGE_N64_CONTROLLER
*/
	/*
	//setup draw environment:
	rsxSetColorMask(context,GCM_COLOR_MASK_B |
							GCM_COLOR_MASK_G |
							GCM_COLOR_MASK_R |
							GCM_COLOR_MASK_A);

	rsxSetColorMaskMRT(context,0);

	u16 x,y,w,h;
	f32 min, max;
	f32 scale[4],offset[4];

	x = 0;
	y = 0;
	w = display_width;
	h = display_height;
	min = 0.0f;
	max = 1.0f;
	scale[0] = w*0.5f;
	scale[1] = h*-0.5f;
	scale[2] = (max - min)*0.5f;
	scale[3] = 0.0f;
	offset[0] = x + w*0.5f;
	offset[1] = y + h*0.5f;
	offset[2] = (max + min)*0.5f;
	offset[3] = 0.0f;

	rsxSetViewport(context,x, y, w, h, min, max, scale, offset);
	rsxSetScissor(context,x,y,w,h);

	rsxSetDepthTestEnable(context,GCM_FALSE);
	rsxSetDepthFunc(context,GCM_LESS);
	rsxSetShadeModel(context,GCM_SHADE_MODEL_SMOOTH);
	rsxSetDepthWriteEnable(context,1);
	rsxSetFrontFace(context,GCM_FRONTFACE_CCW);

	//inline const Matrix4 Matrix4::orthographic( float left, float right, float bottom, float top, float zNear, float zFar )
	Matrix4 P,viewMatrix,modelMatrix,modelViewMatrix;
	Point3 eye_pos = Point3(0.0f,0.0f,20.0f);
	Point3 eye_dir = Point3(0.0f,0.0f,0.0f);
	Vector3 up_vec = Vector3(0.0f,1.0f,0.0f);

	P = transpose(Matrix4::orthographic(0.0f, 640.0f, 0.0f, 480.0f, 0.0f, 1.0f ));
	viewMatrix = Matrix4::lookAt(eye_pos,eye_dir,up_vec);
	modelMatrix = Matrix4::identity();
	modelViewMatrix = viewMatrix*modelMatrix;

	u32 color = 0;
	rsxSetClearColor(context,color);
	rsxSetClearDepthValue(context,0xffff);
	rsxClearSurface(context,GCM_CLEAR_R |
							GCM_CLEAR_G |
							GCM_CLEAR_B |
							GCM_CLEAR_A |
							GCM_CLEAR_S |
							GCM_CLEAR_Z);

	rsxZControl(context,0,1,1);

	for(int i=0;i<8;i++)
		rsxSetViewportClip(context,i,display_width,display_height);
*/
//	gfx->setTEV(GX_MODULATE);
/*
	rsxLoadVertexProgram(context,vpo,vp_ucode);
	rsxSetVertexProgramParameter(context,vpo,projMatrix_id,(float*)&P);
	rsxSetVertexProgramParameter(context,vpo,modelViewMatrix_id,(float*)&modelViewMatrix);

	rsxLoadFragmentProgramLocation(context,fpo,fp_offset,GCM_LOCATION_RSX);
*/
/*	rsxSetUserClipPlaneControl(context,GCM_USER_CLIP_PLANE_DISABLE,
									   GCM_USER_CLIP_PLANE_DISABLE,
									   GCM_USER_CLIP_PLANE_DISABLE,
									   GCM_USER_CLIP_PLANE_DISABLE,
									   GCM_USER_CLIP_PLANE_DISABLE,
									   GCM_USER_CLIP_PLANE_DISABLE);*/
/*
	//void rsxDrawVertexBegin(gcmContextData *context,u32 type);
	rsxDrawVertexBegin(context,GCM_TYPE_QUADS);

		rsxDrawVertex3f(context, vertexPosition_id, 0.0f, 0.0f, 0.0f);
		rsxDrawVertex4f(context, vertexColor0_id, 1.0f, 1.0f, 1.0f, 1.0f);
		rsxDrawVertex2f(context, vertexTexcoord_id, 1.0f, 1.0f);

		rsxDrawVertex3f(context, vertexPosition_id, 640.0f, 0.0f, 0.0f);
		rsxDrawVertex4f(context, vertexColor0_id, 1.0f, 1.0f, 1.0f, 1.0f);
		rsxDrawVertex2f(context, vertexTexcoord_id, 1.0f, 0.0f);

		rsxDrawVertex3f(context, vertexPosition_id, 640.0f, 480.0f, 0.0f);
		rsxDrawVertex4f(context, vertexColor0_id, 0.0f, 0.0f, 0.0f, 1.0f);
		rsxDrawVertex2f(context, vertexTexcoord_id, 0.0f, 0.0f);

		rsxDrawVertex3f(context, vertexPosition_id, 0.0f, 480.0f, 0.0f);
		rsxDrawVertex4f(context, vertexColor0_id, 0.0f, 0.0f, 0.0f, 1.0f);
		rsxDrawVertex2f(context, vertexTexcoord_id, 0.0f, 1.0f);
	rsxDrawVertexEnd(context);
///
	rsxDrawVertexBegin(context,GCM_TYPE_QUADS);
		rsxDrawVertex3f(context, vertexPosition_id, 0.0f, 0.0f, 0.0f);
		rsxDrawVertex4f(context, vertexColor0_id, 1.0f, 1.0f, 1.0f, 1.0f);
		rsxDrawVertex2f(context, vertexTexcoord_id, 1.0f, 1.0f);

		rsxDrawVertex3f(context, vertexPosition_id, 320.0f, 0.0f, 0.0f);
		rsxDrawVertex4f(context, vertexColor0_id, 1.0f, 1.0f, 1.0f, 1.0f);
		rsxDrawVertex2f(context, vertexTexcoord_id, 1.0f, 0.0f);

		rsxDrawVertex3f(context, vertexPosition_id, 320.0f, 240.0f, 0.0f);
		rsxDrawVertex4f(context, vertexColor0_id, 0.0f, 0.0f, 0.0f, 1.0f);
		rsxDrawVertex2f(context, vertexTexcoord_id, 0.0f, 0.0f);

		rsxDrawVertex3f(context, vertexPosition_id, 0.0f, 240.0f, 0.0f);
		rsxDrawVertex4f(context, vertexColor0_id, 0.0f, 0.0f, 0.0f, 1.0f);
		rsxDrawVertex2f(context, vertexTexcoord_id, 0.0f, 1.0f);

	rsxDrawVertexEnd(context);*/
//	gfx->fillRect(50, 0, 100, 100);
//	gfx->fillRect(50, 0, 100, 100);
//void Graphics::drawImage(int textureId, int x, int y, int width, int height, float s1, float s2, float t1, float t2)
/*	gfx->setTEV(GX_MODULATE);
	gfx->setDepth(0);
	gfx->setColor((GXColor) {255,0,255,255});
	gfx->drawImage(0, 0, 0, 100, 240, 0.0f, 1.0f, 0.0f, 1.0f);
//	gfx->drawImage(0, 0, 0, 320, 240, 0.0f, 1.0f, 0.0f, 1.0f);
	gfx->setTEV(GX_REPLACE);
	gfx->drawImage(0, 100, 0, 100, 240, 0.0f, 1.0f, 0.0f, 1.0f);
	gfx->setTEV(GX_PASSCLR);
	gfx->drawImage(0, 200, 0, 100, 240, 0.0f, 1.0f, 0.0f, 1.0f);
*/
//	flip();

	//free RSX buffers
//	if (fp_buffer) rsxFree(fp_buffer);

}

} //namespace menu 
