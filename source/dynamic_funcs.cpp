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

#include <windows.h>
#include "dynamic_funcs.h"

// Since this code requires Win2000 or later, we'll load it dynamically
static HMODULE dll_gdi32 = 0;

GetGlyphIndicesA_t     fGetGlyphIndicesA     = 0;
GetGlyphIndicesW_t     fGetGlyphIndicesW     = 0;
GetFontUnicodeRanges_t fGetFontUnicodeRanges = 0;

void Init()
{
#ifdef LOAD_GDI32
	dll_gdi32 = LoadLibrary("gdi32.dll");
	if( dll_gdi32 != 0 ) 
	{
		fGetGlyphIndicesA     = (GetGlyphIndicesA_t)GetProcAddress(dll_gdi32, "GetGlyphIndicesA");
		fGetGlyphIndicesW     = (GetGlyphIndicesW_t)GetProcAddress(dll_gdi32, "GetGlyphIndicesW");
		fGetFontUnicodeRanges = (GetFontUnicodeRanges_t)GetProcAddress(dll_gdi32, "GetFontUnicodeRanges");
	}
#else
	fGetGlyphIndicesA     = GetGlyphIndicesA;
	fGetGlyphIndicesW     = GetGlyphIndicesW;
	fGetFontUnicodeRanges = GetFontUnicodeRanges;
#endif
}

void Uninit()
{
#ifdef LOAD_GDI32
	if( dll_gdi32 != 0 )
		FreeLibrary(dll_gdi32);
#endif
}