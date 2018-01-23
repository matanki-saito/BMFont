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

#include <process.h>
#include <assert.h>
#include <windows.h>
#include <wingdi.h>
#include <math.h>
#include <Usp10.h>
#include <fstream>

#include "acutil_config.h"
#include "dynamic_funcs.h"
#include "ac_string_util.h"
#include "fontgen.h"
#include "fontchar.h"
#include "unicode.h"
#include "acimg.h"
#include "acutil_unicode.h"
#include "acutil_path.h"
#include "acwin_window.h"

using namespace std;
using namespace acWindow;

#define CLR_BORDER 0x007F00ul
#define CLR_UNUSED 0xFF0000ul

CFontGen::CFontGen()
{
	fontChanged       = true;
	isWorking         = false;
	stopWorking       = false;
	status            = 0;
	arePagesGenerated = false;
	outOfMemory       = false;

	fontName               = "Arial";
	charSet                = ANSI_CHARSET;
	fontSize               = 32;
	aa                     = 1;
	useSmoothing           = true;
	useClearType           = true;
	useHinting             = true;
	renderFromOutline      = false;
	isBold                 = false;
	isItalic               = false;
	useUnicode             = true;
	paddingLeft            = 0;
	paddingRight           = 0;
	paddingUp              = 0;
	paddingDown            = 0;
	spacingHoriz           = 1;
	spacingVert            = 1;
	disableBoxChars        = true;
	outputInvalidCharGlyph = false;
	scaleH                 = 100;
	fixedHeight            = false;
	forceZero              = false;

	outWidth           = 256;
	outHeight          = 256;
	outBitDepth        = 8;
	fourChnlPacked     = false;
	textureFormat      = "tga";
	textureCompression = 0;
	fontDescFormat     = 0;

	outlineThickness   = 0;
	alphaChnl = 1;
	redChnl   = 0;
	greenChnl = 0;
	blueChnl  = 0;
	invA = false;
	invR = false;
	invG = false;
	invB = false;

	memset(selected, 0, sizeof(selected));
	memset(chars, 0, sizeof(chars));
	invalidCharGlyph = 0;
}

CFontGen::~CFontGen()
{
	assert(!isWorking);

	ClearSubsets();

	ClearPages();

	ClearIconImages();
}

int CFontGen::GetError()
{
	if( outOfMemory )
		return -1;

	return 0;
}

void CFontGen::ClearError()
{
	outOfMemory = false;
}

void CFontGen::ClearSubsets()
{
	for( unsigned int n = 0; n < subsets.size(); n++ )
	{
		if( subsets[n] )
			delete subsets[n];
  	}
	subsets.resize(0);
}

void CFontGen::ClearIconImages()
{
	assert(!isWorking);
	arePagesGenerated = false;

	for( unsigned int n = 0; n < iconImages.size(); n++ )
	{
		if( iconImages[n] )
			delete iconImages[n];
	}

	iconImages.resize(0);
}

int CFontGen::AddIconImage(const char *file, int id, int xoffset, int yoffset, int advance)
{
	assert(!isWorking);
	arePagesGenerated = false;

	// Load the image file
	acImage::Image rawImg;
	int r = LoadImageFile(file, rawImg);
	if( r < 0 )
		return -r;

	acImage::Image rgbImg;
	acImage::ConvertToARGB(rgbImg, rawImg);

	cImage *image = new cImage(rgbImg.width, rgbImg.height);
	memcpy(image->pixels, rgbImg.data, rgbImg.width*rgbImg.height*4);

	SIconImage *i = new SIconImage;
	i->fileName = file;
	i->id       = id;
	i->image    = image;
	i->xoffset  = xoffset;
	i->yoffset  = yoffset;
	i->advance  = advance;

	iconImages.push_back(i);

	return 0;
}

bool CFontGen::IsImage(int id)
{
	for( int n = 0; n < (int)iconImages.size(); n++ )
		if( iconImages[n]->id == id )
			return true;

	return false;
}

int CFontGen::DeleteIconImage(int id)
{
	assert(!isWorking);
	arePagesGenerated = false;

	// Find the icon image for removal
	bool isFound = false;
	for( int n = 0; n < (signed)iconImages.size(); n++ )
	{
		if( iconImages[n]->id == id )
		{
			isFound = true;

			SIconImage *img = iconImages[n];
			if( n < (signed)iconImages.size() - 1 )
				iconImages[n] = iconImages[iconImages.size()-1];
			iconImages.pop_back();

			delete img;

			break;
		}
	}

	return isFound ? 0 : -1;
}

int CFontGen::UpdateIconImage(int oldId, int id, const char *file, int xoffset, int yoffset, int advance)
{
	assert(!isWorking);
	arePagesGenerated = false;

	// Find the icon image for update
	bool isFound = false;
	for( int n = 0; n < (signed)iconImages.size(); n++ )
	{
		if( iconImages[n]->id == oldId )
		{
			isFound = true;
			iconImages[n]->id = id;
			iconImages[n]->xoffset = xoffset;
			iconImages[n]->yoffset = yoffset;
			iconImages[n]->advance = advance;

			if( iconImages[n]->fileName != file )
			{
				// Load the image file
				acImage::Image rawImg;
				int r = LoadImageFile(file, rawImg);
				if( r < 0 )
					return -r;

				acImage::Image rgbImg;
				acImage::ConvertToARGB(rgbImg, rawImg);

				cImage *image = new cImage(rgbImg.width, rgbImg.height);
				memcpy(image->pixels, rgbImg.data, rgbImg.width*rgbImg.height*4);

				iconImages[n]->fileName = file;
				delete iconImages[n]->image;
				iconImages[n]->image = image;
			}

			break;
		}
	}

	return isFound ? 0 : -1;
}

int CFontGen::GetIconImageCount()
{
	return (int)iconImages.size();
}

int CFontGen::GetIconImageInfo(int n, string &filename, int &id, int &xoffset, int &yoffset, int &advance)
{
	if( n < 0 || n >= (signed)iconImages.size() )
		return -1;

	filename = iconImages[n]->fileName;
	id = iconImages[n]->id;
	xoffset = iconImages[n]->xoffset;
	yoffset = iconImages[n]->yoffset;
	advance = iconImages[n]->advance;

	return 0;
}

void CFontGen::Abort()
{
	stopWorking = true;

	while( isWorking )
		Sleep(10);
}

bool CFontGen::IsUsingUnicode() const
{
	return useUnicode;
}

int CFontGen::SetUseUnicode(bool set)
{
	if( isWorking ) return -1;
	arePagesGenerated = false;

	if( useUnicode != set )
		fontChanged = true;
	useUnicode = set;

	return 0;
}

bool CFontGen::Is4ChnlPacked() const
{
	return fourChnlPacked;
}

int CFontGen::Set4ChnlPacked(bool set)
{
	if( isWorking ) return -1;
	arePagesGenerated = false;

	fourChnlPacked = set;

	return 0;
}

string CFontGen::GetFontFile() const
{
	return fontFile;
}

string CFontGen::GetTextureFormat() const
{
	return textureFormat;
}

int CFontGen::SetTextureFormat(string &format)
{
	textureFormat = format;

	return 0;
}

int CFontGen::GetTextureCompression() const
{
	return textureCompression;
}

int CFontGen::SetTextureCompression(int compression)
{
	textureCompression = compression;

	return 0;
}

int CFontGen::IsSubsetSelected(int subset)
{
	if( subsets[subset]->selected == -1 )
	{
		bool allChecked = true;
		bool someChecked = false;

		for( int n = subsets[subset]->charBegin; n <= subsets[subset]->charEnd; n++ )
		{
			if( !disabled[n] )
			{
				if( selected[n] )
					someChecked = true;
				else
					allChecked = false;
			}
		}

		if( allChecked && someChecked )
			subsets[subset]->selected = 2;
		else if( someChecked )
			subsets[subset]->selected = 1;
		else
			subsets[subset]->selected = 0;
	}

	return subsets[subset]->selected;
}

int CFontGen::SelectSubset(int subset, bool select)
{
	if( isWorking ) return -1;
	arePagesGenerated = false;

	// Set the new selection state
	for( int n = subsets[subset]->charBegin; n <= subsets[subset]->charEnd; n++ )
		SetSelected(n, select);

	return 0;
}

int CFontGen::ClearAll()
{
	if( isWorking ) return -1;
	arePagesGenerated = false;
	
	memset(selected, 0, (maxUnicodeChar+1)*sizeof(bool));
	numCharsSelected = 0;

	// Clear all subset selected flags
	for( unsigned int n = 0; n < subsets.size(); n++ )
		subsets[n]->selected = -1;

	return 0;
}

int CFontGen::GetStatus()
{
	return status;
}

int CFontGen::GetStatusCounter()
{
	return counter;
}

int CFontGen::GetNumCharsSelected()
{
	return numCharsSelected;
}

int CFontGen::GetNumCharsAvailable()
{
	return numCharsAvailable;
}

bool CFontGen::DidNotFit(int charIdx)
{
	return noFit[charIdx];
}

bool CFontGen::IsDisabled(int charIdx)
{
	return disabled[charIdx];
}

bool CFontGen::IsSelected(int charIdx)
{
	return selected[charIdx];
}

bool CFontGen::IsAlphaInverted() const
{
	return invA;
}

bool CFontGen::IsRedInverted() const
{
	return invR;
}

bool CFontGen::IsGreenInverted() const
{
	return invG;
}

