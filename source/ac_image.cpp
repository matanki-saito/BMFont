/*
   AngelCode Bitmap Font Generator
   Copyright (c) 2004-2014 Andreas Jonsson
  
   This software is provided 'as-is', without any express or implied 
   warranty. In no event will the authors be held liable for any 
   damages arising from the use of this software.

   Permission is granted to anyone to use this software for any 
   purpose, including commercial applications, and to alter it and 
   redistribute it freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you 
      must not claim that you wrote the original software. If you use
      this software in a product, an acknowledgment in the product 
      documentation would be appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and 
      must not be misrepresented as being the original software.

   3. This notice may not be removed or altered from any source 
      distribution.
  
   Andreas Jonsson
   andreas@angelcode.com
*/

#include <string.h>
#include <new>
#include "ac_image.h"

#define FAIL(r) {returnCode = (r); goto cleanup;}

cImage::cImage()
{
	pixels = 0;
	width  = 0;
	height = 0;
	isTopDown = true;
}

cImage::cImage(int width, int height)
{
	pixels = new (std::nothrow) PIXEL[width*height];
	if( pixels )
	{
		this->width = width;
		this->height = height;
	}
	else
	{
		this->width = 0;
		this->height = 0;
	}
}

cImage::~cImage()
{
	if( pixels )
		delete[] pixels;
}

int cImage::CopyToDC(HDC dc, int x, int y, int w, int h)
{
	if( pixels == 0 )
		return 0;

	BITMAPINFO bmi;
	GetBitmapInfoHeader((BITMAPINFOHEADER *)&bmi);

	StretchDIBits(dc, x, y, w, h, 0, 0, width, height, pixels, &bmi, DIB_RGB_COLORS, SRCCOPY);

	return 0;
}

void cImage::GetBitmapInfoHeader(BITMAPINFOHEADER *bmih)
{
	bmih->biSize          = sizeof(BITMAPINFOHEADER);
	bmih->biBitCount      = 32;
	bmih->biWidth         = width;
	bmih->biHeight        = isTopDown ? -height : height; 
	bmih->biCompression   = BI_RGB;
	bmih->biPlanes        = 1;
	bmih->biSizeImage     = 0;
	bmih->biClrImportant  = 0;
	bmih->biClrUsed       = 0;
	bmih->biXPelsPerMeter = 0;
	bmih->biYPelsPerMeter = 0;
}

int cImage::Create(int w, int h)
{
	if( pixels ) delete[] pixels;
	pixels = new (std::nothrow) PIXEL[w*h];
	if( pixels )
	{
		width  = w;
		height = h;
	}
	else
	{
		width = 0;
		height = 0;
		return -1;
	}

	return 0;
}

void cImage::Clear(PIXEL color)
{
	for( int y = 0; y < height; y++ )
		for( int x = 0; x < width; x++ )
			pixels[y*width+x] = color;
}

