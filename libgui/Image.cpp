/**
 * Wii64 - Image.cpp
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

#include "Image.h"

namespace menu {

Image::Image(void* texture, u16 wd, u16 ht, u8 fmt, u8 wrap_s, u8 wrap_t, u8 mipmap)
		: img_ptr(texture),
		  tlut_ptr(0),
		  width(wd),
		  height(ht),
		  format(fmt),
		  tlut_format(0)
{
#ifdef __GX__
	GX_InitTexObj(&obj, img_ptr, width, height, format, wrap_s, wrap_t, mipmap);
#else //__GX__
	wraps = wrap_s;
	wrapt = wrap_t;

	u32 bpp, pitch;
	u8	*pDst8, *pSrc8;
	u16	*pDst16, *pSrc16;
	u32 *pDst32;

	switch(fmt)
	{
	case GX_TF_I4:	//Convert to GCM_TEXTURE_FORMAT_L8
		rsxFmt = GCM_TEXTURE_FORMAT_L8 | GCM_TEXTURE_FORMAT_LIN;
		bpp = 1;
		pitch = (width*bpp);
		rsx_texture_buffer = (u32*)rsxMemalign(128,(width*height*bpp));
		if(!rsx_texture_buffer) return;
		pDst8 = (u8*) rsx_texture_buffer;
		pSrc8 = (u8*) img_ptr;
		for (int i=0; i<height; i+=8)
		{
			for (int j=0; j<width; j+=8)
			{
				for (int ii=0; ii<8; ii++)
				{
					for (int jj=0; jj<8; jj+=2)
					{
						u8 col = *pSrc8++;
						pDst8[(i+ii)*width + (j+jj)    ] = (col & 0xf0) | ((col >> 4) & 0x0f);
						pDst8[(i+ii)*width + (j+jj) + 1] = (col & 0x0f) | ((col << 4) & 0xf0);
					}
				}
			}
		}
		break;
	case GX_TF_I8:	//Convert to GCM_TEXTURE_FORMAT_L8
		rsxFmt = GCM_TEXTURE_FORMAT_L8 | GCM_TEXTURE_FORMAT_LIN;
		bpp = 1;
		pitch = (width*bpp);
		rsx_texture_buffer = (u32*)rsxMemalign(128,(width*height*bpp));
		if(!rsx_texture_buffer) return;
		pDst8 = (u8*) rsx_texture_buffer;
		pSrc8 = (u8*) img_ptr;
		for (int i=0; i<height; i+=4)
		{
			for (int j=0; j<width; j+=8)
			{
				for (int ii=0; ii<4; ii++)
				{
					for (int jj=0; jj<8; jj++)
					{
						pDst8[(i+ii)*width + (j+jj)] = *pSrc8++;
					}
				}
			}
		}
		break;
	case GX_TF_RGB5A3:	//Convert to GCM_TEXTURE_FORMAT_A1R5G5B5
		rsxFmt = GCM_TEXTURE_FORMAT_A1R5G5B5 | GCM_TEXTURE_FORMAT_LIN;
		bpp = 2;
		pitch = (width*bpp);
		rsx_texture_buffer = (u32*)rsxMemalign(128,(width*height*bpp));
		if(!rsx_texture_buffer) return;
		pDst16 = (u16*) rsx_texture_buffer;
		pSrc16 = (u16*) img_ptr;
		for (int i=0; i<height; i+=4)
		{
			for (int j=0; j<width; j+=4)
			{
				for (int ii=0; ii<4; ii++)
				{
					for (int jj=0; jj<4; jj++)
					{
						u16 col = *pSrc16++;
						pDst16[(i+ii)*width + (j+jj)] = col&0x8000 ? col : ((col&0x0f00)<<3) | ((col&0x00f0)<<2) | ((col&0x000f)<<1);
					}
				}
			}
		}
		break;
	case GX_TF_RGBA8:	//Convert to GCM_TEXTURE_FORMAT_A8R8G8B8
	default:
		rsxFmt = GCM_TEXTURE_FORMAT_A8R8G8B8 | GCM_TEXTURE_FORMAT_LIN;
		bpp = 4;
		pitch = (width*bpp);
		rsx_texture_buffer = (u32*)rsxMemalign(128,(width*height*bpp));
		if(!rsx_texture_buffer) return;
		pDst32 = (u32*) rsx_texture_buffer;
		pSrc16 = (u16*) img_ptr;
		for (int i=0; i<height; i+=4)
		{
			for (int j=0; j<width; j+=4)
			{
				for (int ii=0; ii<4; ii++)
				{
					for (int jj=0; jj<4; jj++)
					{
						pDst32[(i+ii)*width + (j+jj)] = (((u32)(pSrc16[0]))<<16) | ((u32)(pSrc16[16]));
						pSrc16++;
					}
				}
				pSrc16+=16;
			}
		}
		break;
	}

	rsxAddressToOffset(rsx_texture_buffer,&rsx_texture_offset);

	texobj.format		= rsxFmt;
	texobj.mipmap		= 1;
	texobj.dimension	= GCM_TEXTURE_DIMS_2D;
	texobj.cubemap		= GCM_FALSE;
	if (rsxFmt == (GCM_TEXTURE_FORMAT_L8 | GCM_TEXTURE_FORMAT_LIN))
	{
		texobj.remap	= ((GCM_TEXTURE_REMAP_TYPE_REMAP << GCM_TEXTURE_REMAP_TYPE_B_SHIFT) |
						   (GCM_TEXTURE_REMAP_TYPE_REMAP << GCM_TEXTURE_REMAP_TYPE_G_SHIFT) |
						   (GCM_TEXTURE_REMAP_TYPE_REMAP << GCM_TEXTURE_REMAP_TYPE_R_SHIFT) |
						   (GCM_TEXTURE_REMAP_TYPE_REMAP << GCM_TEXTURE_REMAP_TYPE_A_SHIFT) |
						   (GCM_TEXTURE_REMAP_COLOR_B << GCM_TEXTURE_REMAP_COLOR_B_SHIFT) |
						   (GCM_TEXTURE_REMAP_COLOR_G << GCM_TEXTURE_REMAP_COLOR_G_SHIFT) |
						   (GCM_TEXTURE_REMAP_COLOR_R << GCM_TEXTURE_REMAP_COLOR_R_SHIFT) |
						   (GCM_TEXTURE_REMAP_COLOR_R << GCM_TEXTURE_REMAP_COLOR_A_SHIFT)); //A is always 1 for rsxFmt=L8 
	}
	else
	{
		texobj.remap	= ((GCM_TEXTURE_REMAP_TYPE_REMAP << GCM_TEXTURE_REMAP_TYPE_B_SHIFT) |
						   (GCM_TEXTURE_REMAP_TYPE_REMAP << GCM_TEXTURE_REMAP_TYPE_G_SHIFT) |
						   (GCM_TEXTURE_REMAP_TYPE_REMAP << GCM_TEXTURE_REMAP_TYPE_R_SHIFT) |
						   (GCM_TEXTURE_REMAP_TYPE_REMAP << GCM_TEXTURE_REMAP_TYPE_A_SHIFT) |
						   (GCM_TEXTURE_REMAP_COLOR_B << GCM_TEXTURE_REMAP_COLOR_B_SHIFT) |
						   (GCM_TEXTURE_REMAP_COLOR_G << GCM_TEXTURE_REMAP_COLOR_G_SHIFT) |
						   (GCM_TEXTURE_REMAP_COLOR_R << GCM_TEXTURE_REMAP_COLOR_R_SHIFT) |
						   (GCM_TEXTURE_REMAP_COLOR_A << GCM_TEXTURE_REMAP_COLOR_A_SHIFT));
	}
	texobj.width		= width;
	texobj.height		= height;
	texobj.depth		= 1;
	texobj.location		= GCM_LOCATION_RSX;
	texobj.pitch		= pitch;
	texobj.offset		= rsx_texture_offset;
#endif //!__GX__
}

Image::Image(void* texture, u16 wd, u16 ht, u8 fmt, u8 wrap_s, u8 wrap_t, u8 mipmap, void* lut, u8 lut_fmt, u8 lut_name, u16 lut_size)
		: img_ptr(texture),
		  tlut_ptr(lut),
		  width(wd),
		  height(ht),
		  format(fmt),
		  tlut_format(lut_fmt),
		  tlut_name(lut_name),
		  tlut_size(lut_size)
{
#ifdef __GX__
	GX_InitTlutObj(&tlut_obj, tlut_ptr, tlut_format, tlut_size);
	GX_InitTexObjCI(&obj, img_ptr, width, height, format, wrap_s, wrap_t, mipmap, tlut_name);
#else //__GX__
	rsx_texture_buffer = NULL;
#endif //!__GX__
}

Image::~Image()
{
#ifndef __GX__
	if (rsx_texture_buffer) rsxFree(rsx_texture_buffer);
#endif //!__GX__
}

void Image::activateImage(u8 mapid)
{
#ifdef __GX__
	if (tlut_ptr) GX_LoadTlut(&tlut_obj, tlut_name);	
	GX_LoadTexObj(&obj, mapid);
#else //__GX__
	//TODO: rework this to not use a global variable
//	rsxFlushBuffer(context);
	rsxInvalidateTextureCache(context,GCM_INVALIDATE_TEXTURE);

	rsxLoadTexture(context,globalTextureUnit_id,&texobj);
	rsxTextureControl(context,globalTextureUnit_id,GCM_TRUE,0<<8,12<<8,GCM_TEXTURE_MAX_ANISO_1);
	rsxTextureFilter(context,globalTextureUnit_id,GCM_TEXTURE_LINEAR,GCM_TEXTURE_LINEAR,GCM_TEXTURE_CONVOLUTION_QUINCUNX);
	rsxTextureWrapMode(context,globalTextureUnit_id,wraps,wrapt,GCM_TEXTURE_CLAMP_TO_EDGE,0,GCM_TEXTURE_ZFUNC_LESS,0);
//	dbg_printf("Image activate  globTexUnit %d\r\n", globalTextureUnit_id);
#endif //!__GX__
}

} //namespace menu