bool CFontGen::IsBlueInverted() const
{
	return invB;
}

HFONT CFontGen::CreateFont(int FontSize) const
{
	if( FontSize == 0 ) FontSize = fontSize*aa;
	DWORD quality = useSmoothing ? ANTIALIASED_QUALITY : NONANTIALIASED_QUALITY;
	if( useSmoothing && useClearType )
		quality = CLEARTYPE_QUALITY;

	// Convert the font name from utf8 to utf16 so we can support fontnames with non-ascii characters
	TCHAR buf[1024];
	ConvertUtf8ToTChar(fontName, buf, 1024);
	HFONT font = ::CreateFont(FontSize, 0, 0, 0, isBold ? FW_BOLD : FW_NORMAL, isItalic ? TRUE : FALSE, 0, 0, charSet, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, quality, DEFAULT_PITCH, buf);

	return font;
}


int CFontGen::SetSelected(int idx, bool set)
{
	if( isWorking ) return -1;
	arePagesGenerated = false;

	if( disabled[idx] )
		return -1;

	if( selected[idx] != set )
	{
		selected[idx] = set;
		numCharsSelected += set ? 1 : -1;

		// Clear the cached subset selected flag
		if( useUnicode )
			subsets[SubsetFromChar(idx)]->selected = -1;
		else
			subsets[0]->selected = -1;
	}

	return 0;
}

int CFontGen::Prepare()
{
	if( isWorking ) return -1;
	arePagesGenerated = false;

	// Update some properties that are exclusive
	if( fourChnlPacked && outBitDepth != 32 )
		Set4ChnlPacked(false);

	ResetFont();

	return 0;
}

int CFontGen::SetOutputInvalidCharGlyph(bool set)
{
	if( isWorking ) return -1;
	arePagesGenerated = false;

	if( outputInvalidCharGlyph != set )
		fontChanged = true;

	outputInvalidCharGlyph = set;
	return 0;
}

int CFontGen::SetDontIncludeKerningPairs(bool set)
{
	if( isWorking ) return -1;
	arePagesGenerated = false;

	if( dontIncludeKerningPairs != set )
		fontChanged = true;

	dontIncludeKerningPairs = set;
	return 0;
}

int CFontGen::SetFontDescFormat(int format)
{
	if( isWorking ) return -1;
	arePagesGenerated = false;

	fontDescFormat = format;
	return 0;
}

int CFontGen::SetOutBitDepth(int bitDepth)
{
	if( isWorking ) return -1;
	arePagesGenerated = false;

	if( bitDepth != 8 && bitDepth != 32 )
		bitDepth = 8;

	outBitDepth = bitDepth;
	return 0;
}

int CFontGen::SetOutHeight(int height)
{
	if( isWorking ) return -1;
	arePagesGenerated = false;

	outHeight = height;
	return 0;
}

int CFontGen::SetOutWidth(int width)
{
	if( isWorking ) return -1;
	arePagesGenerated = false;

	outWidth = width;
	return 0;
}

int CFontGen::GetOutlineThickness() const
{
	return outlineThickness;
}

int CFontGen::SetOutlineThickness(int thickness)
{
	if( isWorking ) return -1;
	arePagesGenerated = false;

	outlineThickness = thickness;
	return 0;
}

int CFontGen::SetAlphaChnl(int value)
{
	if( isWorking ) return -1;
	arePagesGenerated = false;

	if( value < 0 || value > 4 ) value = 0;
	alphaChnl = value;
	return 0;
}

int CFontGen::SetRedChnl(int value)
{
	if( isWorking ) return -1;
	arePagesGenerated = false;

	if( value < 0 || value > 4 ) value = 0;
	redChnl = value;
	return 0;
}

int CFontGen::SetGreenChnl(int value)
{
	if( isWorking ) return -1;
	arePagesGenerated = false;

	if( value < 0 || value > 4 ) value = 0;
	greenChnl = value;
	return 0;
}

int CFontGen::SetBlueChnl(int value)
{
	if( isWorking ) return -1;
	arePagesGenerated = false;

	if( value < 0 || value > 4 ) value = 0;
	blueChnl = value;
	return 0;
}

int CFontGen::SetAlphaInverted(bool set)
{
	if( isWorking ) return -1;
	arePagesGenerated = false;

	invA = set;
	return 0;	
}

int CFontGen::SetRedInverted(bool set)
{
	if( isWorking ) return -1;
	arePagesGenerated = false;

	invR = set;
	return 0;	
}

int CFontGen::SetGreenInverted(bool set)
{
	if( isWorking ) return -1;
	arePagesGenerated = false;

	invG = set;
	return 0;	
}

int CFontGen::SetBlueInverted(bool set)
{
	if( isWorking ) return -1;
	arePagesGenerated = false;

	invB = set;
	return 0;	
}

int CFontGen::SetScaleHeight(int scale)
{
	if( isWorking ) return -1;
	arePagesGenerated = false;

	scaleH = scale;
	return 0;
}

int CFontGen::SetSpacingHoriz(int space)
{
	if( isWorking ) return -1;
	arePagesGenerated = false;

	spacingHoriz = space;
	return 0;
}

int CFontGen::SetSpacingVert(int space)
{
	if( isWorking ) return -1;
	arePagesGenerated = false;

	spacingVert = space;
	return 0;
}

int CFontGen::SetPaddingUp(int pad)
{
	if( isWorking ) return -1;
	arePagesGenerated = false;

	paddingUp = pad;
	return 0;
}

int CFontGen::SetPaddingDown(int pad)
{
	if( isWorking ) return -1;
	arePagesGenerated = false;

	paddingDown = pad;
	return 0;
}

int CFontGen::SetPaddingLeft(int pad)
{
	if( isWorking ) return -1;
	arePagesGenerated = false;

	paddingLeft = pad;
	return 0;
}

int CFontGen::SetPaddingRight(int pad)
{
	if( isWorking ) return -1;
	arePagesGenerated = false;

	paddingRight = pad;
	return 0;
}

int CFontGen::SetFixedHeight(bool fixed)
{
	if( isWorking ) return -1;
	arePagesGenerated = false;

	fixedHeight = fixed;
	return 0;
}

int CFontGen::SetForceZero(bool force)
{
	if( isWorking ) return -1;
	arePagesGenerated = false;

	forceZero = force;
	return 0;
}

int CFontGen::SetUseSmoothing(bool set)
{
	if( isWorking ) return -1;
	arePagesGenerated = false;

	useSmoothing = set;
	return 0;
}

int CFontGen::SetRenderFromOutline(bool set)
{
	if( isWorking ) return -1;
	arePagesGenerated = false;

	renderFromOutline = set;
	return 0;
}

int CFontGen::SetUseHinting(bool set)
{
	if( isWorking ) return -1;
	arePagesGenerated = false;

	useHinting = set;
	return 0;
}

int CFontGen::SetUseClearType(bool set)
{
	if( isWorking ) return -1;
	arePagesGenerated = false;

	useClearType = set;
	return 0;
}

int CFontGen::SetAntiAliasingLevel(int level)
{
	if( isWorking ) return -1;
	arePagesGenerated = false;

	aa = level;
	return 0;
}

int CFontGen::SetItalic(bool set)
{
	if( isWorking ) return -1;
	arePagesGenerated = false;

	isItalic = set;
	return 0;
}

int CFontGen::SetBold(bool set)
{
	if( isWorking ) return -1;
	arePagesGenerated = false;

	isBold = set;
	return 0;
}

int CFontGen::SetFontSize(int fontSize)
{
	if( isWorking ) return -1;
	arePagesGenerated = false;

	this->fontSize = fontSize;
	return 0;
}

int CFontGen::SetCharSet(int charSet)
{
	if( isWorking ) return -1;
	arePagesGenerated = false;

	if( this->charSet != charSet )
		fontChanged = true;

	this->charSet = charSet;
	return 0;
}

int CFontGen::SetFontName(const string &name)
{
	if( isWorking ) return -1;
	arePagesGenerated = false;

	if( fontName != name )
		fontChanged = true;

	fontName = name;
	return 0;
}

int CFontGen::SetFontFile(const string &file)
{
	if( isWorking ) return -1;
	arePagesGenerated = false;

	if( fontFile != file )
	{
		// Remove the old font
		if( fontFile != "" )
		{
			TCHAR buf[1024];
			ConvertUtf8ToTChar(fontFile, buf, 1024);
			RemoveFontResourceEx(buf, FR_PRIVATE, 0);
		}

		// Add the new font so it can be used to draw text with 
		TCHAR buf[1024];
		ConvertUtf8ToTChar(file, buf, 1024);
		AddFontResourceEx(buf, FR_PRIVATE, 0);
	}

	fontFile = file;
	return 0;
}

bool CFontGen::IsOutputInvalidCharGlyphSet() const
{
	return outputInvalidCharGlyph;
}

bool CFontGen::GetDontIncludeKerningPairs() const
{
	return dontIncludeKerningPairs;
}

int CFontGen::GetFontDescFormat() const
{
	return fontDescFormat;
}

int CFontGen::GetOutBitDepth() const
{
	return outBitDepth;
}

int CFontGen::GetOutHeight() const
{
	return outHeight;
}

int CFontGen::GetOutWidth() const
{
	return outWidth;
}

int CFontGen::GetAlphaChnl() const
{
	return alphaChnl;
}

int CFontGen::GetRedChnl() const
{
	return redChnl;
}

