/**
 * Mupen64 - vi.cpp
 * Copyright (C) 2002 Hacktarux
 *
 * Mupen64 homepage: http://mupen64.emulation64.com
 * email address: hacktarux@yahoo.fr
 * 
 * If you want to contribute to the project please contact
 * me first (maybe someone is already making what you are
 * planning to do).
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
 * You should have received a copy of the GNU General Public
 * Licence along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139,
 * USA.
 *
**/

#include <stdio.h>
#include <math.h>
#include <malloc.h>

#include "vi.h"
#include "global.h"
#include "color.h"
#ifdef DEBUGON
# include <debug.h>
#endif
#ifdef PS3
#include <rsx/rsx.h>
#include <sysutil/video.h>
#include "../main/rsxutil.h"

#include <vectormath/cpp/vectormath_aos.h>
using namespace Vectormath::Aos;

#include "combined_shader_vpo.h"
#include "combined_shader_fpo.h"
#endif //PS3


VI::VI(GFX_INFO info) : gfxInfo(info), bpp(0)
{
#ifdef PS3
	FBtex = (u16*)rsxMemalign(128,(640*480*2));
#else //PS3
	FBtex = (u16*) memalign(32,640*480*2);
#endif //!PS3
}

VI::~VI()
{
#ifdef PS3
	rsxFree(FBtex);
#else //PS3
	free(FBtex);
#endif //!PS3
}

void VI::statusChanged()
{
   switch (*gfxInfo.VI_STATUS_REG & 3)
     {
      case 2:
	if (bpp != 16)
	  {
	     bpp = 16;
	     setVideoMode(640, 480);
	  }
	break;
      case 3:
	if (bpp != 32)
	  {
	     printf("VI:32bits\n");
	     bpp =32;
	  }
	break;
     }
}

void VI::widthChanged()
{
   /*switch(gfxInfo.HEADER[0x3c])
     {
      case 0x44:
      case 0x46:
      case 0x49:
      case 0x50:
      case 0x53:
      case 0x55:
      case 0x58:
      case 0x59:
	printf("VI:pal rom\n");
	break;
     }
   width = *gfxInfo.VI_WIDTH_REG;
   height = width * 3 / 4;
   initMode();*/
}

unsigned int convert_pixels(short src1, short src2){
	char b1 = ((src1 >>  0) & 0x1F) * (256/32);
	char g1 = ((src1 >>  5) & 0x1F) * (256/32);
	char r1 = ((src1 >> 10) & 0x1F) * (256/32);
	char b2 = ((src2 >>  0) & 0x1F) * (256/32);
	char g2 = ((src2 >>  5) & 0x1F) * (256/32);
	char r2 = ((src2 >> 10) & 0x1F) * (256/32);
	
	int y1, cb1, cr1, y2, cb2, cr2, cb, cr;
	
	y1 = (299 * r1 + 587 * g1 + 114 * b1) / 1000;
	cb1 = (-16874 * r1 - 33126 * g1 + 50000 * b1 + 12800000) / 100000;
	cr1 = (50000 * r1 - 41869 * g1 - 8131 * b1 + 12800000) / 100000;
	
	y2 = (299 * r2 + 587 * g2 + 114 * b2) / 1000;
	cb2 = (-16874 * r2 - 33126 * g2 + 50000 * b2 + 12800000) / 100000;
	cr2 = (50000 * r2 - 41869 * g2 - 8131 * b2 + 12800000) / 100000;
	 
	cb = (cb1 + cb2) >> 1;
	cr = (cr1 + cr2) >> 1;
	 
	return (y1 << 24) | (cb << 16) | (y2 << 8) | cr;
}

#ifdef PS3
extern u32 *color_buffer[2];
extern VideoResolution res;
#endif //PS3

