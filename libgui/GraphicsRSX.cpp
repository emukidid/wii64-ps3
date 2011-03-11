/**
 * Wii64 - GraphicsGX.cpp
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

#include <math.h>
#include "GraphicsRSX.h"
#include "../main/wii64config.h"
#ifdef HW_RVL
#include "../gc_memory/MEM2.h"
#endif

#define DEFAULT_FIFO_SIZE		(256 * 1024)

//TODO: fix Image to not need this
s32 globalTextureUnit_id;

extern "C" unsigned int usleep(unsigned int us);
void video_mode_init(GXRModeObj *rmode, unsigned int *fb1, unsigned int *fb2);

namespace menu {

Graphics::Graphics(GXRModeObj *rmode)
		: which_fb(0),
		  first_frame(true),
		  depth(1.0f),
		  transparency(1.0f),
		  viewportWidth(640.0f),
		  viewportHeight(480.0f)
{
//	printf("Graphics constructor\n");

	fp_buffer = NULL;
	shader_mode = SHADER_PASSCOLOR;

	setColor((GXColor) {0,0,0,0});
/*
#ifdef HW_RVL
	CONF_Init();
#endif
	VIDEO_Init();
	//vmode = VIDEO_GetPreferredMode(NULL);
	vmode = VIDEO_GetPreferredMode(&vmode_phys);
#if 0
	if(CONF_GetAspectRatio()) {
		vmode->viWidth = 678;
		vmode->viXOrigin = (VI_MAX_WIDTH_PAL - 678) / 2;
	}
#endif
	if (memcmp( &vmode_phys, &TVPal528IntDf, sizeof(GXRModeObj)) == 0)
		memcpy( &vmode_phys, &TVPal574IntDfScale, sizeof(GXRModeObj));

	//vmode->efbHeight = viewportHeight; // Note: all possible modes have efbHeight of 480

	VIDEO_Configure(vmode);

#ifdef MEM2XFB
	xfb[0] = XFB0_LO;
	xfb[1] = XFB1_LO;
#else
	xfb[0] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(vmode));
	xfb[1] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(vmode));
#endif

	console_init (xfb[0], 20, 64, vmode->fbWidth, vmode->xfbHeight, vmode->fbWidth * 2);

	VIDEO_SetNextFramebuffer(xfb[which_fb]);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if(vmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();
	which_fb ^= 1;

	//Pass vmode, xfb[0] and xfb[1] back to main program
	video_mode_init(vmode, (unsigned int*)xfb[0], (unsigned int*)xfb[1]);

	//Perform GX init stuff here?
	//GX_init here or in main?
	//GX_SetViewport( 0.0f, 0.0f, viewportWidth, viewportHeight );
*/	init();
	
}

Graphics::~Graphics()
{
	if (fp_buffer)
		rsxFree(fp_buffer);
}