int CFontGen::GetGreenChnl() const
{
	return greenChnl;
}

int CFontGen::GetBlueChnl() const
{
	return blueChnl;
}

int CFontGen::GetScaleHeight() const
{
	return scaleH;
}

int CFontGen::GetSpacingHoriz() const
{
	return spacingHoriz;
}

int CFontGen::GetSpacingVert() const
{
	return spacingVert;
}

int CFontGen::GetPaddingUp() const
{
	return paddingUp;
}

int CFontGen::GetPaddingDown() const
{
	return paddingDown;
}

int CFontGen::GetPaddingLeft() const
{
	return paddingLeft;
}

int CFontGen::GetPaddingRight() const
{
	return paddingRight;
}

bool CFontGen::GetFixedHeight() const
{
	return fixedHeight;
}

bool CFontGen::GetForceZero() const
{
	return forceZero;
}

bool CFontGen::IsUsingSmoothing() const
{
	return useSmoothing;
}

bool CFontGen::GetRenderFromOutline() const
{
	return renderFromOutline;
}

bool CFontGen::GetUseHinting() const
{
	return useHinting;
}

bool CFontGen::GetUseClearType() const
{
	return useClearType;
}

bool CFontGen::IsItalic() const
{
	return isItalic;
}

bool CFontGen::IsBold() const
{
	return isBold;
}

int CFontGen::GetAntiAliasingLevel() const
{
	return aa;
}

int CFontGen::GetFontSize() const
{
	return fontSize;
}

int CFontGen::GetCharSet() const
{
	return charSet;
}

cImage *CFontGen::GetPageImage(int page, int channel)
{
	pages[page]->GeneratePreviewTexture(channel);
	return pages[page]->GetPageImage();
}

int CFontGen::GetNumPages()
{
	return (int)pages.size();
}

string CFontGen::GetFontName() const
{
	return fontName;
}

unsigned int CFontGen::GetNumUnicodeSubsets()
{
	if( useUnicode )
		return (unsigned int)subsets.size();

	return 0;
}

const SSubset *CFontGen::GetUnicodeSubset(unsigned int set)
{
	if( useUnicode && set < subsets.size() )
		return subsets[set];

	return 0;
}

// Internal
void CFontGen::ResetFont()
{
	if( fontChanged )
	{
		memset(disabled, 0, sizeof(disabled));
		memset(noFit, 0, sizeof(noFit));

		DetermineExistingChars();
	}
	fontChanged = false;
}

// Internal
void CFontGen::DetermineExistingChars()
{
	HDC dc = GetDC(0);

	HFONT font = CreateFont(10);
	HFONT oldFont = (HFONT)SelectObject(dc, font);

	// TODO: This is taking too long. If it is not possible to optimize significantly, then it is necessary
	//       to add a status bar for the user to see the progress while scanning the font for glyphs
	EnumTrueTypeCMAP(dc, unicodeToGlyph);
	if (unicodeToGlyph.empty())
		EnumUnicodeGlyphs(dc, unicodeToGlyph);

	if( fGetGlyphIndicesA )
	{
		numCharsAvailable = 0;
		numCharsSelected = 0;

		if( useUnicode )
		{
			ClearSubsets();
			memset(disabled, 1, (maxUnicodeChar+1)*sizeof(bool));

			// GetGlyphIndices doesn't support surrogate pairs
			// neither does ScriptGetCMap, so we'll have to go the
			// long route and use ScriptItemize and ScriptShape
			SCRIPT_CACHE sc = 0;

			for( int subset = 0; subset < numUnicodeSubsets; subset++ )
			{
				// Unicode subsets that have no defined characters are all disabled
				if( UnicodeSubsets[subset].name[0] == '(' )
					continue;

				unsigned int begin = UnicodeSubsets[subset].beginChar;
				while( begin <= UnicodeSubsets[subset].endChar )
				{
					unsigned int end = begin + 255;
					if( end > UnicodeSubsets[subset].endChar )
						end = UnicodeSubsets[subset].endChar;

					// Create a subset with at most 256 characters
					SSubset *set = new SSubset;
					set->name      = UnicodeSubsets[subset].name;
					set->charBegin = begin;
					set->charEnd   = end;
					subsets.push_back(set);

					// Determine the available characters in this set
					for( unsigned int n = begin; n <= end; n++ )
					{
						bool exists = DoesUnicodeCharExist(n);

						if( !disableBoxChars || exists )
						{
							disabled[n] = false;

							// Mark the subset as available
							set->available = true;

							// Count the number of available characters
							// and update the number of selected ones
							numCharsAvailable++;
							if( selected[n] ) 
								numCharsSelected++;
						}
					}

					// Next 256 characters in the subset
					begin += 256;
				}
			}

			// Clean up the cache created by Uniscribe
			if( sc != 0 )
				ScriptFreeCache(&sc);
		}
		else
		{
			// Create the basic subset
			SSubset *set = new SSubset;
			set->name      = "";
			set->charBegin = 0;
			set->charEnd   = 255;
			subsets.push_back(set);

			memset(disabled, 0, 256*sizeof(bool));

			for( int n = 0; n < 256; n++ )
			{
				char buf[2];
				buf[0] = n;
				buf[1] = '\0';

				WORD idx;
				int r = fGetGlyphIndicesA(dc, buf, 1, &idx, GGI_MARK_NONEXISTING_GLYPHS);

				if( disableBoxChars && (r == GDI_ERROR || idx == 0xFFFF) )
					disabled[n] = true;
				else
				{
					numCharsAvailable++;
					if( selected[n] ) 
						numCharsSelected++;
				}
			}
		}
	}

	SelectObject(dc, oldFont);
	DeleteObject(font);

	ReleaseDC(0, dc);
}

bool CFontGen::DoesUnicodeCharExist(unsigned int ch) const
{
	if (unicodeToGlyph.find(ch) != unicodeToGlyph.end())
		return true;

	return false;
}

// Returns 0 (the default glyph) if the character isn't found
int CFontGen::GetUnicodeGlyph(unsigned int ch) const
{
	auto it = unicodeToGlyph.find(ch);
	if (it == unicodeToGlyph.end())
		return 0;

	return it->second;
}

// Returns 0 (the default glyph) if the character isn't found
int CFontGen::GetNonUnicodeGlyph(unsigned int ch) const
{
	// TODO: This needs to be cached, so the font isn't created with every lookup

	// Translate the char to glyph id
	char str[2] = { (char)ch, '\0' };
	WCHAR glyphs[2];
	HDC dc = GetDC(0);
	HFONT font = CreateFont(16);
	HFONT oldFont = (HFONT)SelectObject(dc, font);
	GCP_RESULTSA result;
	memset(&result, 0, sizeof(GCP_RESULTSA));
	result.lStructSize = sizeof(GCP_RESULTSA);
	result.lpGlyphs = glyphs;
	result.nGlyphs = 2;
	GetCharacterPlacementA(dc, str, 1, 0, &result, 0);
	SelectObject(dc, oldFont);
	DeleteObject(font);

	return result.nGlyphs ? result.lpGlyphs[0] : 0;
}

// Internal
void CFontGen::ClearPages()
{
	for( int n = 0; n < (signed)pages.size(); n++ )
	{
		if( pages[n] ) delete pages[n];
		pages[n] = 0;
	}

	pages.clear();

	for( int n = 0; n < maxUnicodeChar+1; n++ )
	{
		if( chars[n] ) delete chars[n];
		chars[n] = 0;
	}

	if( invalidCharGlyph ) delete invalidCharGlyph;
	invalidCharGlyph = 0;
}

#ifdef TRACE_GENERATE
ofstream trace;
#endif

// Internal
int CFontGen::CreatePage()
{
	CFontPage *page = new CFontPage(this, (int)pages.size(), outWidth, outHeight, spacingHoriz, spacingVert);
	if( page == 0 || !page->IsOK() )
	{
		stopWorking = true;
		outOfMemory = true;

		if( page )
			delete page;

		// Out of memory
#ifdef TRACE_GENERATE
		trace << "Out of memory while allocating memory for page" << endl;
		trace.flush();
#endif

		return -1;
	}

	page->SetPadding(paddingLeft, paddingUp, paddingRight, paddingDown);
	page->SetIntendedFormat(outBitDepth, fourChnlPacked, alphaChnl, redChnl, greenChnl, blueChnl);
	pages.push_back(page);

	return (int)pages.size() - 1;
}

