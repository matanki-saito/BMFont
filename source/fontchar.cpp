/*
   AngelCode Bitmap Font Generator
   Copyright (c) 2004-2017 Andreas Jonsson
  
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

#include <math.h>
#include "fontchar.h"
#include "unicode.h"
#include "acutil_unicode.h"
#include "fontgen.h"
#include "dynamic_funcs.h"
#include <assert.h>

CFontChar::CFontChar()
{
	m_charImg = 0;
}

CFontChar::~CFontChar()
{
	if( m_charImg )
		delete m_charImg;
}

void CFontChar::CreateFromImage(int ch, cImage *image, int xoffset, int yoffset, int advance)
{
	m_isChar  = false;
	m_id      = ch;
	m_width   = image->width;
	m_height  = image->height;
	m_advance = m_width + advance;
	m_xoffset = xoffset;
	m_yoffset = yoffset;
	m_colored = true;

	m_charImg = new cImage(image->width, image->height);
	m_charImg->isTopDown = true;

	// Copy the input image to charImg
	memcpy(m_charImg->pixels, image->pixels, m_width*m_height*4);
}

bool CFontChar::HasOutline()
{
	return m_isChar && m_colored;
}

BYTE CFontChar::GetPixelValue(int x, int y, int encoding)
{
	if( m_isChar )
	{	
		if( encoding == e_one ) return 255;
		if( encoding == e_zero ) return 0;

		// Does the character have an outline?
		if( m_colored )
		{
			DWORD color = m_charImg->pixels[y*m_charImg->width+x];
			if( BYTE(color) )
			{
				if( encoding == e_glyph )
					return (BYTE)color;
				else if( encoding == e_outline )
					return 255;
				else if( encoding == e_glyph_outline )
					return 0x80 | (((BYTE)(color))>>1);
			}
			else
			{
				if( encoding == e_glyph )
					return 0;
				else if( encoding == e_outline )
                    return BYTE(color>>24);
				else if( encoding == e_glyph_outline )
					return BYTE(color>>25);
			}
		}
		else
		{
			// Since the character has no outline we 
			// always return the same value
			return (BYTE)m_charImg->pixels[y*m_charImg->width+x];
		}
	}

	return 0;
}

int CFontChar::DrawInvalidCharGlyph(HFONT font, const CFontGen *gen)
{
	m_id = -1;
	return DrawGlyph(font, 0xFFFF, gen);
}

int CFontChar::DrawChar(HFONT font, int ch, const CFontGen *gen)
{
	m_id = ch;
	return DrawGlyph(font, ch, gen);
}

int CFontChar::DrawGlyphFromOutline(HDC dc, int ch, int fontHeight, int fontAscent, const CFontGen *gen)
{
	// Get the glyph info
	int idx;
	if( gen->IsUsingUnicode() )
		idx = gen->GetUnicodeGlyph(ch);
	else
		idx = ch;

	GLYPHMETRICS gm;
	
	// The DC is already initialized with a transformation matrix, so here we need to use the identity matrix
	MAT2 mat = {{0,1},{0,0},{0,0},{0,1}};

	vector<POINT> points;
	vector<int> polyPointCounts;

	// The code for converting true type outlines to polygons is described in the following article
	// ref: http://support.microsoft.com/kb/87115

	DWORD memSize;
	if( gen->IsUsingUnicode() )
		memSize = GetGlyphOutlineW(dc,idx,GGO_GLYPH_INDEX|GGO_NATIVE|(gen->GetUseHinting()?0:GGO_UNHINTED), &gm, 0, 0, &mat);
	else
		memSize = GetGlyphOutlineA(dc,idx,GGO_NATIVE|(gen->GetUseHinting()?0:GGO_UNHINTED), &gm, 0, 0, &mat);
	if( memSize == GDI_ERROR )
		return -1;

	m_advance = gm.gmCellIncX;

	BYTE *buf = new (std::nothrow) BYTE[memSize];
	if( buf == 0 )
	{
		// Oops, I'm out of memory
		return -2;
	}

	DWORD d;
	if( gen->IsUsingUnicode() )
		d = GetGlyphOutlineW(dc, idx, GGO_GLYPH_INDEX|GGO_NATIVE|(gen->GetUseHinting()?0:GGO_UNHINTED), &gm, memSize, buf, &mat);
	else 
		d = GetGlyphOutlineA(dc, idx, GGO_NATIVE|(gen->GetUseHinting()?0:GGO_UNHINTED), &gm, memSize, buf, &mat);
	if( d == GDI_ERROR )
	{
		delete[] buf;	
		return -1;
	}

	// Always render at 8 times the size then downscale. With this we avoid thin 
	// lines disappearing for small characters, even when we do not use antialiasing
	int scale = 65536 / 8;

	// Determine mininum rectangle
	int64_t minX = 10000<<16;
	int64_t maxX = -10000<<16;
	int64_t minY = 10000<<16;
	int64_t maxY = -10000<<16;
	for( DWORD off = 0; off < memSize; )
	{
		TTPOLYGONHEADER *head = (TTPOLYGONHEADER*)(buf + off);
		head->cb;       // Number of bytes that describe the polygon
		head->pfxStart; // Starting point for the polygon

		int64_t x = (*(int*)&head->pfxStart.x);
		int64_t y = (*(int*)&head->pfxStart.y);

		int polyPointCount = 1;
		POINT pt = {int(x),int(y)};
		points.push_back(pt);
			
		if( x < minX ) minX = x;
		if( x > maxX ) maxX = x;
		if( y < minY ) minY = y;
		if( y > maxY ) maxY = y;

		// The header is followed by N polygon curves (edges)
		DWORD off2 = sizeof(TTPOLYGONHEADER);
		while( off2 < head->cb )
		{
			TTPOLYCURVE *curve = (TTPOLYCURVE*)(buf + off + off2);
			curve->wType; // TT_PRIM_LINE, TT_PRIM_QSPLINE, TT_PRIM_CSPLINE
			curve->cpfx; // number of POINTFX 
			
			if( curve->wType != TT_PRIM_QSPLINE )
			{
				// True type doesn't use cubic bsplines, only quadratic bsplines
				assert( curve->wType == TT_PRIM_LINE );

				for( DWORD n = 0; n < curve->cpfx; n++ )
				{
					x = (*(int*)&curve->apfx[n].x);
					y = (*(int*)&curve->apfx[n].y);

					polyPointCount++;
					POINT pt = {int(x),int(y)};
					points.push_back(pt);

					if( x < minX ) minX = x;
					if( x > maxX ) maxX = x;
					if( y < minY ) minY = y;
					if( y > maxY ) maxY = y;
				}
			}
			else
			{
				for( int n = 0; n < curve->cpfx - 1; n++ )
				{
					int64_t xA = x;
					int64_t yA = y;

					int64_t xB = (*(int*)&curve->apfx[n].x);
					int64_t yB = (*(int*)&curve->apfx[n].y);

					int64_t xC = (*(int*)&curve->apfx[n+1].x);
					int64_t yC = (*(int*)&curve->apfx[n+1].y);

					if( n < curve->cpfx - 2 )
					{
						xC = (xB + xC)/2;
						yC = (yB + yC)/2;
					}

					// Step through the quadratic bspline
					for( int64_t ti = 1; ti <= 100; ti++ )
					{
						int64_t t = ti*65536/100;
						int64_t t2 = t/256*t/256;
						x = (xA-2*xB+xC)/256*t2/256 + (2*xB-2*xA)/256*t/256 + xA;
						y = (yA-2*yB+yC)/256*t2/256 + (2*yB-2*yA)/256*t/256 + yA;

						polyPointCount++;
						POINT pt = {int(x),int(y)};
						points.push_back(pt);

						if( x < minX ) minX = x;
						if( x > maxX ) maxX = x;
						if( y < minY ) minY = y;
						if( y > maxY ) maxY = y;
					}

					// Update current pos
					x = xC;
					y = yC;
				}
			}

			// Move to next polygon curve
			off2 += sizeof(TTPOLYCURVE) + sizeof(POINTFX)*(curve->cpfx-1);
		}

		polyPointCounts.push_back(polyPointCount);

		// Move to next polygon
		off += off2;
	}
	
	delete[] buf;

	// TODO: Check if min or max are too large to handle

	if( points.size() == 0 )
	{
		m_width   = 1;
		m_height  = 1;
		m_xoffset = 0;
		m_yoffset = 0;

		m_charImg = new cImage(m_width, m_height);
		m_charImg->isTopDown = true;
		m_charImg->Clear(0);

		return 0;
	}

	// Round boundaries to even pixels
	minX -= scale/2;
	minY -= scale/2;
	maxX += scale/2;
	maxY += scale/2;
	minX &= 0xFFFFFFFFFFFF0000ui64;
	minY &= 0xFFFFFFFFFFFF0000ui64;
	if( maxX & 0xFFFF ) maxX += 0x10000 - (maxX & 0xFFFF);
	if( maxY & 0xFFFF ) maxY += 0x10000 - (maxY & 0xFFFF);

	m_width  = int(maxX/scale - minX/scale);
	m_height = int(maxY/scale - minY/scale);

	// Create the image that will receive the pixels
	m_charImg = new cImage(m_width, m_height);
	if( m_charImg == 0 || m_charImg->pixels == 0 )
	{
		// Oops, I'm out of memory
		return -2;
	}

	m_charImg->isTopDown = true;
	m_charImg->Clear(0);

	// Draw the character
	DWORD *pixels;
	BITMAPINFO bmi;
	ZeroMemory(&bmi, sizeof(BITMAPINFO));
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = m_charImg->width;
	bmi.bmiHeader.biHeight = m_charImg->height;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;         
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biSizeImage = m_charImg->width * m_charImg->height * 4;

	HBITMAP bm = CreateDIBSection(dc, &bmi, DIB_RGB_COLORS, (void**)&pixels, 0, 0);
	if( bm == 0 )
	{
		// Oops, I'm out of memory
		return -2;
	}

	HBITMAP oldBM = (HBITMAP)SelectObject(dc, bm);

	HBRUSH white = (HBRUSH)GetStockObject(WHITE_BRUSH);
	HBRUSH oldBrush = (HBRUSH)SelectObject(dc, white);
	HPEN pen = CreatePen(PS_SOLID, 0, RGB(255,255,255));
	HPEN oldPen = (HPEN)SelectObject(dc, pen);

	memset(pixels, 0, bmi.bmiHeader.biSizeImage);

	// Offset all points so we do not draw outside the box
	m_xoffset = int(minX);
	m_yoffset = int(minY);
	m_xoffset /= scale;
	m_yoffset /= scale;

	for( UINT n = 0; n < points.size(); n++ )
	{
		points[n].x -= scale/2;
		points[n].y -= scale/2;
		points[n].x /= scale;
		points[n].y /= scale;
		points[n].x -= m_xoffset;
		points[n].y -= m_yoffset;
	}

	// Clear the transform before rendering the polygons as 
	// they have already been created with the transform
	if( SetGraphicsMode(dc, GM_ADVANCED) )
	{
		XFORM mtx;
		mtx.eM11 = 1.0f;
		mtx.eM12 = 0;
		mtx.eM21 = 0;
		mtx.eM22 = 1.0f;
		mtx.eDx = 0;
		mtx.eDy = 0;
		SetWorldTransform(dc, &mtx);
	}

	PolyPolygon(dc, &points[0], &polyPointCounts[0], (int)polyPointCounts.size());

	GdiFlush();

	// Retrieve the pixels to the image
	memcpy(m_charImg->pixels, pixels, m_charImg->width*m_charImg->height*4);

	SelectObject(dc, oldBrush);
	SelectObject(dc, oldBM);
	SelectObject(dc, oldPen);
	DeleteObject(pen);
	DeleteObject(bm);

	DownscaleImage(gen->IsUsingSmoothing());
	m_width   /= 8;
	m_height  /= 8;
	m_xoffset /= 8;
	m_yoffset = fontAscent - int(maxY/65536);

	return 0;
}

int CFontChar::DrawGlyphFromBitmap(HDC dc, int ch, int fontHeight, int fontAscent, const CFontGen *gen)
{
/*	// Get the glyph info
	int idx;
	if( gen->IsUsingUnicode() )
	{
		idx = GetUnicodeGlyphIndex(dc, 0, ch);
		if( idx < 0 )
		{
			// Get the default character instead
			TEXTMETRICW tm;
			GetTextMetricsW(dc, &tm);
			WORD glyph;
			fGetGlyphIndicesW(dc, &tm.tmDefaultChar, 1, &glyph, 0);
			idx = glyph;
		}
	}
	else
	{
		idx = ch;
	}
	*/
