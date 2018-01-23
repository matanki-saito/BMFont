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

#ifndef FONTPAGE_H
#define FONTPAGE_H

#include <vector>
#include "ac_image.h"

class CFontChar;
class CFontGen;

struct SHole
{
	int x;
	int y;
	int w;
	int h;
	int chnl;
};

class CFontPage
{
public:
	CFontPage(CFontGen *gen, int id, int width, int height, int spacingH, int spacingV);
	~CFontPage();

	bool    IsOK();

	void    SetPadding(int left, int up, int right, int down);
	void    SetIntendedFormat(int bitDepth, bool fourChnlPacked, int a, int r, int g, int b);

	void    AddChars(CFontChar **chars, int count);

	void    GeneratePreviewTexture(int channel);
	void    GenerateOutputTexture();

	cImage *GetPageImage();

protected:
	void    AddChar(int x, int y, CFontChar *ch, int channel);
	int     AddChar(CFontChar *ch, int channel);
	void    SortList(CFontChar **ch, int *indices, int count);
	void    AddCharsToPage(CFontChar **ch, int count, bool colored, int channel);
	int     GetNextIdealImageWidth();
	int     DetermineStartX(CFontChar **ch, int *indices, int count, int channel);

	CFontGen *gen;

	int     pageId;
	cImage *pageImg;
	int    *heights[4];
	int     currX;
	int     spacingH;
	int     spacingV;

	int paddingRight;
	int paddingLeft;
	int paddingUp;
	int paddingDown;

	int  bitDepth;
	bool fourChnlPacked;
	int  alphaChnl;
	int  redChnl;
	int  greenChnl;
	int  blueChnl;

	std::vector<CFontChar*> chars;
	std::vector<SHole> holes;
};

#endif