// Internal
void CFontGen::InternalGeneratePages()
{
	if( arePagesGenerated )
	{
		status    = 0;
		isWorking = false;
		return;
	}

#ifdef TRACE_GENERATE
	trace.open("trace_generate.txt");
	trace << "Starting" << endl;
	trace.flush();
#endif

	ClearPages();

	bool didNotFit = false;
	memset(noFit, 0, sizeof(noFit));

	if( stopWorking )
	{
		status    = 0;
		isWorking = false;

#ifdef TRACE_GENERATE
		trace << "Aborted" << endl;
		trace.close();
#endif

		return;
	}

	const int maxChars = useUnicode ? maxUnicodeChar+1 : 256;

#ifdef TRACE_GENERATE
	trace << "There are " << iconImages.size() << " images" << endl;
	trace.flush();
#endif

	// Add the imported images to the character list
	for( int n = 0; n < (signed)iconImages.size(); n++ )
	{
		int ch = iconImages[n]->id;
		chars[ch] = new CFontChar();
		chars[ch]->CreateFromImage(n, iconImages[n]->image, iconImages[n]->xoffset, iconImages[n]->yoffset, iconImages[n]->advance);

#ifdef TRACE_GENERATE
//		trace << "Character [" << ch << "] created from image" << endl;
//		trace.flush();
#endif

		if( chars[ch]->m_height > 0 && chars[ch]->m_width > 0 )
		{
			if( (chars[ch]->m_height + paddingUp + paddingDown) > outHeight-spacingVert || 
				(chars[ch]->m_width + paddingRight + paddingLeft) > outWidth-spacingHoriz )
			{
				didNotFit = true;
				noFit[ch] = true;

				// Delete the character again so that it isn't considered again
				delete chars[ch];
				chars[ch] = 0;

#ifdef TRACE_GENERATE
				trace << "Character [" << ch << "] is too large to fit texture" << endl;
				trace.flush();
#endif
			}
		}

		if( stopWorking )
		{
			status    = 0;
			isWorking = false;

#ifdef TRACE_GENERATE
			trace << "Aborted" << endl;
			trace.close();
#endif
			return;
		}
	}

	// Draw each of the chars into individual images
	HFONT font = CreateFont(0);
	for( int n = 0; n < maxChars; n++ )
	{
		if( !disabled[n] && selected[n] )
		{
			// Unless the image is taken by an imported icon
			// Draw the character in a separate image
			// Determine the dimensions of the character
			if( chars[n] == 0 )
			{
				chars[n] = new CFontChar();
				int r = chars[n]->DrawChar(font, n, this);
				if( r < 0 )
				{
					// The character couldn't be drawn (probably due to out of memory)
					outOfMemory = true;
					delete chars[n];
					chars[n] = 0;
					stopWorking = true;

#ifdef TRACE_GENERATE
					trace << "Out of memory when drawing character [" << n << "]" << endl;
					trace.flush();
#endif
				}
				if( outlineThickness && chars[n] )
					chars[n]->AddOutline(outlineThickness);

#ifdef TRACE_GENERATE
//		trace << "Character [" << n << "] was drawn" << endl;
//		trace.flush();
#endif

				if( chars[n] && chars[n]->m_height > 0 && chars[n]->m_width > 0 )
				{
					if( (chars[n]->m_height + paddingUp + paddingDown) > outHeight-spacingVert || 
						(chars[n]->m_width + paddingRight + paddingLeft) > outWidth-spacingHoriz )
					{
						didNotFit = true;
						noFit[n] = true;	

						// Delete the character again so that it isn't considered again
						delete chars[n];
						chars[n] = 0;

#ifdef TRACE_GENERATE
						trace << "Character [" << n << "] is too large to fit texture" << endl;
						trace.flush();
#endif
					}
				}
			}
			counter++;

			if( stopWorking )
			{
				if( outOfMemory )
				{
					// Free up memory so the user can continue to use the app
					ClearPages();
				}

				status    = 0;
				isWorking = false;
				DeleteObject(font);

#ifdef TRACE_GENERATE
				trace << "Aborted" << endl;
				trace.close();
#endif

				return;
			}
		}
	}

#ifdef TRACE_GENERATE
	trace << counter << " characters were drawn" << endl;
	trace.flush();
#endif

	// Add the invalid char glyph
	if( outputInvalidCharGlyph )
	{
		invalidCharGlyph = new CFontChar();
		int r = invalidCharGlyph->DrawInvalidCharGlyph(font, this);
		if( r < 0 )
		{
			// The character couldn't be drawn (probably due to out of memory)
			outOfMemory = true;
			delete invalidCharGlyph;
			invalidCharGlyph = 0;
			stopWorking = true;

#ifdef TRACE_GENERATE
			trace << "Out of memory when drawing invalid char" << endl;
			trace.flush();
#endif
		}
		if( outlineThickness && invalidCharGlyph )
			invalidCharGlyph->AddOutline(outlineThickness);

#ifdef TRACE_GENERATE
		trace << "Invalid char was drawn" << endl;
		trace.flush();
#endif

		if( invalidCharGlyph &&
			(invalidCharGlyph->m_height + paddingUp + paddingDown) > outHeight-spacingVert || 
			(invalidCharGlyph->m_width + paddingRight + paddingLeft) > outWidth-spacingHoriz )
		{
			didNotFit = true;

			// Delete the character again so that it isn't considered again
			delete invalidCharGlyph;
			invalidCharGlyph = 0;

#ifdef TRACE_GENERATE
			trace << "Invalid char is too large to fit texture" << endl;
			trace.flush();
#endif
		}
	}

	DeleteObject(font);

	// Build a list of used characters
	status = 2;
	counter = 0;

	static CFontChar *ch[maxUnicodeChar+2];
	int numChars = 0;
	for( int n = 0; n < maxChars; n++ )
	{
		if( chars[n] )
			ch[numChars++] = chars[n];
	}

	if( outputInvalidCharGlyph && invalidCharGlyph )
		ch[numChars++] = invalidCharGlyph;

	// Create pages until there are no more chars
	while( numChars > 0 )
	{
#ifdef TRACE_GENERATE
		trace << "There are " << numChars << " left to add to textures" << endl;
		trace.flush();
#endif

		int page = CreatePage();

		if( stopWorking )
		{
			if( outOfMemory )
			{
#ifdef TRACE_GENERATE
				trace << "Clearing memory" << endl;
				trace.flush();
#endif
				// Free up memory so the user can continue to work
				ClearPages();
			}

			status    = 0;
			isWorking = false;

#ifdef TRACE_GENERATE
			trace << "Aborted" << endl;
			trace.close();
#endif

			return;
		}

#ifdef TRACE_GENERATE
		trace << "Page " << page << " was created" << endl;
		trace.flush();
#endif

		pages[page]->AddChars(ch, numChars);

#ifdef TRACE_GENERATE
		trace << "Compacting list of remaining characters" << endl;
		trace.flush();
#endif

		// Compact list
		for( int n = 0; n < numChars; n++ )
		{
			if( ch[n] == 0 )
			{
				// Find the last char
				for( numChars--; numChars > n; numChars-- )
				{
					if( ch[numChars] )
					{
						ch[n] = ch[numChars];
						ch[numChars] = 0;
						break;
					}
				}
			}
		}
	}

	status    = 0;
	isWorking = false;
	arePagesGenerated = true;

#ifdef TRACE_GENERATE
	trace << "Finished" << endl;
	trace.close();
#endif
}

// Internal
void CFontGen::GenerateThread(CFontGen *fontGen)
{
	fontGen->InternalGeneratePages();
}

int CFontGen::GeneratePages(bool async)
{
	if( isWorking ) return -1;

	// Make a final validation of configuration
	{
		// DDS with compression only support 32bit textures
		if( textureFormat == "dds" &&
			textureCompression > 0 &&
			outBitDepth == 8 )
		{
			// Change the format to 32bit with all color channels set to 1
			SetOutBitDepth(32);
			SetRedChnl(e_one);
			SetGreenChnl(e_one);
			SetBlueChnl(e_one);
		}
	}

	// Set status refresh timer
	status = 1;
	counter = 0;

	isWorking         = true;
	stopWorking       = false;

	memset(noFit, 0, sizeof(noFit));

	if( async )
		_beginthread((void (*)(void*))GenerateThread, 0, this);	
	else
		InternalGeneratePages();

	return 0;
}

