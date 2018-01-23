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

#ifndef UNICODE_H
#define UNICODE_H

#include <string>
#include <vector>
#include <map>
#include <Usp10.h>
using std::string;
using std::vector;
class CFontGen;

// Interesting links
//
// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/intl/unicode_63ub.asp
// http://unicode.org/charts/
// http://www.i18nguy.com/surrogates.html
// http://msdn2.microsoft.com/en-us/library/ms776414.aspx
// http://www.code2000.net
// http://www.mihai-nita.net/article.php?artID=charmapex
// http://www.microsoft.com/typography/otspec/cmap.htm

struct UnicodeSubset_t
{
	const char *name;
	unsigned int beginChar;
	unsigned int endChar;
};

extern const UnicodeSubset_t UnicodeSubsets[];
extern const int numUnicodeSubsets;

string GetCharSetName(int charSet);
int GetCharSet(const char *charSetName);
int GetSubsetFromChar(unsigned int chr);

int GetGlyphABCWidths(HDC dc, SCRIPT_CACHE *sc, UINT glyph, ABC *abc);

void GetKerningPairsFromGPOS(HDC dc, vector<KERNINGPAIR> &pairs, vector<UINT> &chars, const CFontGen *gen);
void GetKerningPairsFromKERN(HDC dc, vector<KERNINGPAIR> &pairs, vector<UINT> &chars, const CFontGen *gen);
int EnumTrueTypeCMAP(HDC dc, std::map<unsigned int, unsigned int> &unicodeToGlyph);
int EnumUnicodeGlyphs(HDC dc, std::map<unsigned int, unsigned int> &unicodeToGlyph);

#define TAG(a,b,c,d) ((a) | ((b) << 8) | ((c) << 16) | ((d) << 24))
#define SWAP32(x) ((((x)&0xFF)<<24)|(((x)&0xFF00)<<8)|(((x)&0xFF0000)>>8)|((x>>24)&0xFF))
#define SWAP16(x) ((((x)&0xFF)<<8)|((x>>8)&0xFF))

#define GETUSHORT(x) WORD(SWAP16(*(WORD*)(x)))
#define GETSHORT(x)  short(SWAP16(*(WORD*)(x)))
#define GETUINT(x)   DWORD(SWAP32(*(DWORD*)(x)))
#define GETINT(x)    int(SWAP32(*(DWORD*)(x)))

std::string GetFontFileName(const std::string &faceName, bool bold, bool italic);
void ConvertWCharToUtf8(const WCHAR *buf, std::string &utf8);
void ConvertUtf8ToWChar(const std::string &utf8, WCHAR *buf, size_t bufSize);

#endif