void Graphics::init()
{
	//Setup textures, arrays, matrices...
//	f32 aspect_ratio = 4.0f/3.0f;

	fpsize = 0;
	projMatrix_id = -1;
	modelViewMatrix_id = -1;
	vertexPosition_id = -1;
	vertexColor0_id = -1;
	vertexTexcoord_id = -1;
	textureUnit_id = -1;
	mode_id = -1;
	vp_ucode = NULL;
	fp_ucode = NULL;

	Point3 eye_pos = Point3(0.0f,0.0f,20.0f);
	Point3 eye_dir = Point3(0.0f,0.0f,0.0f);
	Vector3 up_vec = Vector3(0.0f,1.0f,0.0f);

//	modelViewMatrix = Matrix4::lookAt(eye_pos,eye_dir,up_vec) * Matrix4::identity();
//	modelViewMatrix = Matrix4::lookAt(eye_pos,eye_dir,up_vec);
	modelViewMatrix = Matrix4::identity();
	projMatrix = transpose(Matrix4::orthographic(0.0f, 640.0f, 480.0f, 0.0f, 0.0f, 700.0f ));
//	projMatrix = transpose(Matrix4::orthographic(0.0f, 640.0f, 0.0f, 480.0f, 0.0f, 700.0f ));

	vpo = (rsxVertexProgram*)combined_shader_vpo;
	fpo = (rsxFragmentProgram*)combined_shader_fpo;

	vp_ucode = rsxVertexProgramGetUCode(vpo);
	projMatrix_id = rsxVertexProgramGetConst(vpo,"projMatrix");
	modelViewMatrix_id = rsxVertexProgramGetConst(vpo,"modelViewMatrix");
	vertexPosition_id = rsxVertexProgramGetAttrib(vpo,"vertexPosition");
	vertexColor0_id = rsxVertexProgramGetAttrib(vpo,"vertexColor");
	vertexTexcoord_id = rsxVertexProgramGetAttrib(vpo,"vertexTexcoord");

	fp_ucode = rsxFragmentProgramGetUCode(fpo,&fpsize);
	fp_buffer = (u32*)rsxMemalign(64,fpsize);
	memcpy(fp_buffer,fp_ucode,fpsize);
	rsxAddressToOffset(fp_buffer,&fp_offset);

	mode_id = rsxFragmentProgramGetConst(fpo,"mode");
	textureUnit_id = rsxFragmentProgramGetAttrib(fpo,"texture");
	globalTextureUnit_id = textureUnit_id;



/*	if(screenMode)	guOrtho(currentProjectionMtx, 0, 479, -104, 743, 0, 700);
	else			guOrtho(currentProjectionMtx, 0, 479, 0, 639, 0, 700);
*/
/*
	f32 yscale;
	u32 xfbHeight;
	void *gpfifo = NULL;
	GXColor background = {0, 0, 0, 0xff};

	gpfifo = memalign(32,DEFAULT_FIFO_SIZE);
	memset(gpfifo,0,DEFAULT_FIFO_SIZE);
	GX_Init(gpfifo,DEFAULT_FIFO_SIZE);
	GX_SetCopyClear(background, GX_MAX_Z24);

	GX_SetViewport(0,0,vmode->fbWidth,vmode->efbHeight,0,1);
	yscale = GX_GetYScaleFactor(vmode->efbHeight,vmode->xfbHeight);
	xfbHeight = GX_SetDispCopyYScale(yscale);
	GX_SetScissor(0,0,vmode->fbWidth,vmode->efbHeight);
	GX_SetDispCopySrc(0,0,vmode->fbWidth,vmode->efbHeight);
	GX_SetDispCopyDst(vmode->fbWidth,xfbHeight);
	GX_SetCopyFilter(vmode->aa,vmode->sample_pattern,GX_TRUE,vmode->vfilter);
	GX_SetFieldMode(vmode->field_rendering,((vmode->viHeight==2*vmode->xfbHeight)?GX_ENABLE:GX_DISABLE));
 
	if (vmode->aa)
        GX_SetPixelFmt(GX_PF_RGB565_Z16, GX_ZC_LINEAR);
    else
        GX_SetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);

	GX_SetCullMode(GX_CULL_NONE);
	GX_SetDispCopyGamma(GX_GM_1_0);

	GX_InvVtxCache();
	GX_InvalidateTexAll();

	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

	GX_ClearVtxDesc();
	GX_SetVtxDesc(GX_VA_PTNMTXIDX, GX_PNMTX0);
	GX_SetVtxDesc(GX_VA_TEX0MTXIDX, GX_TEXMTX0);
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);

	setTEV(GX_PASSCLR);
	newModelView();
	loadModelView();
	loadOrthographic();
	*/
}