void VI::updateScreen()
{
//   printf("Should be updating screen: bpp = %d, width_reg = %d\n", bpp, *gfxInfo.VI_WIDTH_REG);
   if (!bpp) {return;}
   if (!*gfxInfo.VI_WIDTH_REG) {return;}
   int h_end = *gfxInfo.VI_H_START_REG & 0x3FF;
   int h_start = (*gfxInfo.VI_H_START_REG >> 16) & 0x3FF;
   int v_end = *gfxInfo.VI_V_START_REG & 0x3FF;
   int v_start = (*gfxInfo.VI_V_START_REG >> 16) & 0x3FF;
   float scale_x = ((int)*gfxInfo.VI_X_SCALE_REG & 0xFFF) / 1024.0f;
   float scale_y = (((int)*gfxInfo.VI_Y_SCALE_REG & 0xFFF)>>1) / 1024.0f;
   
   short *im16 = (short*)((char*)gfxInfo.RDRAM +
			  (*gfxInfo.VI_ORIGIN_REG & 0x7FFFFF));
//   int *buf16 = (int*)getScreenPointer();
   int minx = (640-(h_end-h_start))/2;
   int maxx = 640-minx;
   int miny = (480-(v_end-v_start))/2;
   int maxy = 480-miny;
   int ind = 0;
   float px, py;
   py=0.0f;

#ifdef DEBUGON
   _break();
#endif
   //printf("Beginning to copy framebuffer... N64FB offset = %08x", *gfxInfo.VI_ORIGIN_REG & 0x7FFFFF);
   //printf("\nmin: (%d,%d) max: (%d,%d), GCFB = %08x, N64FB = %08x\n",
   //        minx, miny, maxx, maxy, buf16, im16);
   //printf("scale_x = %f, scale_y = %f\n", scale_x, scale_y);
   //fflush(stdout);
   // Here I'm disabling antialiasing to try to track down the bug
/*   if (TRUE || (*gfxInfo.VI_STATUS_REG & 0x30) == 0x30) // not antialiased
     {
     	//printf(" Not antialiased ");
     	//fflush(stdout);
	for (int j=0; j<480; j++)
	  {
	     if (j < miny || j > maxy)
	       for (int i=0; i<640/2; i++)
		 buf16[j*640/2+i] = 0;
	     else
	       {
		  px=0.0f;
		  for (int i=0; i<640/2; i++)
		    {
		       if (i < minx || i > maxx)
			 buf16[j*640/2+i] = 0;
		       else
			 {
			    short pix1 = im16[((int)py*(*gfxInfo.VI_WIDTH_REG)+(int)px)]>>1;
			    px += scale_x;
			    short pix2 = im16[((int)py*(*gfxInfo.VI_WIDTH_REG)+(int)px)]>>1;
			    px += scale_x;
			    buf16[j*640/2+i] = convert_pixels(pix1, pix2);
			 }
			//printf(" (%d,%d) ", i, j); fflush(stdout);
		    }
		  py += scale_y;
	       }
	  }
     }
   else
     {
     	//printf(" Antialiased ");
     	//fflush(stdout);
	for (int j=0; j<480; j++)
	  {
	     if (j < miny || j > maxy)
	       for (int i=0; i<640; i++)
		 buf16[j*640+i] = 0;
	     else
	       {
		  px=0;
		  for (int i=0; i<640; i++)
		    {
		       if (i < minx || i > maxx)
			 buf16[j*640+i] = 0;
		       else
			 {
			    bool xint = (px - (int)px) == 0.0f, yint = (py - (int)py) == 0.0f;
			    if (xint && yint)
			      {
				 buf16[j*640+i] = 
				   im16[((int)py*(*gfxInfo.VI_WIDTH_REG)+(int)px)^S16]>>1;
			      }
			    else if (yint)
			      {
				 Color16 l,r;
				 int w = *gfxInfo.VI_WIDTH_REG;
				 l=im16[((int)py*w+(int)px)^S16];
				 r=im16[((int)py*w+(int)(px+1.0f))^S16];
				 buf16[j*640+i] = 
				   (int)(l*(1.0f-(px-(int)px))+r*(px-(int)px))>>1;
			      }
			    else if (xint)
			      {
				 Color16 t,b;
				 int w = *gfxInfo.VI_WIDTH_REG;
				 t=im16[((int)py*w+(int)px)^S16];
				 b=im16[((int)(py+1)*w+(int)px)^S16];
				 buf16[j*640+i] = 
				   (int)(t*(1-(py-(int)py))+b*(py-(int)py))>>1;
			      }
			    else
			      {
				 Color16 t,b,l,r;
				 int w = *gfxInfo.VI_WIDTH_REG;
				 l=im16[((int)py*w+(int)px)^S16];
				 r=im16[((int)py*w+(int)(px+1))^S16];
				 t=l*(1-(px-(int)px))+r*(px-(int)px);
				 l=im16[((int)(py+1)*w+(int)px)^S16];
				 r=im16[((int)(py+1)*w+(int)(px+1))^S16];
				 b=l*(1-(px-(int)px))+r*(px-(int)px);
				 buf16[j*640+i] = 
				   (int)(t*(1-(py-(int)py))+b*(py-(int)py))>>1;
			      }
			    px += scale_x;
			 }
		    }
		  py += scale_y;
	       }
	  }
     }*/

#ifdef PS3
	//init shader:
	u32 fpsize = 0;
	u32 fp_offset;
	u32 *fp_buffer = NULL;

	s32 projMatrix_id = -1;
	s32 modelViewMatrix_id = -1;
	s32 vertexPosition_id = -1;
	s32 vertexColor0_id = -1;
	s32 vertexTexcoord_id = -1;
	s32 textureUnit_id = -1;
	s32 mode_id = -1;

	void *vp_ucode = NULL;
	rsxVertexProgram *vpo = (rsxVertexProgram*)combined_shader_vpo;

	void *fp_ucode = NULL;
	rsxFragmentProgram *fpo = (rsxFragmentProgram*)combined_shader_fpo;

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

	for (int j=0; j<480; j++)
	{
		for (int i=0; i<640; i++)
		{
			if (j < miny || j > maxy)
				FBtex[ind++] = 0;
			else
			{
				px = scale_x*i;
				py = scale_y*j;
				if (i < minx || i > maxx)
					FBtex[ind++] = 0;
				else
					FBtex[ind++] = 0x8000 | (im16[((int)py*(*gfxInfo.VI_WIDTH_REG)+(int)px)]>>1); //convert R5G5B5A1 to A1R5G5B5
			}
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
	rsxLoadTexture(context,textureUnit_id,&texture);
	rsxTextureControl(context,textureUnit_id,GCM_TRUE,0<<8,12<<8,GCM_TEXTURE_MAX_ANISO_1);
	rsxTextureFilter(context,textureUnit_id,GCM_TEXTURE_LINEAR,GCM_TEXTURE_LINEAR,GCM_TEXTURE_CONVOLUTION_QUINCUNX);
	rsxTextureWrapMode(context,textureUnit_id,GCM_TEXTURE_CLAMP_TO_EDGE,GCM_TEXTURE_CLAMP_TO_EDGE,GCM_TEXTURE_CLAMP_TO_EDGE,0,GCM_TEXTURE_ZFUNC_LESS,0);

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
	rsxSetDepthFunc(context,GCM_LESS);
	rsxSetShadeModel(context,GCM_SHADE_MODEL_SMOOTH);
	rsxSetDepthWriteEnable(context,1);
	rsxSetFrontFace(context,GCM_FRONTFACE_CCW);

	//inline const Matrix4 Matrix4::orthographic( float left, float right, float bottom, float top, float zNear, float zFar )
	Matrix4 projMatrix,viewMatrix,modelMatrix,modelViewMatrix;
	Point3 eye_pos = Point3(0.0f,0.0f,20.0f);
	Point3 eye_dir = Point3(0.0f,0.0f,0.0f);
	Vector3 up_vec = Vector3(0.0f,1.0f,0.0f);

	projMatrix = transpose(Matrix4::orthographic(0.0f, 640.0f, 480.0f, 0.0f, 0.0f, 1.0f ));
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

	//Turn off Blending
	rsxSetBlendEnable(context, GCM_FALSE);

	for(int i=0;i<8;i++)
		rsxSetViewportClip(context,i,display_width,display_height);

	rsxLoadVertexProgram(context,vpo,vp_ucode);
	rsxSetVertexProgramParameter(context,vpo,projMatrix_id,(float*)&projMatrix);
	rsxSetVertexProgramParameter(context,vpo,modelViewMatrix_id,(float*)&modelViewMatrix);

	float shader_mode = 1; //SHADER_PASSTEX
	rsxSetFragmentProgramParameter(context,fpo,mode_id,&shader_mode,fp_offset);
	rsxLoadFragmentProgramLocation(context,fpo,fp_offset,GCM_LOCATION_RSX);

	rsxSetUserClipPlaneControl(context,GCM_USER_CLIP_PLANE_DISABLE,
									   GCM_USER_CLIP_PLANE_DISABLE,
									   GCM_USER_CLIP_PLANE_DISABLE,
									   GCM_USER_CLIP_PLANE_DISABLE,
									   GCM_USER_CLIP_PLANE_DISABLE,
									   GCM_USER_CLIP_PLANE_DISABLE);

	//void rsxDrawVertexBegin(gcmContextData *context,u32 type);
	rsxDrawVertexBegin(context,GCM_TYPE_QUADS);

		rsxDrawVertex4f(context, vertexColor0_id, 1.0f, 1.0f, 1.0f, 1.0f);
		rsxDrawVertex2f(context, vertexTexcoord_id, 0.0f, 0.0f);
		rsxDrawVertex3f(context, vertexPosition_id, 0.0f, 0.0f, 0.0f);

		rsxDrawVertex4f(context, vertexColor0_id, 1.0f, 1.0f, 1.0f, 1.0f);
		rsxDrawVertex2f(context, vertexTexcoord_id, 1.0f, 0.0f);
		rsxDrawVertex3f(context, vertexPosition_id, 640.0f, 0.0f, 0.0f);

		rsxDrawVertex4f(context, vertexColor0_id, 1.0f, 1.0f, 1.0f, 1.0f);
		rsxDrawVertex2f(context, vertexTexcoord_id, 1.0f, 1.0f);
		rsxDrawVertex3f(context, vertexPosition_id, 640.0f, 480.0f, 0.0f);

		rsxDrawVertex4f(context, vertexColor0_id, 1.0f, 1.0f, 1.0f, 1.0f);
		rsxDrawVertex2f(context, vertexTexcoord_id, 0.0f, 1.0f);
		rsxDrawVertex3f(context, vertexPosition_id, 0.0f, 480.0f, 0.0f);

	rsxDrawVertexEnd(context);

	flip();

	//free RSX buffers
	if (fp_buffer) rsxFree(fp_buffer);

#if 0
	//PS3 - Copy N64 framebuffer directly to RSX framebuffer using the PPU.
	//N64 Framebuffer is in RGB5A1 format. Write it directly to the current RSX framebuffer.
	u32* buffer = color_buffer[curr_fb];
	int x_offset = (res.width - 640)/2;
	int y_offset = (res.height - 480)/2;
	if (x_offset < 0) x_offset = 0;
	if (y_offset < 0) y_offset = 0;
	for (int j=0; j<480; j++)
	{
		if (j+y_offset > res.height) continue;
		for (int i=0; i<640; i++)
		{
			if (i+x_offset > res.width) continue;
			px = scale_x*i;
			py = scale_y*j;
			u16 color16 = im16[((int)py*(*gfxInfo.VI_WIDTH_REG)+(int)px)];
			u32 color = ((color16 & 0xF800)<<8) | ((color16 & 0x07C0)<<5) | ((color16 & 0x003E)<<2); //XRGB
			buffer[(j+y_offset)*res.width + i + x_offset] = color; 
		}
	}
	flip();
#endif

#else //PS3
	//Implementation for GC/Wii
	//N64 Framebuffer is in RGB5A1 format, so shift by 1 and retile.
	for (int j=0; j<480; j+=4)
	{
		for (int i=0; i<640; i+=4)
		{
			for (int jj=0; jj<4; jj++)
			{
				if (j+jj < miny || j+jj > maxy)
				{
					FBtex[ind++] = 0;
					FBtex[ind++] = 0;
					FBtex[ind++] = 0;
					FBtex[ind++] = 0;
				}
				else
				{
					px = scale_x*i;
					py = scale_y*(j+jj);
					for (int ii=0; ii<4; ii++)
					{
						if (i+ii < minx || i+ii > maxx)
							FBtex[ind++] = 0;
						else
							FBtex[ind++] = 0x8000 | (im16[((int)py*(*gfxInfo.VI_WIDTH_REG)+(int)px)]>>1);
						px += scale_x;
					}
				}
			}
		}
	}

	GX_SetCopyClear ((GXColor){0,0,0,255}, 0xFFFFFF);
	GX_CopyDisp (vi->getScreenPointer(), GX_TRUE);	//clear the EFB before executing new Dlist
	GX_DrawDone ();
	vi->updateDEBUG();

	//Initialize texture
	GX_InitTexObj(&FBtexObj, FBtex, 640, 480, GX_TF_RGB5A3, GX_CLAMP, GX_CLAMP, GX_FALSE); 
	DCFlushRange(FBtex, 640*480*2);
	GX_InvalidateTexAll();
	GX_LoadTexObj(&FBtexObj, GX_TEXMAP0);

	GX_SetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR); 
	GX_SetAlphaCompare(GX_ALWAYS,0,GX_AOP_AND,GX_ALWAYS,0);
	GX_SetZMode(GX_DISABLE,GX_ALWAYS,GX_FALSE);
	GX_SetCullMode (GX_CULL_NONE);
	GX_SetFog(GX_FOG_NONE,0.1,1.0,0.0,1.0,(GXColor) {0,0,0,255});

	Mtx44 GXprojection;
	guMtxIdentity(GXprojection);
	guOrtho(GXprojection, 0, 480, 0, 640, 0.0f, 1.0f);
	GX_LoadProjectionMtx(GXprojection, GX_ORTHOGRAPHIC); 
	Mtx	GXmodelViewIdent;
	guMtxIdentity(GXmodelViewIdent);
	GX_LoadPosMtxImm(GXmodelViewIdent,GX_PNMTX0);
	GX_SetViewport((f32) 0,(f32) 0,(f32) 640,(f32) 480, 0.0f, 1.0f);
	GX_SetScissor((u32) 0,(u32) 0,(u32) 640,(u32) 480);	//Set to the same size as the viewport.
	//set vertex description
	GX_ClearVtxDesc();
	GX_SetVtxDesc(GX_VA_PTNMTXIDX, GX_PNMTX0);
	GX_SetVtxDesc(GX_VA_TEX0MTXIDX, GX_TEXMTX0);
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);
	//set vertex attribute formats
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_F32, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

	//enable textures
	GX_SetNumChans (0);
	GX_SetNumTexGens (1);
	GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
	GX_SetNumTevStages (1);
	GX_SetTevOrder (GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
	GX_SetTevOp (GX_TEVSTAGE0, GX_REPLACE);

	GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
		GX_Position2f32( 0.0f, 0.0f );
		GX_TexCoord2f32( 0.0f, 0.0f );
		GX_Position2f32( 640.0f, 0.0f );
		GX_TexCoord2f32( 1.0f, 0.0f );
		GX_Position2f32( 640.0f, 480.0f );
		GX_TexCoord2f32( 1.0f, 1.0f );
		GX_Position2f32( 0.0f, 480.0f );
		GX_TexCoord2f32( 0.0f, 1.0f );
	GX_End();
	GX_DrawDone();

   //printf(" done.\nBlitting...");
   //fflush(stdout);
   blit();
   //printf(" done.\n");
#endif //!PS3
}

void VI::debug_plot(int x, int y, int c)
{
   short *buf16 = (short*)getScreenPointer();
   buf16[y*640+x] = c>>1;
}

void VI::flush()
{
   blit();
}