int CFontGen::SaveFont(const char *szFile)
{
	if( isWorking ) return -1;

	// The pages must be generated first
	if( !arePagesGenerated ) return -1;

	// Create a memory dc
	HDC dc = CreateCompatibleDC(0);

	HFONT font = CreateFont(0);
	HFONT oldFont = (HFONT)SelectObject(dc, font);

	// Determine the size needed for the char
	int height, base;

	TEXTMETRIC tm;
	GetTextMetrics(dc, &tm);

	// Round up to make sure fractional pixels are covered
	height = (int)ceil(float(tm.tmHeight)/aa);
	base = (int)ceil(float(tm.tmAscent)/aa);

	// Save the character attributes
	FILE *f;

	string filename = szFile;
	if( _stricmp(filename.substr(filename.length() - 4).c_str(), ".fnt") == 0 )
		filename = filename.substr(0, filename.length() - 4);

	errno_t e = fopen_s(&f, (filename + ".fnt").c_str(), "wb");
	if( e != 0 || f == 0 )
		return -1;

	// Get the filename without path
	size_t r = filename.rfind('\\');
	string filenameonly;
	if( r != -1 )
		filenameonly = filename.substr(r+1);
	else
		filenameonly = filename;
		
	if( outBitDepth != 32 ) fourChnlPacked = false;
	size_t numPages = pages.size();

	// Determine the number of digits needed for the page file id
	int numDigits = numPages > 1 ? int(log10(float(numPages-1))+1) : 1;

	if( fontDescFormat == 1 ) 
	{
		fprintf(f, "<?xml version=\"1.0\"?>\r\n");
		fprintf(f, "<font>\r\n");
		fprintf(f, "  <info face=\"%s\" size=\"%d\" bold=\"%d\" italic=\"%d\" charset=\"%s\" unicode=\"%d\" stretchH=\"%d\" smooth=\"%d\" aa=\"%d\" padding=\"%d,%d,%d,%d\" spacing=\"%d,%d\" outline=\"%d\"/>\r\n", fontName.c_str(), fontSize, isBold, isItalic, useUnicode ? "" : GetCharSetName(charSet).c_str(), useUnicode, scaleH, useSmoothing, aa, paddingUp, paddingRight, paddingDown, paddingLeft, spacingHoriz, spacingVert, outlineThickness);
		fprintf(f, "  <common lineHeight=\"%d\" base=\"%d\" scaleW=\"%d\" scaleH=\"%d\" pages=\"%d\" packed=\"%d\" alphaChnl=\"%d\" redChnl=\"%d\" greenChnl=\"%d\" blueChnl=\"%d\"/>\r\n", int(ceilf(height*float(scaleH)/100.0f)), int(ceilf(base*float(scaleH)/100.0f)), outWidth, outHeight, int(numPages), fourChnlPacked, alphaChnl, redChnl, greenChnl, blueChnl);

		fprintf(f, "  <pages>\r\n");
		for( size_t n = 0; n < numPages; n++ )
			fprintf(f, "    <page id=\"%d\" file=\"%s_%0*d.%s\" />\r\n", (int)n, filenameonly.c_str(), numDigits, (int)n, textureFormat.c_str());
		fprintf(f, "  </pages>\r\n");
	}
	else if( fontDescFormat == 0 )
	{
		fprintf(f, "info face=\"%s\" size=%d bold=%d italic=%d charset=\"%s\" unicode=%d stretchH=%d smooth=%d aa=%d padding=%d,%d,%d,%d spacing=%d,%d outline=%d\r\n", fontName.c_str(), fontSize, isBold, isItalic, useUnicode ? "" : GetCharSetName(charSet).c_str(), useUnicode, scaleH, useSmoothing, aa, paddingUp, paddingRight, paddingDown, paddingLeft, spacingHoriz, spacingVert, outlineThickness);
		fprintf(f, "common lineHeight=%d base=%d scaleW=%d scaleH=%d pages=%d packed=%d alphaChnl=%d redChnl=%d greenChnl=%d blueChnl=%d\r\n", int(ceilf(height*float(scaleH)/100.0f)), int(ceilf(base*float(scaleH)/100.0f)), outWidth, outHeight, int(numPages), fourChnlPacked, alphaChnl, redChnl, greenChnl, blueChnl);

		for( size_t n = 0; n < numPages; n++ )
			fprintf(f, "page id=%d file=\"%s_%0*d.%s\"\r\n", (int)n, filenameonly.c_str(), numDigits, (int)n, textureFormat.c_str());
	}
	else
	{
		// Write the magic word and file version
		fwrite("BMF", 3, 1, f);
		fputc(3, f); 

		// Write the info block
#pragma pack(push)
#pragma pack(1)
		struct infoBlock
		{
			int            blockSize;
			unsigned short fontSize;
			char           reserved    :4;
			char           bold        :1;
			char           italic      :1;
			char           unicode     :1;
			char           smooth      :1;
			unsigned char  charSet;
			unsigned short stretchH;
			char           aa;
			unsigned char  paddingUp;
			unsigned char  paddingRight;
			unsigned char  paddingDown;
			unsigned char  paddingLeft;
			unsigned char  spacingHoriz;
			unsigned char  spacingVert;
			unsigned char  outline;
			char           fontName[1];
		} info;
#pragma pack(pop)

		info.blockSize    = int(sizeof(info) + fontName.length() - 4);
		info.fontSize     = fontSize;
		info.reserved     = 0;
		info.bold         = isBold;
		info.italic       = isItalic;
		info.unicode      = useUnicode;
		info.smooth       = useSmoothing;
		info.charSet      = charSet;
		info.stretchH     = scaleH;
		info.aa           = aa;
		info.paddingUp    = paddingUp;
		info.paddingRight = paddingRight;
		info.paddingDown  = paddingDown;
		info.paddingLeft  = paddingLeft;
		info.spacingHoriz = spacingHoriz;
		info.spacingVert  = spacingVert;
		info.outline      = outlineThickness;

		fputc(1, f);
		fwrite(&info, sizeof(info)-1, 1, f);
		fwrite(fontName.c_str(), fontName.length()+1, 1, f);

		// Write the common block
#pragma pack(push)
#pragma pack(1)
		struct commonBlock
		{
			int blockSize;
			unsigned short lineHeight;
			unsigned short base;
			unsigned short scaleW;
			unsigned short scaleH;
			unsigned short pages;
			unsigned char  packed:1;
			unsigned char  reserved:7;
			unsigned char  alphaChnl;
			unsigned char  redChnl;
			unsigned char  greenChnl;
			unsigned char  blueChnl;
		} common; 
#pragma pack(pop)

		common.blockSize  = sizeof(common) - 4;
		common.lineHeight = int(ceilf(height*float(scaleH)/100.0f));
		common.base       = int(ceilf(base*float(scaleH)/100.0f));
		common.scaleW     = outWidth;
		common.scaleH     = outHeight;
		common.pages      = unsigned short(numPages);
		common.reserved   = 0;
		common.packed     = fourChnlPacked;
		common.alphaChnl  = alphaChnl;
		common.redChnl    = redChnl;
		common.greenChnl  = greenChnl;
		common.blueChnl   = blueChnl;

		fputc(2, f);
		fwrite(&common, sizeof(common), 1, f);

		// Write the page block
		fputc(3, f);
		int size = int((filenameonly.length() + numDigits + 2 + textureFormat.length() + 1)*numPages);
		fwrite(&size, sizeof(size), 1, f);

		for( size_t n = 0; n < numPages; n++ )
		{
			fprintf(f, "%s_%0*d.%s", filenameonly.c_str(), numDigits, (int)n, textureFormat.c_str());
			fputc(0, f);
		}
	}

	const int maxChars = useUnicode ? maxUnicodeChar+1 : 256;

	// Count the number of characters that will be written
	int numChars = 0;
	int n;
	for( n = 0; n < maxChars; n++ )
		if( chars[n] )
			numChars++;

	if( invalidCharGlyph )
		numChars++;

	if( fontDescFormat == 0 )
		fprintf(f, "chars count=%d\r\n", numChars);
	else if( fontDescFormat == 1 )
		fprintf(f, "  <chars count=\"%d\">\r\n", numChars);
	else if( fontDescFormat == 2 )
	{
		fputc(4, f);

		// Determine the size of this block
		int size = (4+2+2+2+2+2+2+2+1+1)*numChars;
		fwrite(&size, 4, 1, f);
	}

	if( invalidCharGlyph )
	{
		if( fontDescFormat == 1 )
			fprintf(f, "    <char id=\"%d\" x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\" xoffset=\"%d\" yoffset=\"%d\" xadvance=\"%d\" page=\"%d\" chnl=\"%d\" />\r\n", -1, invalidCharGlyph->m_x, invalidCharGlyph->m_y, invalidCharGlyph->m_width, invalidCharGlyph->m_height, invalidCharGlyph->m_xoffset, invalidCharGlyph->m_yoffset, invalidCharGlyph->m_advance, invalidCharGlyph->m_page, invalidCharGlyph->m_chnl);
		else if( fontDescFormat == 0 )
			fprintf(f, "char id=%-4d x=%-5d y=%-5d width=%-5d height=%-5d xoffset=%-5d yoffset=%-5d xadvance=%-5d page=%-2d chnl=%-2d\r\n", -1, invalidCharGlyph->m_x, invalidCharGlyph->m_y, invalidCharGlyph->m_width, invalidCharGlyph->m_height, invalidCharGlyph->m_xoffset, invalidCharGlyph->m_yoffset, invalidCharGlyph->m_advance, invalidCharGlyph->m_page, invalidCharGlyph->m_chnl);
		else
		{
#pragma pack(push)
#pragma pack(1)
			struct charBlock
			{
				DWORD id;
				WORD x;
				WORD y;
				WORD width;
				WORD height;
				short xoffset;
				short yoffset;
				short xadvance;
				char  page;
				char  channel;
			} charInfo;
#pragma pack(pop)

			charInfo.id = -1;
			charInfo.x  = invalidCharGlyph->m_x;
			charInfo.y  = invalidCharGlyph->m_y;
			charInfo.width = invalidCharGlyph->m_width;
			charInfo.height = invalidCharGlyph->m_height;
			charInfo.xoffset = invalidCharGlyph->m_xoffset;
			charInfo.yoffset = invalidCharGlyph->m_yoffset;
			charInfo.xadvance = invalidCharGlyph->m_advance;
			charInfo.page = invalidCharGlyph->m_page;
			charInfo.channel = invalidCharGlyph->m_chnl;

			fwrite(&charInfo, sizeof(charInfo), 1, f);
		}
	}

	for( n = 0; n < maxChars; n++ )
	{
        if( chars[n] )
		{
			int page, chnl;
			page = chars[n]->m_page;
			chnl = chars[n]->m_chnl;
			
			if( fontDescFormat == 1 )
				fprintf(f, "    <char id=\"%d\" x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\" xoffset=\"%d\" yoffset=\"%d\" xadvance=\"%d\" page=\"%d\" chnl=\"%d\" />\r\n", n, chars[n]->m_x, chars[n]->m_y, chars[n]->m_width, chars[n]->m_height, chars[n]->m_xoffset, chars[n]->m_yoffset, chars[n]->m_advance, page, chnl);
			else if( fontDescFormat == 0 )
				fprintf(f, "char id=%-4d x=%-5d y=%-5d width=%-5d height=%-5d xoffset=%-5d yoffset=%-5d xadvance=%-5d page=%-2d chnl=%-2d\r\n", n, chars[n]->m_x, chars[n]->m_y, chars[n]->m_width, chars[n]->m_height, chars[n]->m_xoffset, chars[n]->m_yoffset, chars[n]->m_advance, page, chnl);
			else
			{
#pragma pack(push)
#pragma pack(1)
				struct charBlock
				{
					DWORD id;
					WORD x;
					WORD y;
					WORD width;
					WORD height;
					short xoffset;
					short yoffset;
					short xadvance;
					char  page;
					char  channel;
				} charInfo;
#pragma pack(pop)

				charInfo.id = n;
				charInfo.x  = chars[n]->m_x;
				charInfo.y  = chars[n]->m_y;
				charInfo.width = chars[n]->m_width;
				charInfo.height = chars[n]->m_height;
				charInfo.xoffset = chars[n]->m_xoffset;
				charInfo.yoffset = chars[n]->m_yoffset;
				charInfo.xadvance = chars[n]->m_advance;
				charInfo.page = page;
				charInfo.channel = chnl;

				fwrite(&charInfo, sizeof(charInfo), 1, f);
			}
		}
	}

	if( fontDescFormat == 1 )
		fprintf(f, "  </chars>\r\n");

	if( !dontIncludeKerningPairs )
	{
		// Save the kerning pairs as well
		vector<KERNINGPAIR> pairs;
		if( useUnicode )
		{
			// TODO: How do I obtain the kerning pairs for 
			// the characters in the higher planes?

			int num = GetKerningPairsW(dc, 0, 0);
			if( num > 0 )
			{
				pairs.resize(num);
				GetKerningPairsW(dc, num, &pairs[0]);
			}
		}
		else
		{
			int num = GetKerningPairsA(dc, 0, 0);
			if( num > 0 )
			{
				pairs.resize(num);
				GetKerningPairsA(dc, num, &pairs[0]);
			}
		}
	/*
		{
			pairs.resize(0);

			// Build a list of all selected chars
			vector<UINT> chars;
			chars.reserve(GetNumCharsSelected());
			for( UINT n = 0; n <= maxUnicodeChar; n++ )
			{
				if( selected[n] )
				{
					chars.push_back(n);
					if( chars.size() == GetNumCharsSelected() )
						break;
				}
			}

			GetKerningPairsFromKERN(dc, pairs, chars, this);
		}
	*/
		if( pairs.size() == 0 )
		{
			// Build a list of all selected chars
			vector<UINT> chars;
			chars.reserve(GetNumCharsSelected());
			for( UINT n = 0; n <= maxUnicodeChar; n++ )
			{
				if( selected[n] )
				{
					chars.push_back(n);
					if( chars.size() == GetNumCharsSelected() )
						break;
				}
			}

			GetKerningPairsFromGPOS(dc, pairs, chars, this);
		}

		if( pairs.size() > 0 )
		{
			// It's been reported that for Chinese WinXP the kerning pairs for 
			// non-unicode charsets may contain characters > 255, so we need to 
			// filter for this.
			for( unsigned int n = 0; n < pairs.size(); n++ )
			{
				if( pairs[n].iKernAmount/aa == 0 ||       // Filter kerning pairs where the adjustment is too small
					pairs[n].wFirst >= maxChars ||        // Filter kerning pairs if they are outside the valid range
					pairs[n].wSecond >= maxChars ||
					disabled[pairs[n].wFirst] ||          // Filter kerning pairs for characters that won't be exported
					disabled[pairs[n].wSecond] ||
					!selected[pairs[n].wFirst] ||         // Filter kerning pairs for characters that won't be exported
					!selected[pairs[n].wSecond] ||
					chars[pairs[n].wFirst] == 0 ||        // Filter kerning pairs for characters that won't be exported
					chars[pairs[n].wSecond] == 0 ||
					!chars[pairs[n].wFirst]->m_isChar ||  // Filter kerning pairs for imported images
					!chars[pairs[n].wSecond]->m_isChar )
				{
					pairs[n] = pairs[pairs.size()-1];
					pairs.pop_back();
					n--;
				}
			}

			// The list of kerning pairs returned by GetKerningPairs sometimes also 
			// have duplicates, so that needs to be filtered too. It seems to be a bug 
			// when calling GetKerningPairsA but not for W.
			// TODO: Sometimes the adjustment is not equal for the duplicates, which should be used then?
			if( !useUnicode )
			{
				for( unsigned int n = 0; n < pairs.size(); n++ )
				{
					for( unsigned int m = n+1; m < pairs.size(); m++ )
					{
						if( pairs[n].wFirst  == pairs[m].wFirst  &&
							pairs[n].wSecond == pairs[m].wSecond )
						{
							pairs[m] = pairs[pairs.size()-1];
							pairs.pop_back();
							m--;
						}
					}
				}
			}
		}

		if( pairs.size() > 0 )
		{
			// Write the header
			if( fontDescFormat == 0 )
				fprintf(f, "kernings count=%d\r\n", (int)pairs.size());
			else if( fontDescFormat == 1 )
				fprintf(f, "  <kernings count=\"%d\">\r\n", (int)pairs.size());
			else if( fontDescFormat == 2 )
			{
				fputc(5, f);

				// Determine the size of the block
				int size = (int)pairs.size()*10;
				fwrite(&size, 4, 1, f);
			}
		}

		for( unsigned int n = 0; n < pairs.size(); n++ )
		{
			if( fontDescFormat == 1 )
				fprintf(f, "    <kerning first=\"%d\" second=\"%d\" amount=\"%d\" />\r\n", pairs[n].wFirst, pairs[n].wSecond, pairs[n].iKernAmount/aa);
			else if( fontDescFormat == 0 )
				fprintf(f, "kerning first=%-3d second=%-3d amount=%-4d\r\n", pairs[n].wFirst, pairs[n].wSecond, pairs[n].iKernAmount/aa);
			else 
			{
#pragma pack(push)
#pragma pack(1)
				struct kerningBlock
				{
					DWORD first;
					DWORD second;
					short amount;
				} kerning;
#pragma pack(pop)
				kerning.first = pairs[n].wFirst;
				kerning.second = pairs[n].wSecond;
				kerning.amount = pairs[n].iKernAmount/aa;

				fwrite(&kerning, sizeof(kerning), 1, f);
			}
		}

		if( pairs.size() > 0 && fontDescFormat == 1 )
			fprintf(f, "  </kernings>\r\n");
	}

	if( fontDescFormat == 1 ) fprintf(f, "</font>\r\n");

	fclose(f);

	SelectObject(dc, oldFont);
	DeleteObject(font);

	DeleteDC(dc);

	// Save the image file
	for( n = 0; n < (signed)pages.size(); n++ )
	{
		string str = acStringFormat("%s_%0*d.%s", filename.c_str(), numDigits, n, textureFormat.c_str());

		acImage::Image image;
		image.width = outWidth;
		image.height = outHeight;
		if( outBitDepth == 32 )
		{
			image.pitch = image.width*4;
			image.format = acImage::PF_A8R8G8B8;
		}
		else
		{
			image.pitch = image.width;
			image.format = acImage::PF_A8;
		}

		image.data = new BYTE[image.pitch * image.height];

		// Generate the output texture for saving
		pages[n]->GenerateOutputTexture();

		cImage *page = pages[n]->GetPageImage();
		if( outBitDepth == 8 )
		{
			// Write image data
			for( int y = 0; y < outHeight; y++ )
			{
				for( int x = 0; x < outWidth; x++ )
				{
					DWORD pixel = page->pixels[y*outWidth + x];
					image.data[y*image.pitch + x] = (BYTE)(pixel>>24);
				}
			}
		}
		else
		{
			// Write image data
			for( int y = 0; y < outHeight; y++ )
			{
				for( int x = 0; x < outWidth; x++ )
				{
					DWORD pixel = page->pixels[y*outWidth + x];
					*(DWORD*)&image.data[y*image.pitch + x*4] = pixel;
				}
			}
		}

		if( textureFormat == "tga" )
			acImage::SaveTga(str.c_str(), image, textureCompression ? acImage::TGA_RLE : 0);
		else if( textureFormat == "png" )
			acImage::SavePng(str.c_str(), image);
		else if( textureFormat == "dds" )
			acImage::SaveDds(str.c_str(), image, textureCompression);
	}

	return 0;
}

