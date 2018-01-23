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

#ifndef DYNAMIC_FUNCS_H
#define DYNAMIC_FUNCS_H

#ifndef GGI_MARK_NONEXISTING_GLYPHS

#define GGI_MARK_NONEXISTING_GLYPHS  0X0001

typedef struct tagWCRANGE {
  WCHAR  wcLow;
  USHORT cGlyphs;
} WCRANGE;

typedef struct tagGLYPHSET {
  DWORD    cbThis;
  DWORD    flAccel;
  DWORD    cGlyphsSupported;
  DWORD    cRanges;
  WCRANGE  ranges[1];
} GLYPHSET;

// Load the functions from GDI32
#define LOAD_GDI32

#endif

typedef DWORD (_stdcall *GetGlyphIndicesA_t)(HDC hdc, LPCSTR lpstr, int c, LPWORD pgi, DWORD fl);
typedef DWORD (_stdcall *GetGlyphIndicesW_t)(HDC hdc, LPCWSTR lpstr, int c, LPWORD pgi, DWORD fl);
typedef DWORD (_stdcall *GetFontUnicodeRanges_t)(HDC hdc, GLYPHSET *gs);

extern GetGlyphIndicesA_t fGetGlyphIndicesA;
extern GetGlyphIndicesW_t fGetGlyphIndicesW;
extern GetFontUnicodeRanges_t fGetFontUnicodeRanges;

void Init();
void Uninit();

#endif