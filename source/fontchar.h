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

#ifndef FONTCHAR_H
#define FONTCHAR_H

#include "ac_image.h"

class CFontGen;

class CFontChar
{
public:
	CFontChar();
	~CFontChar();

	int  DrawChar(HFONT font, int id, const CFontGen *gen);
	int  DrawInvalidCharGlyph(HFONT font, const CFontGen *gen);
	void AddOutline(int thickness);

	int  DrawGlyph(HFONT font, int glyph, const CFontGen *gen);
	int  DrawGlyphFromOutline(HDC dc, int glyph, int fontHeight, int fontAscent, const CFontGen *gen);
	int  DrawGlyphFromBitmap(HDC dc, int glyph, int fontHeight, int fontAscent, const CFontGen *gen);

	void DownscaleImage(bool useSmoothing);
	void TrimLeftAndRight();

	void CreateFromImage(int id, cImage *image, int xoffset, int yoffset, int advance);

	int m_id;

	int m_x;
	int m_y;
	int m_width;
	int m_height;
	int m_yoffset;
	int m_xoffset;
	int m_advance;
	int m_page;
	int m_chnl;

	bool HasOutline();
	BYTE GetPixelValue(int x, int y, int encoding);

	bool m_colored;
	bool m_isChar;

	cImage *m_charImg;
};

#endif