/*
	Do not use GetGlyphOutline to retrieve the glyph bitmap. 
	- It doesn't prevent clipping of glyphs that go above or below cell height
	- When glyph go outside the cell height the black box doesn't reflect the true size
	- It has less grayscale levels for antialiasing than the TextOut function has
	- ClearType isn't available

	I'm leaving the code here in case I ever want to use it again


	MAT2 mat = {{0,1},{0,0},{0,0},{0,1}};
	GLYPHMETRICS gm;
	DWORD d;
	if( useUnicode )
	{
		d = GetGlyphOutlineW(dc,idx,GGO_GLYPH_INDEX|(useSmoothing ? GGO_GRAY8_BITMAP : GGO_BITMAP),&gm,0,0,&mat);
	}
	else
	{
		d = GetGlyphOutlineA(dc,idx,(useSmoothing ? GGO_GRAY8_BITMAP : GGO_BITMAP),&gm,0,0,&mat);
	}
	if( d != GDI_ERROR )
	{
		// Create the image that will receive the pixels
		m_width = gm.gmBlackBoxX;
		m_height = gm.gmBlackBoxY;
		m_xoffset = gm.gmptGlyphOrigin.x;
		m_advance = gm.gmCellIncX;
		m_yoffset = fontAscent - gm.gmptGlyphOrigin.y;

		// GetGlyphOutline sometimes returns the incorrect height, usually when the
		// glyph have accentual marks very high up. We can calculate the true height 
		// from the buffer size. 

		// Actually, it is not possible to calculate the true height. If the glyph is too far
		// outside the cell height, then the blackbox may not reflect the true width or height
		// thus we do not have anything that can be trusted for recalculating the height. This
		// is why I decided to abandon this way of rendering.

		UINT pitch = m_width;
		if( pitch & 0x3 ) pitch += 4 - (pitch & 0x3);
		if( d / pitch > (unsigned)m_height ) 
		{
			m_yoffset -= d / pitch - m_height;
			m_height = d / pitch;
		}

        // Create the image
		m_charImg = new cImage(m_width, m_height);
		m_charImg->isTopDown = true;
		m_charImg->Clear(0);

		// Get the actual bitmap
		if( d > 0 )
		{
			BYTE *tmpPixels = new BYTE[d];
			if( useUnicode )
				d = GetGlyphOutlineW(dc,idx,GGO_GLYPH_INDEX|(useSmoothing ? GGO_GRAY8_BITMAP : GGO_BITMAP),&gm,d,tmpPixels,&mat);
			else
				d = GetGlyphOutlineA(dc,idx,(useSmoothing ? GGO_GRAY8_BITMAP : GGO_BITMAP),&gm,d,tmpPixels,&mat);

			if( useSmoothing )
			{
				// The above outputs the glyph with 65 levels of gray, so we need to convert this to 256 levels of gray
				for( int y = 0; y < m_charImg->height; y++ )
				{
					for( int x = 0; x < m_charImg->width; x++ )
					{
						BYTE v = 255 * tmpPixels[x+y*pitch] / 64;
						m_charImg->pixels[x+y*m_charImg->width] = (v << 24) | (v << 16) | (v << 8) | v;
					}
				}
			}
			else
			{
				UINT pitch = m_charImg->width / 8 + ((m_charImg->width & 0x7) ? 1 : 0);
				if( pitch & 0x3 ) pitch += 4 - (pitch & 0x3);

				// The above outputs a glyph in a monochrome bitmap
				for( int y = 0; y < m_charImg->height; y++ )
				{
					for( int x = 0; x < m_charImg->width; )
					{
						// Transform each byte into 8 pixels
						for( int bit = 7; bit >= 0 && x < m_charImg->width; bit--, x++ )
						{
							m_charImg->pixels[x+y*m_charImg->width] = ((tmpPixels[x/8+y*pitch] >> bit) & 1) ? 0xFFFFFFFF : 0; 
						}
					}
				}
			}

			delete[] tmpPixels;
		}
	}
	else
*/
	{
		// GetGlyphOutline only works for true type fonts, so we need a fallback for other fonts

		// Determine the size needed for the char
		ABC abc;
		if( gen->IsUsingUnicode() )
		{
			if( GetGlyphABCWidths(dc, 0, gen->GetUnicodeGlyph(ch), &abc) < 0 )
				memset(&abc, 0, sizeof(abc));

			m_width = int(abc.abcB);
		}
		else
		{
			if( GetCharABCWidths(dc, ch, ch, &abc) )
			{
				m_width = int(abc.abcB);
			}
			else
			{
				// Use GetCharWidth32() instead
				GetCharWidth32(dc, ch, ch, &m_width);

				abc.abcA = abc.abcC = 0;
				abc.abcB = (unsigned)m_width;
			}
		}

		// If the requested font size is too large, the Windows API has a
		// bug that causes negative width to be returned in some cases
		if( m_width < 0 )
			m_width = 0;

		m_height = fontHeight;
		m_xoffset = abc.abcA;
		m_advance = (abc.abcA + m_width + abc.abcC);
		m_yoffset = 0;

		if( m_width == 0 )
		{
			m_width   = 1;
			m_height  = 1;
			m_xoffset = 0;
			m_yoffset = 0;

			m_charImg = new cImage(m_width, m_height);
			m_charImg->isTopDown = true;
			m_charImg->Clear(0);

			return 0;
		}

		// We need to add extra width, because width received from GDI 
		// doesn't always account for overhang, e.g. due to italic style
		UINT extraWidth = m_width;
		m_width += extraWidth*2;
		m_xoffset -= extraWidth;

		// Create the image that will receive the pixels
		m_charImg = new cImage(m_width, m_height);
		if( m_charImg == 0 || m_charImg->pixels == 0 )
		{
			// Oops, I'm out of memory
			return -2;
		}
		m_charImg->isTopDown = true;
		m_charImg->Clear(0);

		// Draw the character
		DWORD *pixels;
		BITMAPINFO bmi;
		ZeroMemory(&bmi, sizeof(BITMAPINFO));
		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth = m_charImg->width;
		bmi.bmiHeader.biHeight = -m_charImg->height;
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = 32;         
		bmi.bmiHeader.biCompression = BI_RGB;
		bmi.bmiHeader.biSizeImage = m_charImg->width * m_charImg->height * 4;

		HBITMAP bm = CreateDIBSection(dc, &bmi, DIB_RGB_COLORS, (void**)&pixels, 0, 0);
		if( bm == 0 )
		{
			// Oops, I'm out of memory
			return -2;
		}

		HBITMAP oldBM = (HBITMAP)SelectObject(dc, bm);

		memset(pixels, 0, bmi.bmiHeader.biSizeImage);

		SetTextColor(dc, RGB(255,255,255));
		SetBkColor(dc, RGB(0,0,0));
		SetBkMode(dc, TRANSPARENT);

		if( gen->IsUsingUnicode() )
		{
			WCHAR buf[2];
			int length = acUtility::EncodeUTF16(ch, (unsigned char*)buf, 0);

			// GetCharacterPlacement appears to be buggy, and doesn't always 
			// produce accurate result. Instead we'll find the glyph index on 
			// our own.
			WCHAR glyphs[2] = { 0 };
			glyphs[0] = gen->GetUnicodeGlyph(ch);

			// Use ExtTextOut instead of TextOut to avoid 
			// internal language specific processing done by TextOut
			ExtTextOutW(dc, extraWidth - abc.abcA, 0, ETO_GLYPH_INDEX, NULL, glyphs, 1, NULL);

/*
			// Testing different ways of rendering
			// All of them have the same problem of clipping glyphs that go above or below the cell

			// I'm leaving the code here for reference, if anyone wants to know how to draw text with these other functions

			// Ordinary GDI calls
			RECT rc;
			rc.left = 0;
			rc.top = 0;
			rc.right = fontHeight*5;
			rc.bottom = fontHeight*5;
			//ExtTextOutW(dc, -abc.abcA+fontHeight*2, +fontHeight*2, ETO_CLIPPED|ETO_GLYPH_INDEX, &rc, buf, length/2, 0);
			//DrawTextExW(dc, buf, length/2, &rc, DT_CENTER|DT_NOCLIP|DT_SINGLELINE|DT_VCENTER, 0); 

			// Uniscribe call, complex
			SCRIPT_ITEM si[21];
			int csi;
			SCRIPT_CACHE sc = 0;
			WORD glyphs[20];
			WORD clusters[2];
			SCRIPT_VISATTR va[20];
			int cGlyphs;
			int advance[20];
			GOFFSET offsets[20];
			ABC abc;
			ScriptItemize(buf, length/2, 20, 0, 0, si, &csi);
			ScriptShape(dc, &sc, buf, length/2, 20, &si[0].a, glyphs, clusters, va, &cGlyphs);
			ScriptPlace(dc, &sc, glyphs, cGlyphs, va, &si[0].a, advance, offsets, &abc);
			ScriptTextOut(dc, &sc, fontHeight*2, fontHeight*2, 0, 0, &si[0].a, 0, 0, glyphs, cGlyphs, advance, 0, offsets);
			ScriptFreeCache(&sc);

			// Uniscribe call, simpler
			SCRIPT_STRING_ANALYSIS ssa;
			ScriptStringAnalyse(dc, buf, length/2, 20, -1, SSA_GLYPHS, 0, 0, 0, 0, 0, 0, &ssa);
			ScriptStringOut(ssa, fontHeight*2, fontHeight*2, 0, 0, 0, 0, 0);
			ScriptStringFree(&ssa);
*/
		}
		else
			TextOutA(dc, extraWidth-abc.abcA, 0, (char*)&ch, 1);

		GdiFlush();

		// Retrieve the pixels to the image
		memcpy(m_charImg->pixels, pixels, m_charImg->width*m_charImg->height*4);

		// Clean up
		SelectObject(dc, oldBM);
		DeleteObject(bm);

		if( gen->GetUseClearType() )
		{
			// Need to convert the red and blue levels to grayscale
			for( int y = 0; y < m_charImg->height; y++ )
			{
				for( int x = 0; x < m_charImg->width; x++ )
				{
					UINT c = m_charImg->pixels[y*m_charImg->width + x];
					c = (c&0xFF) + ((c>>8)&0xFF) + ((c>>16)&0xFF);
					c /= 3;
					c = c + (c<<8) + (c<<16);
					m_charImg->pixels[y*m_charImg->width + x] = c;
				}
			}
		}
	}

	return 0;
}