string CFontGen::GetLastConfigFile() const
{
	return fontConfigFile;
}

int CFontGen::SaveConfiguration(const char *szFile)
{
	string filename = szFile;
	if( _stricmp(filename.substr(filename.length() - 5).c_str(), ".bmfc") == 0 )
		filename = filename.substr(0, filename.length() - 5);

	FILE *f = 0;
	errno_t e  = fopen_s(&f, (filename + ".bmfc").c_str(), "wb");
	if( e != 0 || f == 0 ) return -1;
	
	// Keep the name so it can be queried later
	fontConfigFile = filename;

	fprintf(f, "# AngelCode Bitmap Font Generator configuration file\n");
	fprintf(f, "fileVersion=%d\n", 1);

	fprintf(f, "\n# font settings\n");
	fprintf(f, "fontName=%s\n", fontName.c_str());

	// Determine relative path to save the font file
	string tmp = acUtility::GetRelativePath(filename, fontFile);
	fprintf(f, "fontFile=%s\n", tmp.c_str());

	fprintf(f, "charSet=%d\n", charSet);
	fprintf(f, "fontSize=%d\n", fontSize);
	fprintf(f, "aa=%d\n", aa);
	fprintf(f, "scaleH=%d\n", scaleH);
	fprintf(f, "useSmoothing=%d\n", useSmoothing);
	fprintf(f, "isBold=%d\n", isBold);
	fprintf(f, "isItalic=%d\n", isItalic);
	fprintf(f, "useUnicode=%d\n", useUnicode);
	fprintf(f, "disableBoxChars=%d\n", disableBoxChars);
	fprintf(f, "outputInvalidCharGlyph=%d\n", outputInvalidCharGlyph);
	fprintf(f, "dontIncludeKerningPairs=%d\n", dontIncludeKerningPairs);
	fprintf(f, "useHinting=%d\n", useHinting);
	fprintf(f, "renderFromOutline=%d\n", renderFromOutline);
	fprintf(f, "useClearType=%d\n", useClearType);

	fprintf(f, "\n# character alignment\n");
	fprintf(f, "paddingDown=%d\n", paddingDown);
	fprintf(f, "paddingUp=%d\n", paddingUp);
	fprintf(f, "paddingRight=%d\n", paddingRight);
	fprintf(f, "paddingLeft=%d\n", paddingLeft);	
	fprintf(f, "spacingHoriz=%d\n", spacingHoriz);
	fprintf(f, "spacingVert=%d\n", spacingVert);
	fprintf(f, "useFixedHeight=%d\n", fixedHeight);
	fprintf(f, "forceZero=%d\n", forceZero);

	fprintf(f, "\n# output file\n");
	fprintf(f, "outWidth=%d\n", outWidth);
	fprintf(f, "outHeight=%d\n", outHeight);
	fprintf(f, "outBitDepth=%d\n", outBitDepth);
	fprintf(f, "fontDescFormat=%d\n", fontDescFormat);
	fprintf(f, "fourChnlPacked=%d\n", fourChnlPacked);
	fprintf(f, "textureFormat=%s\n", textureFormat.c_str());
	fprintf(f, "textureCompression=%d\n", textureCompression);
	fprintf(f, "alphaChnl=%d\n", alphaChnl);
	fprintf(f, "redChnl=%d\n", redChnl);
	fprintf(f, "greenChnl=%d\n", greenChnl);
	fprintf(f, "blueChnl=%d\n", blueChnl);
	fprintf(f, "invA=%d\n", invA);
	fprintf(f, "invR=%d\n", invR);
	fprintf(f, "invG=%d\n", invG);
	fprintf(f, "invB=%d\n", invB);

	fprintf(f, "\n# outline\n");
	fprintf(f, "outlineThickness=%d\n", outlineThickness);

	fprintf(f, "\n# selected chars\n");
	
	int maxChars = useUnicode ? maxUnicodeChar+1 : 256;
	int lastChar = -1;
	bool isRange = false;
	int lineLength = 0;
	for( int n = 0; n < maxChars; n++ )
	{
		if( selected[n] && !disabled[n] )
		{
			// Is this the first char on the line?
			if( lastChar == -1 )
				lineLength += fprintf(f, "chars=%d", n);
			// Is this a new start?
			else if( lastChar != n-1 )
				lineLength += fprintf(f, ",%d", n);
			// Is this a range?
			if( lastChar == n-1 )
			{
				if( !isRange ) 
				{
					lineLength += fprintf(f, "-");
					isRange = true;
				}
				// Is this the last char in the range?
				if( n+1 == maxChars || !selected[n+1] || disabled[n+1] )
				{
					lineLength += fprintf(f, "%d", n);
					isRange = false;
				}
			}

			lastChar = n;

			// Make sure the line doesn't get too long
			if( lineLength > 100 && !isRange )
			{
				fprintf(f, "\n");
				lineLength = 0;
				lastChar = -1;
			}
		}
	}
	fprintf(f, "\n");

	fprintf(f, "\n# imported icon images\n");
	for( int n = 0; n < (signed)iconImages.size(); n++ )
	{
		string tmp = acUtility::GetRelativePath(filename, iconImages[n]->fileName);
		fprintf(f, "icon=\"%s\",%d,%d,%d,%d\n", tmp.c_str(), iconImages[n]->id, iconImages[n]->xoffset, iconImages[n]->yoffset, iconImages[n]->advance);
	}

	fclose(f);

	return 0;
}