void Graphics::drawInit()
{
	//dbg_printf("Graphics drawInit\r\n");
	//init_shader();
	setTEV(GX_PASSCLR);


	rsxInvalidateTextureCache(context,GCM_INVALIDATE_TEXTURE);

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

	rsxSetDepthTestEnable(context,GCM_TRUE);
	rsxSetDepthFunc(context,GCM_ALWAYS);//GCM_GEQUAL);//GCM_LESS
	rsxSetShadeModel(context,GCM_SHADE_MODEL_SMOOTH);
	rsxSetDepthWriteEnable(context,1);
	rsxSetFrontFace(context,GCM_FRONTFACE_CCW);
	rsxSetCullFace(context,GCM_CULL_BACK);
	rsxSetCullFaceEnable(context,GCM_FALSE);

	//Clear color buffer
	u32 color = 0;
	rsxSetClearColor(context,color);
//	rsxSetClearDepthValue(context,0xffff);
	rsxSetClearDepthValue(context,0x0);
	rsxClearSurface(context,GCM_CLEAR_R |
							GCM_CLEAR_G |
							GCM_CLEAR_B |
							GCM_CLEAR_A |
							GCM_CLEAR_S |
							GCM_CLEAR_Z);

	rsxZControl(context,0,1,1);

	for(int i=0;i<8;i++)
		rsxSetViewportClip(context,i,display_width,display_height);

	rsxSetUserClipPlaneControl(context,GCM_USER_CLIP_PLANE_DISABLE,
									   GCM_USER_CLIP_PLANE_DISABLE,
									   GCM_USER_CLIP_PLANE_DISABLE,
									   GCM_USER_CLIP_PLANE_DISABLE,
									   GCM_USER_CLIP_PLANE_DISABLE,
									   GCM_USER_CLIP_PLANE_DISABLE);

	//Turn off Blending
	rsxSetBlendEnable(context, GCM_FALSE);
	rsxSetBlendFunc(context, GCM_ONE, GCM_ZERO, GCM_ONE, GCM_ZERO);

	//Load Vertex and Fragment Programs
	rsxLoadVertexProgram(context,vpo,vp_ucode);
	rsxSetVertexProgramParameter(context,vpo,projMatrix_id,(float*)&projMatrix);
	rsxSetVertexProgramParameter(context,vpo,modelViewMatrix_id,(float*)&modelViewMatrix);

	rsxSetFragmentProgramParameter(context,fpo,mode_id,&shader_mode,fp_offset);
	rsxLoadFragmentProgramLocation(context,fpo,fp_offset,GCM_LOCATION_RSX);


/*	// Reset various parameters from gfx plugin
	GX_SetZTexture(GX_ZT_DISABLE,GX_TF_Z16,0);	//GX_ZT_DISABLE or GX_ZT_REPLACE; set in gDP.cpp
	GX_SetZCompLoc(GX_TRUE);	// Do Z-compare before texturing.
	GX_SetFog(GX_FOG_NONE,0,1,0,1,(GXColor){0,0,0,255});
	GX_SetViewport(0,0,vmode->fbWidth,vmode->efbHeight,0,1);
	GX_SetCoPlanar(GX_DISABLE);
	GX_SetClipMode(GX_CLIP_ENABLE);
	GX_SetScissor(0,0,vmode->fbWidth,vmode->efbHeight);
	GX_SetAlphaCompare(GX_ALWAYS,0,GX_AOP_AND,GX_ALWAYS,0);

	GX_SetZMode(GX_ENABLE,GX_ALWAYS,GX_TRUE);

	//set blend mode
	GX_SetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR); //Fix src alpha
	GX_SetColorUpdate(GX_ENABLE);
	GX_SetAlphaUpdate(GX_ENABLE);
	GX_SetDstAlpha(GX_DISABLE, 0xFF);
	//set cull mode
	GX_SetCullMode (GX_CULL_NONE);

	GX_InvVtxCache();
	GX_InvalidateTexAll();

	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

	GX_ClearVtxDesc();
	GX_SetVtxDesc(GX_VA_PTNMTXIDX, GX_PNMTX0);
	GX_SetVtxDesc(GX_VA_TEX0MTXIDX, GX_TEXMTX0);
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);

	setTEV(GX_PASSCLR);
	newModelView();
	loadModelView();
	loadOrthographic();
*/
}

void Graphics::swapBuffers()
{
	//dbg_printf("Graphics swapBuffers\r\n");

	flip();
/*
//	printf("Graphics swapBuffers\n");
//	if(which_fb==1) usleep(1000000);
	GX_SetCopyClear((GXColor){0, 0, 0, 0xFF}, GX_MAX_Z24);
	GX_CopyDisp(xfb[which_fb],GX_TRUE);
	GX_Flush();

	VIDEO_SetNextFramebuffer(xfb[which_fb]);
	if(first_frame) {
		first_frame = false;
		VIDEO_SetBlack(GX_FALSE);
	}
	VIDEO_Flush();
 	VIDEO_WaitVSync();
	which_fb ^= 1;
//	printf("Graphics endSwapBuffers\n");
*/
}

void Graphics::clearEFB(GXColor color, u32 zvalue)
{
	//Clear color buffer
	u32 color32 = ((u32)color.r<<24)|((u32)color.g<<16)|((u32)color.b<<16)|(u32)color.a;
	rsxSetClearColor(context,color32);
	rsxSetClearDepthValue(context,0xffff);
	rsxClearSurface(context,GCM_CLEAR_R |
							GCM_CLEAR_G |
							GCM_CLEAR_B |
							GCM_CLEAR_A |
							GCM_CLEAR_S |
							GCM_CLEAR_Z);


/*	GX_SetColorUpdate(GX_ENABLE);
	GX_SetAlphaUpdate(GX_ENABLE);
	GX_SetZMode(GX_ENABLE,GX_ALWAYS,GX_TRUE);
	GX_SetCopyClear(color, zvalue);
	GX_CopyDisp(xfb[which_fb],GX_TRUE);
	GX_Flush();
	*/
}

