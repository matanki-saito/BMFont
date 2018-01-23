/*
   AngelCode Bitmap Font Generator
   Copyright (c) 2004-2016 Andreas Jonsson
  
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

#ifndef FONTGEN_H
#define FONTGEN_H

#include <windows.h>

#include <string>
using std::string;
#include <vector>
using std::vector;
#include <map>
using std::map;

#include "fontpage.h"

static const int maxUnicodeChar = 0x10FFFF;
class CFontChar;

struct SSubset
{
	SSubset() {charBegin = 0; charEnd = 0; available = false; selected = -1;}

	string name;
	int    charBegin;
	int    charEnd;
	bool   available;
	int    selected;
};

struct SIconImage
{
	SIconImage() {image = 0;}
	~SIconImage() {if( image ) delete image;}
	string  fileName;
	int     id;
	cImage *image;
	int     xoffset;
	int     yoffset;
	int     advance;
};

enum EChnlValues
{
	e_glyph,
	e_outline,
	e_glyph_outline,
	e_zero,
	e_one,
};

class CFontGen
{
public:
	CFontGen();
	~CFontGen();

	// Unicode subsets
	unsigned int   GetNumUnicodeSubsets();
	const SSubset *GetUnicodeSubset(unsigned int set);

	// Generate font pages asynchronously
	int     GeneratePages(bool async = true);
	void    Abort();
	int     GetStatus();
	int     GetStatusCounter();
	int     GetError();
	void    ClearError();

	// Icon images
	int     AddIconImage(const char *file, int id, int xoffset, int yoffset, int advance);
	int     GetIconImageCount();
	int     GetIconImageInfo(int n, string &filename, int &id, int &xoffset, int &yoffset, int &advance);
	int     DeleteIconImage(int id);
	int     UpdateIconImage(int oldId, int id, const char *file, int xoffset, int yoffset, int advance);
	void    ClearIconImages();
	bool    IsImage(int id);

	// Select individual characters
	bool    IsSelected(int charIdx);
	int     SetSelected(int charIdx, bool set);
	int     SelectSubset(int subset, bool set);
	int     ClearAll();
	int     IsSubsetSelected(int subset);
	bool    IsDisabled(int charIdx);
	bool    DidNotFit(int charIdx);
	bool    IsOutputInvalidCharGlyphSet()  const;
	int     SetOutputInvalidCharGlyph(bool set);
	bool    GetDontIncludeKerningPairs() const;
	int     SetDontIncludeKerningPairs(bool set);
	int     GetNumCharsSelected();
	int     GetNumCharsAvailable();
	int     SelectCharsFromFile(const char *filename);
	bool    DoesUnicodeCharExist(unsigned int ch) const;
	int     GetUnicodeGlyph(unsigned int ch) const;
	int     GetNonUnicodeGlyph(unsigned int ch) const;

	// Failed characters
	int     GetNumFailedChars();
	int     FindNextFailedCharacterSubset(int startSubset);
	void    ClearFailedCharacters();
	int     SubsetFromChar(int ch);

	// Font properties
	string  GetFontName() const;           int SetFontName(const string &name);
	string  GetFontFile() const;           int SetFontFile(const string &file);
	int     GetCharSet() const;            int SetCharSet(int charSet);
	int     GetFontSize() const;           int SetFontSize(int fontSize);
	bool    IsBold() const;                int SetBold(bool set);
	bool    IsItalic() const;              int SetItalic(bool set);
	int     GetAntiAliasingLevel() const;  int SetAntiAliasingLevel(int level);
	bool    IsUsingSmoothing() const;      int SetUseSmoothing(bool set);
	bool    IsUsingUnicode() const;        int SetUseUnicode(bool set);
	bool    GetRenderFromOutline() const;  int SetRenderFromOutline(bool set);
	bool    GetUseHinting() const;         int SetUseHinting(bool set);
	bool    GetUseClearType() const;       int SetUseClearType(bool set);

	// Character padding and spacing
	int     GetPaddingDown() const;        int SetPaddingDown(int pad);
	int     GetPaddingUp() const;          int SetPaddingUp(int pad);
	int     GetPaddingLeft() const;        int SetPaddingLeft(int pad);
	int     GetPaddingRight() const;       int SetPaddingRight(int pad);
	int     GetSpacingHoriz() const;       int SetSpacingHoriz(int space);
	int     GetSpacingVert() const;        int SetSpacingVert(int space);
	int     GetScaleHeight() const;        int SetScaleHeight(int scale);
	bool    GetFixedHeight() const;        int SetFixedHeight(bool fixed);
	bool    GetForceZero() const;          int SetForceZero(bool force);
									
	// Output font file
	int     GetOutWidth() const;           int SetOutWidth(int width);
	int     GetOutHeight() const;          int SetOutHeight(int height);
	int     GetOutBitDepth() const;        int SetOutBitDepth(int bitDepth);
	int     GetFontDescFormat() const;     int SetFontDescFormat(int format);
	bool    Is4ChnlPacked() const;         int Set4ChnlPacked(bool set);
	string  GetTextureFormat() const;      int SetTextureFormat(string &format);
	int     GetTextureCompression() const; int SetTextureCompression(int compression);
	int     GetAlphaChnl() const;          int SetAlphaChnl(int value);
	int     GetRedChnl() const;            int SetRedChnl(int value);
	int     GetGreenChnl() const;          int SetGreenChnl(int value);
	int     GetBlueChnl() const;           int SetBlueChnl(int value);
	bool    IsAlphaInverted() const;       int SetAlphaInverted(bool set);
	bool    IsRedInverted() const;         int SetRedInverted(bool set);
	bool    IsGreenInverted() const;       int SetGreenInverted(bool set);
	bool    IsBlueInverted() const;        int SetBlueInverted(bool set);

	// Outline
	int     GetOutlineThickness() const;   int SetOutlineThickness(int thickness);

	// Call this after updating the font properties
	int     Prepare();

	// A helper function for creating the font object
	HFONT   CreateFont(int fontSize) const;

	// Visualize pages
	int     GetNumPages();
	cImage *GetPageImage(int page, int channel);

	// Save the font to disk
	int     SaveFont(const char *filename);

	// Configuration
	int     SaveConfiguration(const char *filename);
	int     LoadConfiguration(const char *filename);
	string  GetLastConfigFile() const;

protected:
	friend class CFontPage;

	void ResetFont();
	void ClearPages();
	int  CreatePage();
	void ClearSubsets();
	void DetermineExistingChars();

	static void __cdecl GenerateThread(CFontGen *fontGen);
	void InternalGeneratePages();

	bool fontChanged;

	bool isWorking;
	bool stopWorking;
	int  status;
	bool outOfMemory;
	int  counter;
	bool disableBoxChars;
	bool outputInvalidCharGlyph;
	bool arePagesGenerated;
	bool dontIncludeKerningPairs;

	// Font properties
	string fontName;
	string fontFile;
	int    charSet;
	int    fontSize;
	int    aa;
	int    scaleH;
	bool   useSmoothing;
	bool   isBold;
	bool   isItalic;
	bool   useUnicode;
	bool   renderFromOutline;
	bool   useHinting;
	bool   useClearType;

	// Char alignment options
	int  paddingDown;
	int  paddingUp;
	int  paddingRight;
	int  paddingLeft;	
	int  spacingHoriz;
	int  spacingVert;
	bool fixedHeight;
	bool forceZero;

	// File output options
	int    outWidth;
	int    outHeight;
	int    outBitDepth;
	int    fontDescFormat;
	bool   fourChnlPacked;
	string textureFormat;
	int    textureCompression;
	int    alphaChnl;
	int    redChnl;
	int    greenChnl;
	int    blueChnl;
	bool   invA;
	bool   invR;
	bool   invG;
	bool   invB;

	// Outline
	int    outlineThickness;

	// Characters
	int  numCharsSelected;
	int  numCharsAvailable;
	bool disabled[maxUnicodeChar+1];
	bool selected[maxUnicodeChar+1];
	bool noFit[maxUnicodeChar+1];
	CFontChar *chars[maxUnicodeChar+1];
	CFontChar *invalidCharGlyph;

	// Font textures
	vector<CFontPage *> pages;

	// Icon images
	vector<SIconImage *> iconImages;

	// Available character subsets
	vector<SSubset *> subsets;
	unsigned int lastFoundSubset;

	// Character to glyph mapping
	map<unsigned int, unsigned int> unicodeToGlyph;

	// Font config
	string fontConfigFile;
};

#endif