int CFontGen::LoadConfiguration(const char *filename)
{
	acUtility::CConfig config;
	int r = config.LoadConfigFile(filename);
	if( r < 0 )
		return -1;

	// Keep the name so it can be queried later
	fontConfigFile = filename;

	// Reset the icon images
	ClearIconImages();

	// Read the values into temporary variables 
	int    _fileVersion;            config.GetAttrAsInt("fileVersion", _fileVersion);
	string _fontName;               config.GetAttrAsString("fontName", _fontName, 0, "Arial");
	string _fontFile;               config.GetAttrAsString("fontFile", _fontFile, 0, "");

	// Determine the full path for the font file
	if( _fontFile != "" )
		_fontFile = acUtility::GetFullPath(filename, _fontFile);

	int    _charSet;                config.GetAttrAsInt("charSet", _charSet, 0, ANSI_CHARSET);
	int    _fontSize;               config.GetAttrAsInt("fontSize", _fontSize, 0, 32);
	int    _aa;                     config.GetAttrAsInt("aa", _aa, 0, 1);
	int    _scaleH;                 config.GetAttrAsInt("scaleH", _scaleH, 0, 100);
	bool   _useSmoothing;           config.GetAttrAsBool("useSmoothing", _useSmoothing, 0, true);
	bool   _isBold;                 config.GetAttrAsBool("isBold", _isBold, 0, false);
	bool   _isItalic;               config.GetAttrAsBool("isItalic", _isItalic, 0, false);
	bool   _useUnicode;             config.GetAttrAsBool("useUnicode", _useUnicode, 0, true);
	bool   _useHinting;             config.GetAttrAsBool("useHinting", _useHinting, 0, true);
	bool   _renderFromOutline;      config.GetAttrAsBool("renderFromOutline", _renderFromOutline, 0, false);
	bool   _useClearType;           config.GetAttrAsBool("useClearType", _useClearType, 0, true);
	int    _paddingDown;            config.GetAttrAsInt("paddingDown", _paddingDown, 0, 0);
	int    _paddingUp;              config.GetAttrAsInt("paddingUp", _paddingUp, 0, 0);
	int    _paddingRight;           config.GetAttrAsInt("paddingRight", _paddingRight, 0, 0);
	int    _paddingLeft;            config.GetAttrAsInt("paddingLeft", _paddingLeft, 0, 0);
	int    _spacingHoriz;           config.GetAttrAsInt("spacingHoriz", _spacingHoriz, 0, 1);
	int    _spacingVert;            config.GetAttrAsInt("spacingVert", _spacingVert, 0, 1);
	bool   _fixedHeight;            config.GetAttrAsBool("useFixedHeight", _fixedHeight, 0, false);
	bool   _forceZero;              config.GetAttrAsBool("forceZero", _forceZero, 0, false);
	int    _outWidth;               config.GetAttrAsInt("outWidth", _outWidth, 0, 256);
	int    _outHeight;              config.GetAttrAsInt("outHeight", _outHeight, 0, 256);
	int    _outBitDepth;            config.GetAttrAsInt("outBitDepth", _outBitDepth, 0, 8);
	int    _fontDescFormat;         config.GetAttrAsInt("fontDescFormat", _fontDescFormat, 0, 0);
	bool   _fourChnlPacked;         config.GetAttrAsBool("fourChnlPacked", _fourChnlPacked, 0, false);
	string _textureFormat;          config.GetAttrAsString("textureFormat", _textureFormat, 0, "tga");
	int    _textureCompression;     config.GetAttrAsInt("textureCompression", _textureCompression, 0, 0);
	bool   _outputInvalidCharGlyph; config.GetAttrAsBool("outputInvalidCharGlyph", _outputInvalidCharGlyph, 0, false);
	bool   _dontIncludeKerningPairs; config.GetAttrAsBool("dontIncludeKerningPairs", _dontIncludeKerningPairs, 0, false);
	int    _outlineThickness;       config.GetAttrAsInt("outlineThickness", _outlineThickness, 0, 0);
	int    _alphaChnl;              config.GetAttrAsInt("alphaChnl", _alphaChnl, 0, 1);
	int    _redChnl;                config.GetAttrAsInt("redChnl", _redChnl, 0, 0);
	int    _greenChnl;              config.GetAttrAsInt("greenChnl", _greenChnl, 0, 0);
	int    _blueChnl;               config.GetAttrAsInt("blueChnl", _blueChnl, 0, 0);
	bool   _invA;                   config.GetAttrAsBool("invA", _invA, 0, false);
	bool   _invR;                   config.GetAttrAsBool("invR", _invR, 0, false);
	bool   _invG;                   config.GetAttrAsBool("invG", _invG, 0, false);
	bool   _invB;                   config.GetAttrAsBool("invB", _invB, 0, false);

	static bool _selected[maxUnicodeChar+1];
	memset(_selected, 0, sizeof(_selected));

	for( int n = 0; n < config.GetAttrCount("chars"); n++ )
	{
		string line; config.GetAttrAsString("chars", line, n);
		char *c = &line[0];

		// Parse the ranges
		while( *c >= '0' && *c <= '9' )
		{
			int firstChar = strtol(c, &c, 10);
			if( *c == '-' )
			{
				int lastChar = strtol(c+1, &c, 10);
				for( int n = firstChar; n <= lastChar; n++ )
					_selected[n] = true;
			}
			else
				_selected[firstChar] = true;

			if( *c == ',' ) c++;
		}
	}

	for( int n = 0; n < config.GetAttrCount("icon"); n++ )
	{
		string line; config.GetAttrAsString("icon", line, n);
		char *c = &line[0];

		// Parse the icon image
		char *start = strchr(c, '"');
		char *end = 0;
		if( start )
			end = strchr(start+1, '"');
		if( end )
		{
			string file;
			file.assign(start+1, end);
			file = acUtility::GetFullPath(filename, file);
			
			int id = 0, xoffset = 0, yoffset = 0, advance = 0;
			if( *(end+1) == ',' )
			{
				c = end+2;
				id = strtol(c, &c, 10);
				if( *c == ',' )
				{
					xoffset = strtol(c+1, &c, 10);
					if( *c == ',' )
					{
						yoffset = strtol(c+1, &c, 10);
						if( *c == ',' )
						{
							advance = strtol(c+1, &c, 10);
						}
					}
				}
			}

			AddIconImage(file.c_str(), id, xoffset, yoffset, advance);
		}
	}

	// Make sure the values are in valid ranges
	size_t pos = _fontName.find_last_not_of(" \t\n\r");
	if( pos != string::npos ) _fontName.erase(pos + 1);
	if( _aa < 1 ) _aa = 1; if( _aa > 4 ) _aa = 4;
	if( _scaleH < 1 ) _scaleH = 1;
	if( _paddingDown < 0 ) _paddingDown = 0;
	if( _paddingUp < 0 ) _paddingUp = 0;
	if( _paddingRight < 0 ) _paddingRight = 0;
	if( _paddingLeft < 0 ) _paddingLeft = 0;
	if( _spacingHoriz < 0 ) _spacingHoriz = 0;
	if( _spacingVert < 0 ) _spacingVert = 0;
	if( _outWidth < 1 ) _outWidth = 1;
	if( _outHeight < 1 ) _outHeight = 1;
	if( _outBitDepth != 8 && _outBitDepth != 32 ) _outBitDepth = 8;
	if( _fontDescFormat < 0 || _fontDescFormat > 2 ) _fontDescFormat = 0;
    
	pos = _textureFormat.find_last_not_of(" \t\n\r");
	if( pos != string::npos ) _textureFormat.erase(pos + 1);

	if( _textureFormat == "tga" )
	{
		if( _textureCompression < 0 ) _textureCompression = 0;
		if( _textureCompression > 1 ) _textureCompression = 1;
	}
	else if( _textureFormat == "png" )
	{
		_textureCompression = 0;
	}
	else if( _textureFormat == "dds" )
	{
		if( _textureCompression < 0 ) _textureCompression = 0;
		if( _textureCompression > 3 ) _textureCompression = 3;
	}
	else
	{
		_textureFormat      = "tga";
		_textureCompression = 0;
	}

	// Is it the right file version?
	if( _fileVersion != 1 )
		return -1;

	// Set the properties
	SetFontName(_fontName);
	SetFontFile(_fontFile);
	SetCharSet(_charSet);
	SetFontSize(_fontSize);
	SetAntiAliasingLevel(_aa);
	SetScaleHeight(_scaleH);
	SetUseSmoothing(_useSmoothing);
	SetRenderFromOutline(_renderFromOutline);
	SetUseHinting(_useHinting);
	SetUseClearType(_useClearType);
	SetBold(_isBold);
	SetItalic(_isItalic);
	SetUseUnicode(_useUnicode);
	SetPaddingDown(_paddingDown);
	SetPaddingUp(_paddingUp);
	SetPaddingRight(_paddingRight);
	SetPaddingLeft(_paddingLeft);
	SetSpacingHoriz(_spacingHoriz);
	SetSpacingVert(_spacingVert);
	SetFixedHeight(_fixedHeight);
	SetForceZero(_forceZero);
	SetOutWidth(_outWidth);
	SetOutHeight(_outHeight);
	SetOutBitDepth(_outBitDepth);
	SetFontDescFormat(_fontDescFormat);
	Set4ChnlPacked(_fourChnlPacked);
	SetOutputInvalidCharGlyph(_outputInvalidCharGlyph);
	SetDontIncludeKerningPairs(_dontIncludeKerningPairs);
	SetTextureFormat(_textureFormat);
	SetTextureCompression(_textureCompression);
	SetOutlineThickness(_outlineThickness);
	SetAlphaChnl(_alphaChnl);
	SetRedChnl(_redChnl);
	SetGreenChnl(_greenChnl);
	SetBlueChnl(_blueChnl);
	SetAlphaInverted(_invA);
	SetRedInverted(_invR);
	SetGreenInverted(_invG);
	SetBlueInverted(_invB);

	Prepare();

	int maxChars = useUnicode ? maxUnicodeChar+1 : 256;
	for( int n = 0; n < maxChars; n++ )
		SetSelected(n, _selected[n]);

	return 0;
}