void Graphics::newModelView()
{
	
//	guMtxIdentity(currentModelViewMtx);
}

void Graphics::translate(float x, float y, float z)
{
/*	Mtx tmp;
	guMtxTrans (tmp, x, y, z);
	guMtxConcat (currentModelViewMtx, tmp, currentModelViewMtx);*/
}

void Graphics::translateApply(float x, float y, float z)
{
//	guMtxTransApply(currentModelViewMtx,currentModelViewMtx,x,y,z);
}

void Graphics::rotate(float degrees)
{
//	guMtxRotDeg(currentModelViewMtx,'Z',degrees);
}

void Graphics::loadModelView()
{
//	GX_LoadPosMtxImm(currentModelViewMtx,GX_PNMTX0);
}

void Graphics::loadOrthographic()
{
/*	if(screenMode)	guOrtho(currentProjectionMtx, 0, 479, -104, 743, 0, 700);
	else			guOrtho(currentProjectionMtx, 0, 479, 0, 639, 0, 700);
	GX_LoadProjectionMtx(currentProjectionMtx, GX_ORTHOGRAPHIC);*/
}

void Graphics::setDepth(float newDepth)
{
	depth = newDepth;
}

float Graphics::getDepth()
{
	return depth;
}

void Graphics::setColor(GXColor color)
{
	for (int i = 0; i < 4; i++){
		currentColor[i].r = color.r;
		currentColor[i].g = color.g;
		currentColor[i].b = color.b;
		currentColor[i].a = color.a;
	}
	applyCurrentColor();
}

void Graphics::setColor(GXColor* color)
{
	for (int i = 0; i < 4; i++){
		currentColor[i].r = color[i].r;
		currentColor[i].g = color[i].g;
		currentColor[i].b = color[i].b;
		currentColor[i].a = color[i].a;
	}
	applyCurrentColor();
}

void Graphics::drawRect(int x, int y, int width, int height)
{
	rsxDrawVertexBegin(context,GCM_TYPE_LINE_STRIP);
		rsxDrawVertex4f(context, vertexColor0_id, (float)appliedColor[0].r/255.0f, (float)appliedColor[0].g/255.0f, 
			(float)appliedColor[0].b/255.0f, (float)appliedColor[0].a/255.0f);
		rsxDrawVertex2f(context, vertexTexcoord_id, 0.0f,0.0f);
		rsxDrawVertex4f(context, vertexPosition_id, (float) x,(float) y, depth, 1.0f);

		rsxDrawVertex4f(context, vertexColor0_id, (float)appliedColor[1].r/255.0f, (float)appliedColor[1].g/255.0f, 
			(float)appliedColor[1].b/255.0f, (float)appliedColor[1].a/255.0f);
		rsxDrawVertex2f(context, vertexTexcoord_id, 0.0f,0.0f);
		rsxDrawVertex4f(context, vertexPosition_id, (float) (x+width),(float) y, depth, 1.0f);

		rsxDrawVertex4f(context, vertexColor0_id, (float)appliedColor[2].r/255.0f, (float)appliedColor[2].g/255.0f, 
			(float)appliedColor[2].b/255.0f, (float)appliedColor[2].a/255.0f);
		rsxDrawVertex2f(context, vertexTexcoord_id, 0.0f,0.0f);
		rsxDrawVertex4f(context, vertexPosition_id, (float) (x+width),(float) (y+height), depth, 1.0f);

		rsxDrawVertex4f(context, vertexColor0_id, (float)appliedColor[3].r/255.0f, (float)appliedColor[3].g/255.0f, 
			(float)appliedColor[3].b/255.0f, (float)appliedColor[3].a/255.0f);
		rsxDrawVertex2f(context, vertexTexcoord_id, 0.0f,0.0f);
		rsxDrawVertex4f(context, vertexPosition_id, (float) x,(float) (y+height), depth, 1.0f);
	rsxDrawVertexEnd(context);

/*	GX_Begin(GX_LINESTRIP, GX_VTXFMT0, 4);
		GX_Position3f32((float) x,(float) y, depth );
		GX_Color4u8(appliedColor[0].r, appliedColor[0].g, appliedColor[0].b, appliedColor[0].a);
		GX_TexCoord2f32(0.0f,0.0f);
		GX_Position3f32((float) (x+width),(float) y, depth );
		GX_Color4u8(appliedColor[1].r, appliedColor[1].g, appliedColor[1].b, appliedColor[1].a);
		GX_TexCoord2f32(0.0f,0.0f);
		GX_Position3f32((float) (x+width),(float) (y+height), depth );
		GX_Color4u8(appliedColor[2].r, appliedColor[2].g, appliedColor[2].b, appliedColor[2].a);
		GX_TexCoord2f32(0.0f,0.0f);
		GX_Position3f32((float) x,(float) (y+height), depth );
		GX_Color4u8(appliedColor[3].r, appliedColor[3].g, appliedColor[3].b, appliedColor[3].a);
		GX_TexCoord2f32(0.0f,0.0f);
	GX_End();*/
}