void CFontChar::TrimLeftAndRight()
{
	// Remove excessive width
	int left = -1;
	for( int x = 0; x < m_charImg->width; x++ )
	{
		for( int y = 0; y < m_charImg->height; y++ )
		{
			UINT c = m_charImg->pixels[y*m_charImg->width + x];
			if( c )
			{
				left = x;
				break;
			}
		}
		if( left >= 0 )
			break;
	}

	int right = 0;
	for( int x = m_charImg->width-1; x > 0; x-- )
	{
		for( int y = 0; y < m_charImg->height; y++ )
		{
			UINT c = m_charImg->pixels[y*m_charImg->width + x];
			if( c )
			{
				right = x;
				break;
			}
		}
		if( right > 0 )
			break;
	}

	// Copy the smaller image
	if( left >= 0 )
	{
		cImage *img = new cImage(right-left+1, m_height);
		for( int y = 0; y < m_charImg->height; y++ )
			for( int x = 0; x < img->width; x++ )
				img->pixels[y*img->width + x] = m_charImg->pixels[y*m_charImg->width + x + left];

		delete m_charImg;
		m_charImg = img;

		m_width = img->width;
		m_xoffset += left;
	}
}

int CFontChar::DrawGlyph(HFONT font, int ch, const CFontGen *gen)
{
	m_colored = false;
	m_isChar  = true;

	// Create a memory dc
	HDC dc = CreateCompatibleDC(0);

	// Set the font
	HFONT oldFont = (HFONT)SelectObject(dc, font);

	// We need to determine text metrics before applying the world transform, because 
	// the returned text metrics with transform is not consistent. The tmHeight for example
	// is always the same, independently of the scale, but the tmAscent varies slightly,
	// though not proportionally with the scale.
	TEXTMETRIC tm;
	GetTextMetrics(dc, &tm);

	// Compute the height and ascent with scale
	int fontHeight = int(ceilf(tm.tmHeight*float(gen->GetScaleHeight())/100.0f));
	int fontAscent = int(ceilf(tm.tmAscent*float(gen->GetScaleHeight())/100.0f));

	// Scale the coordinate system so that the font is stretched
	if( SetGraphicsMode(dc, GM_ADVANCED) )
	{
		XFORM mtx;
		mtx.eM11 = 1.0f;
		mtx.eM12 = 0;
		mtx.eM21 = 0;
		mtx.eM22 = float(gen->GetScaleHeight())/100.0f;
		mtx.eDx = 0;
		mtx.eDy = 0;
		SetWorldTransform(dc, &mtx);
	}

	int r = -1;
	if( gen->GetRenderFromOutline() )
		r = DrawGlyphFromOutline(dc, ch, fontHeight, fontAscent, gen);
	
	// In case of error fall back to drawing from bitmap
	// Don't fall back in case of out of memory
	if( r < 0 && r != -2 )
		r = DrawGlyphFromBitmap(dc, ch, fontHeight, fontAscent, gen);

	if( r < 0 )
	{
		// Failed to draw the image (probably out of memory)
		SelectObject(dc, oldFont);
		DeleteDC(dc);
		return r;
	}

	// Remove excessive width
	TrimLeftAndRight();

	// Downscale in case of supersampling
	// Must downscale before removing empty lines
	int aa = gen->GetAntiAliasingLevel();
	if( aa > 1 )
	{
		m_width = int(ceilf(float(m_width)/aa));
		m_height = int(ceilf(float(m_height)/aa));
		m_xoffset /= aa;
		m_yoffset /= aa;
		m_advance /= aa;

		cImage img;
		img.Create(m_width, m_height);

		if( aa == 2 )
		{
			for( int y = 0; y < img.height; y++ )
			{
				for( int x = 0; x < img.width; x++ )
				{
					int sy = y*2;
					int c = 0;

					c += m_charImg->pixels[x*2 + sy*m_charImg->width] & 0xFF;
					if( x*2+1 < m_charImg->width ) c += m_charImg->pixels[x*2+1 + sy*m_charImg->width] & 0xFF;

					if( sy+1 < m_charImg->height )
					{
						c += m_charImg->pixels[x*2 + (sy+1)*m_charImg->width] & 0xFF;
						if( x*2+1 < m_charImg->width ) c += m_charImg->pixels[x*2+1 + (sy+1)*m_charImg->width] & 0xFF;
					}

					c /= 4;

					img.pixels[y*img.width + x] = c | (c<<8) | (c<<16) | (c<<24);
				}
			}
		}
		else if( aa == 3 )
		{
			for( int y = 0; y < img.height; y++ )
			{
				for( int x = 0; x < img.width; x++ )
				{
					int sy = y*3;
					int c = 0;

					c += m_charImg->pixels[x*3 + sy*m_charImg->width] & 0xFF;
					if( x*3+1 < m_charImg->width ) c += m_charImg->pixels[x*3+1 + sy*m_charImg->width] & 0xFF;
					if( x*3+2 < m_charImg->width ) c += m_charImg->pixels[x*3+2 + sy*m_charImg->width] & 0xFF;

					if( sy+1 < m_charImg->height )
					{
						c += m_charImg->pixels[x*3+0 + (sy+1)*m_charImg->width] & 0xFF;
						if( x*3+1 < m_charImg->width ) c += m_charImg->pixels[x*3+1 + (sy+1)*m_charImg->width] & 0xFF;
						if( x*3+2 < m_charImg->width ) c += m_charImg->pixels[x*3+2 + (sy+1)*m_charImg->width] & 0xFF;
					}

					if( sy+2 < m_charImg->height )
					{
						c += m_charImg->pixels[x*3+0 + (sy+2)*m_charImg->width] & 0xFF;
						if( x*3+1 < m_charImg->width ) c += m_charImg->pixels[x*3+1 + (sy+2)*m_charImg->width] & 0xFF;
						if( x*3+2 < m_charImg->width ) c += m_charImg->pixels[x*3+2 + (sy+2)*m_charImg->width] & 0xFF;
					}

					c /= 9;

					img.pixels[y*img.width + x] = c | (c<<8) | (c<<16) | (c<<24);
				}
			}
		}
		else if( aa == 4 )
		{
			for( int y = 0; y < img.height; y++ )
			{
				for( int x = 0; x < img.width; x++ )
				{
					int sy = y*4;
					int c = 0;

					c += m_charImg->pixels[x*4 + sy*m_charImg->width] & 0xFF;
					if( x*4+1 < m_charImg->width ) c += m_charImg->pixels[x*4+1 + sy*m_charImg->width] & 0xFF;
					if( x*4+2 < m_charImg->width ) c += m_charImg->pixels[x*4+2 + sy*m_charImg->width] & 0xFF;
					if( x*4+3 < m_charImg->width ) c += m_charImg->pixels[x*4+3 + sy*m_charImg->width] & 0xFF;

					if( sy+1 < m_charImg->height )
					{
						c += m_charImg->pixels[x*4+0 + (sy+1)*m_charImg->width] & 0xFF;
						if( x*4+1 < m_charImg->width ) c += m_charImg->pixels[x*4+1 + (sy+1)*m_charImg->width] & 0xFF;
						if( x*4+2 < m_charImg->width ) c += m_charImg->pixels[x*4+2 + (sy+1)*m_charImg->width] & 0xFF;
						if( x*4+3 < m_charImg->width ) c += m_charImg->pixels[x*4+3 + (sy+1)*m_charImg->width] & 0xFF;
					}

					if( sy+2 < m_charImg->height )
					{
						c += m_charImg->pixels[x*4+0 + (sy+2)*m_charImg->width] & 0xFF;
						if( x*4+1 < m_charImg->width ) c += m_charImg->pixels[x*4+1 + (sy+2)*m_charImg->width] & 0xFF;
						if( x*4+2 < m_charImg->width ) c += m_charImg->pixels[x*4+2 + (sy+2)*m_charImg->width] & 0xFF;
						if( x*4+3 < m_charImg->width ) c += m_charImg->pixels[x*4+3 + (sy+2)*m_charImg->width] & 0xFF;
					}

					if( sy+3 < m_charImg->height )
					{
						c += m_charImg->pixels[x*4+0 + (sy+3)*m_charImg->width] & 0xFF;
						if( x*4+1 < m_charImg->width ) c += m_charImg->pixels[x*4+1 + (sy+3)*m_charImg->width] & 0xFF;
						if( x*4+2 < m_charImg->width ) c += m_charImg->pixels[x*4+2 + (sy+3)*m_charImg->width] & 0xFF;
						if( x*4+3 < m_charImg->width ) c += m_charImg->pixels[x*4+3 + (sy+3)*m_charImg->width] & 0xFF;
					}

					c /= 16;

					img.pixels[y*img.width + x] = c | (c<<8) | (c<<16) | (c<<24);
				}
			}
		}

		// Move the pixels to the charImg member
		m_charImg->width = img.width;
		m_charImg->height = img.height;
		delete[] m_charImg->pixels;
		m_charImg->pixels = img.pixels;
		img.pixels = 0;
	}

	// Adjust the cell height
	if( gen->GetFixedHeight() || gen->GetForceZero() )
	{
		fontHeight = int(ceilf(float(fontHeight)/aa));

		// Expand the image to the full cellheight with empty lines to 
		cImage *tmp = m_charImg;
		m_charImg = new cImage(tmp->width, fontHeight);
		m_charImg->Clear(0);

		// Make sure we don't draw outside the final cell
		if( tmp->height + m_yoffset > fontHeight )
			tmp->height -= tmp->height + m_yoffset - fontHeight;

		for( int y = m_yoffset < 0 ? -m_yoffset : 0; y < tmp->height; y++ )
			for( int x = 0; x < tmp->width; x++ )
				m_charImg->pixels[x+(y+m_yoffset)*m_charImg->width] = tmp->pixels[x+y*tmp->width];

		delete tmp;

		m_height = fontHeight;
		m_yoffset = 0;
	}
	else
	{
		int removedLines = 0;

		// Discount scanlines that are not drawn
		for( int y = 0; m_height > 1 && y < m_charImg->height; y++ )
		{
			bool empty = true;
			for( int x = 0; x < m_charImg->width; x++ )
			{
				if( m_charImg->pixels[y*m_charImg->width+x] != 0 )
				{
					empty = false;
					break;
				}
			}

			if( empty )
			{
				m_height--;
				m_yoffset++;
				removedLines++;
			}
			else
				break;
		}

		// Discount scanlines that are not drawn
		for( int y = m_charImg->height-1; m_height > 1; y-- )
		{
			bool empty = true;
			for( int x = 0; x < m_charImg->width; x++ )
			{
				if( m_charImg->pixels[y*m_charImg->width+x] != 0 )
				{
					empty = false;
					break;
				}
			}

			if( empty )
			{
				m_height--;
			}
			else
				break;
		}

		// Remove the empty scanlines
		if( removedLines )
		{
			for( int y = 0; y < m_height; y++ )
				for( int x = 0; x < m_width; x++ )
					m_charImg->pixels[y*m_charImg->width+x] = m_charImg->pixels[(y+removedLines)*m_charImg->width+x];			
		}

		m_charImg->height = m_height;
	}

	// Adjust offsets and xadvance
	if( gen->GetForceZero() )
	{
		int leftX = 0;
		if( m_xoffset > 0 )
		{
			// add empty columns to the left
			leftX = m_xoffset;
			m_width += m_xoffset;
		}
		m_xoffset = 0;

		int rightX = 0;
		if( m_advance < m_width )
			m_advance = m_width;
		else if( m_advance > m_width )
		{
			// add empty columns to the right
			rightX = m_advance - m_width;
			m_width += rightX;
		}

		if( leftX || rightX )
		{
			cImage *cpy = new cImage(m_width, m_height);
			cpy->Clear(0);
			for( int y = 0; y < m_charImg->height; y++ )
			{
				for( int x = 0; x < m_charImg->width; x++ )
				{
					cpy->pixels[leftX + x + y*cpy->width] = m_charImg->pixels[x + y*m_charImg->width];
				}
			}
			delete m_charImg;
			m_charImg = cpy;
		}
	}

	// Clean up
	SelectObject(dc, oldFont);
	DeleteDC(dc);

	return 0;
}