int CFontGen::GetNumFailedChars()
{
	int numFailed = 0;
	for( int n = 0; n < maxUnicodeChar+1; n++ )
		if( noFit[n] ) numFailed++;

	return numFailed;
}

int CFontGen::SelectCharsFromFile(const char *filename)
{
	if( isWorking ) return -1;

	FILE *f = 0;
	errno_t e = fopen_s(&f, filename, "rb");
	if( e != 0 || f == 0 ) return -1;

	memset(noFit, 0, sizeof(noFit));

	if( IsUsingUnicode() )
	{
		unsigned char byteOrderMark[4];
		size_t cnt = fread(byteOrderMark, 1, 4, f);
		fseek(f, 0, SEEK_SET);

		// Try to determine the encoding from the byte order mark
		bool utf8 = false;
		bool utf16_littleEndian = false;
		bool utf16_bigEndian = false;
		if( byteOrderMark[0] == 0xEF &&
			byteOrderMark[1] == 0xBB &&
			byteOrderMark[2] == 0xBF )
			utf8 = true;
		else if( byteOrderMark[0] == 0xFF &&
				 byteOrderMark[1] == 0xFE )
			utf16_littleEndian = true;
		else if( byteOrderMark[0] == 0xFE &&
				 byteOrderMark[1] == 0xFF )
			utf16_bigEndian = true;
		else
		{
			// TODO: Ask the user
			// TODO: Use IsTextUnicode from Windows API
			//       ref: https://msdn.microsoft.com/en-us/library/windows/desktop/dd318672(v=vs.85).aspx
			// No byte order mark was found, let's read as utf-16 little endian
			utf16_littleEndian = true;
		}

		unsigned char buf[1024];
		size_t remain = 0;
		while( (cnt = fread(buf + remain, 1, 1024 - remain, f) + remain) )
		{
			UINT n = 0;
			for( ; n < cnt && n < 1020; )
			{
				unsigned int len;
				int value;

				if( utf8 )
					value = acUtility::DecodeUTF8(buf+n, &len);
				else
					value = acUtility::DecodeUTF16(buf+n, &len, utf16_littleEndian ? acUtility::LITTLE_ENDIAN : acUtility::BIG_ENDIAN);

				if( value >= 0 )
				{
					n += len;
					if( SetSelected(value, true) < 0 )
					{
						// Control characters are not visible
						if( value >= 32 && value != 0xFEFF )
							noFit[value] = true;
					}
				}
				else
				{
					// Invalid byte sequence, skip one unit
					if( utf8 )
						n++;
					else
						n += 2;
				}
			}

			// Move the last remaining characters to the beginning of the buffer
			if( n < cnt )
			{
				remain = cnt - n;
				for( size_t l = 0; l < remain; l++ )
				{
					buf[l] = buf[n+l];
				}
			}
			else
				remain = 0;
		}
	}
	else
	{
		unsigned char buf[1024];
		size_t cnt;
		while( (cnt = fread(buf, 1, 1024, f)) )
			for( UINT n = 0; n < cnt; n++ )
			{
				if( SetSelected(buf[n], true) < 0 )
					noFit[buf[n]] = true;
			}
	}

	fclose(f);

	return 0;
}

int CFontGen::FindNextFailedCharacterSubset(int startSubset)
{
	if( useUnicode )
	{
		// What is the start of the next subset?
		startSubset++;
		if( startSubset >= (signed)subsets.size() )
			startSubset = 0;

		int startChar = subsets[startSubset]->charBegin;
		for( int n = startChar; n < maxUnicodeChar+1; n++ )
		{
			if( noFit[n] )
				return SubsetFromChar(n);
		}

		for( int n = 0; n <= startChar; n++ )
		{
			if( noFit[n] )
				return SubsetFromChar(n);
		}
	}

	return 0;
}

int CFontGen::SubsetFromChar(int ch)
{
	if( useUnicode )
	{
		if( lastFoundSubset < subsets.size() &&
			ch >= subsets[lastFoundSubset]->charBegin &&
			ch <= subsets[lastFoundSubset]->charEnd )
			return lastFoundSubset;

		// TODO: Use binary search
		for( unsigned int n = 0; n < subsets.size(); n++ )
		{
			if( ch >= subsets[n]->charBegin &&
				ch <= subsets[n]->charEnd )
			{
				lastFoundSubset = n;
				return n;
			}
		}
	}

	return 0;
}

void CFontGen::ClearFailedCharacters()
{
	memset(noFit, 0, sizeof(noFit));
}