void Graphics::fillRect(int x, int y, int width, int height)
{
//	dbg_printf("fillRect x %d, y %d, wd %d, ht %d, dpth %d, Col %d, %d, %d, %d\r\n", x, y, width, height, depth, 
//		appliedColor[0].r, appliedColor[0].g, appliedColor[0].b, appliedColor[0].a);

	rsxDrawVertexBegin(context,GCM_TYPE_QUADS);
		rsxDrawVertex4f(context, vertexColor0_id, (float)appliedColor[0].r/255.0f, (float)appliedColor[0].g/255.0f, 
			(float)appliedColor[0].b/255.0f, (float)appliedColor[0].a/255.0f);
		rsxDrawVertex2f(context, vertexTexcoord_id, 0.0f,0.0f);
		rsxDrawVertex4f(context, vertexPosition_id, (float) x,(float) y, depth, 1.0f);

		rsxDrawVertex4f(context, vertexColor0_id, (float)appliedColor[1].r/255.0f, (float)appliedColor[1].g/255.0f, 
			(float)appliedColor[1].b/255.0f, (float)appliedColor[1].a/255.0f);
		rsxDrawVertex2f(context, vertexTexcoord_id, 0.0f,0.0f);
		rsxDrawVertex4f(context, vertexPosition_id, (float) (x+width),(float) y, depth, 1.0f);

		rsxDrawVertex4f(context, vertexColor0_id, (float)appliedColor[2].r/255.0f, (float)appliedColor[2].g/255.0f, 
			(float)appliedColor[2].b/255.0f, (float)appliedColor[2].a/255.0f);
		rsxDrawVertex2f(context, vertexTexcoord_id, 0.0f,0.0f);
		rsxDrawVertex4f(context, vertexPosition_id, (float) (x+width),(float) (y+height), depth, 1.0f);

		rsxDrawVertex4f(context, vertexColor0_id, (float)appliedColor[3].r/255.0f, (float)appliedColor[3].g/255.0f, 
			(float)appliedColor[3].b/255.0f, (float)appliedColor[3].a/255.0f);
		rsxDrawVertex2f(context, vertexTexcoord_id, 0.0f,0.0f);
		rsxDrawVertex4f(context, vertexPosition_id, (float) x,(float) (y+height), depth, 1.0f);
	rsxDrawVertexEnd(context);

/*		rsxDrawVertex3f(context, vertexPosition_id, 0.0f, 0.0f, 0.0f);
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
*/
/*	GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
		GX_Position3f32((float) x,(float) y, depth );
		GX_Color4u8(appliedColor[0].r, appliedColor[0].g, appliedColor[0].b, appliedColor[0].a);
		GX_TexCoord2f32(0.0f,0.0f);
		GX_Position3f32((float) (x+width),(float) y, depth );
		GX_Color4u8(appliedColor[1].r, appliedColor[1].g, appliedColor[1].b, appliedColor[1].a);
		GX_TexCoord2f32(0.0f,0.0f);
		GX_Position3f32((float) (x+width),(float) (y+height), depth );
		GX_Color4u8(appliedColor[2].r, appliedColor[2].g, appliedColor[2].b, appliedColor[2].a);
		GX_TexCoord2f32(0.0f,0.0f);
		GX_Position3f32((float) x,(float) (y+height), depth );
		GX_Color4u8(appliedColor[3].r, appliedColor[3].g, appliedColor[3].b, appliedColor[3].a);
		GX_TexCoord2f32(0.0f,0.0f);
	GX_End();*/
}