void CFontChar::DownscaleImage(bool useSmoothing)
{
	cImage img;
	img.Create(m_charImg->width/8, m_charImg->height/8);

	// The image will always composed of 8x8 blocks
	assert( (m_charImg->width & 0x7) == 0 );
	assert( (m_charImg->height & 0x7) == 0 );

	for( int y = 0; y < img.height; y++ )
	{
		for( int x = 0; x < img.width; x++ )
		{
			int sy = y*8;
			int c = 0;

			for( int i = 0; i < 8; i++ )
			{
				c += m_charImg->pixels[x*8+0 + (sy+i)*m_charImg->width] & 0xFF;
				c += m_charImg->pixels[x*8+1 + (sy+i)*m_charImg->width] & 0xFF;
				c += m_charImg->pixels[x*8+2 + (sy+i)*m_charImg->width] & 0xFF;
				c += m_charImg->pixels[x*8+3 + (sy+i)*m_charImg->width] & 0xFF;
				c += m_charImg->pixels[x*8+4 + (sy+i)*m_charImg->width] & 0xFF;
				c += m_charImg->pixels[x*8+5 + (sy+i)*m_charImg->width] & 0xFF;
				c += m_charImg->pixels[x*8+6 + (sy+i)*m_charImg->width] & 0xFF;
				c += m_charImg->pixels[x*8+7 + (sy+i)*m_charImg->width] & 0xFF;
			}

			c /= 64;

			if( !useSmoothing )
				c = (c >= 150) ? 255 : 0;

			img.pixels[y*img.width + x] = c | (c<<8) | (c<<16) | (c<<24);
		}
	}

	// Move the pixels to the charImg member
	m_charImg->width = img.width;
	m_charImg->height = img.height;
	delete[] m_charImg->pixels;
	m_charImg->pixels = img.pixels;
	img.pixels = 0;
}