void Graphics::drawImage(int textureId, int x, int y, int width, int height, float s1, float s2, float t1, float t2)
{
	//input position and tex coords are measured from top left of screen/texture
	rsxDrawVertexBegin(context,GCM_TYPE_QUADS);
		rsxDrawVertex4f(context, vertexColor0_id, (float)appliedColor[0].r/255.0f, (float)appliedColor[0].g/255.0f, 
			(float)appliedColor[0].b/255.0f, (float)appliedColor[0].a/255.0f);
		rsxDrawVertex2f(context, vertexTexcoord_id, s1, t1);
//		rsxDrawVertex2f(context, vertexTexcoord_id, s2, t1);
//		rsxDrawVertex2f(context, vertexTexcoord_id, s1, t1);
		rsxDrawVertex4f(context, vertexPosition_id, (float) x,(float) y, depth, 1.0f);

		rsxDrawVertex4f(context, vertexColor0_id, (float)appliedColor[1].r/255.0f, (float)appliedColor[1].g/255.0f, 
			(float)appliedColor[1].b/255.0f, (float)appliedColor[1].a/255.0f);
		rsxDrawVertex2f(context, vertexTexcoord_id, s2, t1);
//		rsxDrawVertex2f(context, vertexTexcoord_id, s2, t2);
//		rsxDrawVertex2f(context, vertexTexcoord_id, s2, t1);
		rsxDrawVertex4f(context, vertexPosition_id, (float) (x+width),(float) y, depth, 1.0f);

		rsxDrawVertex4f(context, vertexColor0_id, (float)appliedColor[2].r/255.0f, (float)appliedColor[2].g/255.0f, 
			(float)appliedColor[2].b/255.0f, (float)appliedColor[2].a/255.0f);
		rsxDrawVertex2f(context, vertexTexcoord_id, s2, t2);
//		rsxDrawVertex2f(context, vertexTexcoord_id, s1, t2);
//		rsxDrawVertex2f(context, vertexTexcoord_id, s2, t2);
		rsxDrawVertex4f(context, vertexPosition_id, (float) (x+width),(float) (y+height), depth, 1.0f);

		rsxDrawVertex4f(context, vertexColor0_id, (float)appliedColor[3].r/255.0f, (float)appliedColor[3].g/255.0f, 
			(float)appliedColor[3].b/255.0f, (float)appliedColor[3].a/255.0f);
		rsxDrawVertex2f(context, vertexTexcoord_id, s1, t2);
//		rsxDrawVertex2f(context, vertexTexcoord_id, s1, t1);
//		rsxDrawVertex2f(context, vertexTexcoord_id, s1, t2);
		rsxDrawVertex4f(context, vertexPosition_id, (float) x,(float) (y+height), depth, 1.0f);
	rsxDrawVertexEnd(context);

/*	//Init texture here or in calling code?
	GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
		GX_Position3f32((float) x,(float) y, depth );
		GX_Color4u8(appliedColor[0].r, appliedColor[0].g, appliedColor[0].b, appliedColor[0].a);
		GX_TexCoord2f32(s1,t1);
		GX_Position3f32((float) (x+width),(float) y, depth );
		GX_Color4u8(appliedColor[1].r, appliedColor[1].g, appliedColor[1].b, appliedColor[1].a);
		GX_TexCoord2f32(s2,t1);
		GX_Position3f32((float) (x+width),(float) (y+height), depth );
		GX_Color4u8(appliedColor[2].r, appliedColor[2].g, appliedColor[2].b, appliedColor[2].a);
		GX_TexCoord2f32(s2,t2);
		GX_Position3f32((float) x,(float) (y+height), depth );
		GX_Color4u8(appliedColor[3].r, appliedColor[3].g, appliedColor[3].b, appliedColor[3].a);
		GX_TexCoord2f32(s1,t2);
	GX_End();*/
}

void Graphics::testPrim()
{
//	setTEV(GX_PASSCLR);
//	setTEV(GX_REPLACE);
//	setTEV(GX_MODULATE);
	return;
	rsxDrawVertexBegin(context,GCM_TYPE_QUADS);
		rsxDrawVertex2f(context, vertexTexcoord_id, 0, 0);
		rsxDrawVertex4f(context, vertexColor0_id, 1, 0, 0, 1); //red
		rsxDrawVertex4f(context, vertexPosition_id,  100, 300, depth, 1.0f);

		rsxDrawVertex2f(context, vertexTexcoord_id, 0, 1);
		rsxDrawVertex4f(context, vertexColor0_id, 0, 1, 0, 1); //green
		rsxDrawVertex4f(context, vertexPosition_id,  100, 400, depth, 1.0f);

		rsxDrawVertex2f(context, vertexTexcoord_id, 1, 1);
		rsxDrawVertex4f(context, vertexColor0_id, 0, 0, 1, 1); //blue
		rsxDrawVertex4f(context, vertexPosition_id,  200, 400, depth, 1.0f);

		rsxDrawVertex2f(context, vertexTexcoord_id, 1, 0);
		rsxDrawVertex4f(context, vertexColor0_id, 1, 1, 1, 1); //white
		rsxDrawVertex4f(context, vertexPosition_id,  200, 300, depth, 1.0f);
	rsxDrawVertexEnd(context);
}

void Graphics::drawLine(int x1, int y1, int x2, int y2)
{
	rsxDrawVertexBegin(context,GCM_TYPE_LINES);
		rsxDrawVertex4f(context, vertexColor0_id, (float)appliedColor[0].r/255.0f, (float)appliedColor[0].g/255.0f, 
			(float)appliedColor[0].b/255.0f, (float)appliedColor[0].a/255.0f);
		rsxDrawVertex2f(context, vertexTexcoord_id, 0.0f,0.0f);
		rsxDrawVertex4f(context, vertexPosition_id, (float) x1,(float) y1, depth, 1.0f);

		rsxDrawVertex4f(context, vertexColor0_id, (float)appliedColor[1].r/255.0f, (float)appliedColor[1].g/255.0f, 
			(float)appliedColor[1].b/255.0f, (float)appliedColor[1].a/255.0f);
		rsxDrawVertex2f(context, vertexTexcoord_id, 0.0f,0.0f);
		rsxDrawVertex4f(context, vertexPosition_id, (float) x2,(float) y2, depth, 1.0f);
	rsxDrawVertexEnd(context);

/*	GX_Begin(GX_LINES, GX_VTXFMT0, 2);
		GX_Position3f32((float) x1,(float) y1, depth );
		GX_Color4u8(appliedColor[0].r, appliedColor[0].g, appliedColor[0].b, appliedColor[0].a);
		GX_TexCoord2f32(0.0f,0.0f);
		GX_Position3f32((float) x2,(float) y2, depth );
		GX_Color4u8(appliedColor[1].r, appliedColor[1].g, appliedColor[1].b, appliedColor[1].a);
		GX_TexCoord2f32(0.0f,0.0f);
	GX_End();*/
}

#ifndef PI
#define PI 3.14159f
#endif

void Graphics::drawCircle(int x, int y, int radius, int numSegments)
{
	float angle, point_x, point_y;

	rsxDrawVertexBegin(context,GCM_TYPE_LINE_STRIP);
	//GX_Begin(GX_LINESTRIP, GX_VTXFMT0, numSegments+1);

	for (int i = 0; i<=numSegments; i++)
	{
		angle = 2*PI * i/numSegments;
		point_x = (float)x + (float)radius * cos( angle );
		point_y = (float)y + (float)radius * sin( angle );

		rsxDrawVertex4f(context, vertexColor0_id, (float)appliedColor[0].r/255.0f, (float)appliedColor[0].g/255.0f, 
			(float)appliedColor[0].b/255.0f, (float)appliedColor[0].a/255.0f);
		rsxDrawVertex2f(context, vertexTexcoord_id, 0.0f,0.0f);
		rsxDrawVertex4f(context, vertexPosition_id, (float) point_x,(float) point_y, depth, 1.0f);
		//GX_Position3f32((float) point_x,(float) point_y, depth );
		//GX_Color4u8(appliedColor[0].r, appliedColor[0].g, appliedColor[0].b, appliedColor[0].a);
		//GX_TexCoord2f32(0.0f,0.0f);
	}

	rsxDrawVertexEnd(context);
	//GX_End();
}

void Graphics::drawString(int x, int y, std::string str)
{
	//todo
}

void Graphics::drawPoint(int x, int y, int radius)
{
	//TODO: Implement this in psl1ght
	//cellGcmSetPointSize(context,(float)radius*3)
	rsxDrawVertexBegin(context,GCM_TYPE_POINTS);
		rsxDrawVertex4f(context, vertexColor0_id, (float)appliedColor[0].r/255.0f, (float)appliedColor[0].g/255.0f, 
			(float)appliedColor[0].b/255.0f, (float)appliedColor[0].a/255.0f);
		rsxDrawVertex2f(context, vertexTexcoord_id, 0.0f,0.0f);
		rsxDrawVertex4f(context, vertexPosition_id, (float) x,(float) y, depth, 1.0f);
	rsxDrawVertexEnd(context);

/*	GX_SetPointSize(u8 (radius *3 ),GX_TO_ZERO);
	GX_Begin(GX_POINTS, GX_VTXFMT0, 1);
		GX_Position3f32((float) x,(float) y, depth );
		GX_Color4u8(appliedColor[0].r, appliedColor[0].g, appliedColor[0].b, appliedColor[0].a);
		GX_TexCoord2f32(0.0f,0.0f);
	GX_End();*/
}