void CFontChar::AddOutline(int thickness)
{
	if( !m_charImg->height || !m_charImg->width )
		return;

	m_colored = true;

	m_width  += thickness*2;
	m_height += thickness*2;
	m_xoffset -= thickness;
	m_yoffset -= thickness;

	cImage img;
	img.Create(m_charImg->width+2*thickness, m_charImg->height+2*thickness);
	img.Clear(0);

	// Create the kernel
	int kernelWidth = thickness*2+1;
	float *kernel = new float[kernelWidth*kernelWidth];

	// Circular kernel with anti-aliasing
	for( int y = 0; y < kernelWidth; y++ )
	{
		for( int x = 0; x < kernelWidth; x++ )
		{
			float val;
			if( x == thickness || y == thickness )
				val = 1;
			else 
			{
				val = thickness+1 - thickness*float((x-thickness)*(x-thickness)+(y-thickness)*(y-thickness))/(thickness*thickness);
				if( val > 1 ) val = 1;
				else if( val < 0 ) val = 0;
			}
			kernel[y*kernelWidth+x] = val;
		}
	}

	// Create the outline
	for( int y1 = 0; y1 < m_charImg->height; y1++ )
	{
		for( int x1 = 0; x1 < m_charImg->width; x1++ )
		{
			DWORD cs = m_charImg->pixels[y1*m_charImg->width+x1] & 0xFF;
			for( int y2 = 0; y2 < kernelWidth; y2++ )
			{
				for( int x2 = 0; x2 < kernelWidth; x2++ )
				{
					if( x2 == thickness && y2 == thickness )
					{
						if( cs )
							img.pixels[(y1+y2)*img.width+(x1+x2)] = 0xFF000000|(cs<<16)|(cs<<8)|cs;
					}
					else
					{
						DWORD val = DWORD(cs*kernel[y2*kernelWidth+x2])<<24;
						DWORD cd = img.pixels[(y1+y2)*img.width+(x1+x2)];
						if( val > cd )
							img.pixels[(y1+y2)*img.width+(x1+x2)] = val;
					}
				}
			}
		}
	}

	delete[] kernel;

	// Move the new image to charImg
	m_charImg->width = img.width;
	m_charImg->height = img.height;
	delete[] m_charImg->pixels;
	m_charImg->pixels = img.pixels;
	img.pixels = 0;
}