void Graphics::setLineWidth(int width)
{
	//TODO: Implement this in psl1ght
	//void cellGcmSetLineWidth(context,uint32_t width)

	//GX_SetLineWidth((u8) (width * 6), GX_TO_ZERO );
}

void Graphics::pushDepth(float d)
{
	depthStack.push(getDepth());
	setDepth(d);
}

void Graphics::popDepth()
{
	depthStack.pop();
	if(depthStack.size() != 0)
	{
		setDepth(depthStack.top());
	}
	else
	{
		setDepth(1.0f);
	}
}

void Graphics::enableScissor(int x, int y, int width, int height)
{
	//TODO

/*	if(screenMode)
	{
		int x1 = (x+104)*640/848;
		int x2 = (x+width+104)*640/848;
		GX_SetScissor((u32) x1,(u32) y,(u32) x2-x1,(u32) height);
	}
	else
		GX_SetScissor((u32) x,(u32) y,(u32) width,(u32) height);*/
}

void Graphics::disableScissor()
{
	//TODO

	//GX_SetScissor((u32) 0,(u32) 0,(u32) viewportWidth,(u32) viewportHeight); //Set to the same size as the viewport.
}

void Graphics::enableBlending(bool blend)
{
	//TODO: Implement this in psl1ght
	//void rsxSetBlendEnable(gcmContextData *context,u32 enable);
	//void rsxSetBlendFunc(gcmContextData *context,u16 sfcolor,u16 dfcolor,u16 sfalpha,u16 dfalpha);
	//void rsxSetBlendEquation(gcmContextData *context,u16 color,u16 alpha);
	//void rsxSetBlendColor(gcmContextData *context,u16 color0,u16 color1);
	if (blend)
	{
		rsxSetBlendFunc(context, GCM_SRC_ALPHA, GCM_ONE_MINUS_SRC_ALPHA, GCM_SRC_ALPHA, GCM_ONE_MINUS_SRC_ALPHA);
		rsxSetBlendEnable(context, GCM_TRUE);
	}
	else
	{
		rsxSetBlendFunc(context, GCM_ONE, GCM_ZERO, GCM_ONE, GCM_ZERO);
		rsxSetBlendEnable(context, GCM_FALSE);
	}

/*	if (blend)
		GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
	else
		GX_SetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);*/
}

void Graphics::setTEV(int tev_op)
{
	switch (tev_op)
	{
	case GX_REPLACE:
		shader_mode = (float) SHADER_PASSTEX;
		break;
	case GX_PASSCLR:
		shader_mode = (float) SHADER_PASSCOLOR;
		break;
	case GX_MODULATE:
	default:
		shader_mode = (float) SHADER_MODULATE;
		break;
	}
	rsxSetFragmentProgramParameter(context,fpo,mode_id,&shader_mode,fp_offset);
	rsxLoadFragmentProgramLocation(context,fpo,fp_offset,GCM_LOCATION_RSX);

	//Called with: GX_MODULATE, GX_PASSCLR, GX_REPLACE
/*	GX_SetNumTevStages(1);
	GX_SetNumChans (1);
	GX_SetNumTexGens (1);
	GX_SetTevOrder (GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
	GX_SetTevOp (GX_TEVSTAGE0, tev_op);*/
}

void Graphics::pushTransparency(float f)
{
	transparencyStack.push(getTransparency());
	setTransparency(f);
}

void Graphics::popTransparency()
{
	transparencyStack.pop();
	if(transparencyStack.size() != 0)
	{
		setTransparency(transparencyStack.top());
	}
	else
	{
		setTransparency(1.0f);
	}
}

void Graphics::setTransparency(float f)
{
	transparency = f;
	applyCurrentColor();
}

float Graphics::getTransparency()
{
	return transparency;
}

void Graphics::applyCurrentColor()
{
	for (int i = 0; i < 4; i++){
		appliedColor[i].r = currentColor[i].r;
		appliedColor[i].g = currentColor[i].g;
		appliedColor[i].b = currentColor[i].b;
		appliedColor[i].a = (u8) (getCurrentTransparency(i) * 255.0f);
	}
}

float Graphics::getCurrentTransparency(int index)
{
	float alpha = (float)currentColor[index].a/255.0f;
	float val = alpha * transparency;
	return val;
}

} //namespace menu 
