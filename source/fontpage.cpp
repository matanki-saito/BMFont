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

#include <algorithm>
#include <fstream>

#include "fontpage.h"
#include "fontchar.h"
#include "fontgen.h"

using namespace std;

#define CLR_BORDER 0x007F00ul
#define CLR_UNUSED 0xFF0000ul

CFontPage::CFontPage(CFontGen *gen, int id, int width, int height, int spacingH, int spacingV)
{
	this->gen = gen;
	pageId = id;

	// Allocate and clear the image with the unused color
	pageImg = new (std::nothrow) cImage(width, height);
	pageImg->Clear(CLR_UNUSED);

	// Initialize the height array that shows free space
	heights[0] = new (std::nothrow) int[width];
	if( heights[0] )
		memset(heights[0], 0, width*4);
	heights[1] = 0;
	heights[2] = 0;
	heights[3] = 0;

	currX = 0;

	this->spacingH = spacingH;
	this->spacingV = spacingV;

	paddingRight  = 0;
	paddingLeft   = 0;
	paddingUp     = 0;
	paddingDown   = 0;
}

bool CFontPage::IsOK()
{
	if( pageImg == 0 || heights[0] == 0 ) return false;

	if( pageImg->pixels == 0 ) return false;

	return true;
}

CFontPage::~CFontPage()
{
	if( pageImg )
		delete pageImg;

	for( int n = 0; n < 4; n++ )
		if( heights[n] )
			delete[] heights[n];
}

void CFontPage::SetIntendedFormat(int bitDepth, bool fourChnlPacked, int a, int r, int g, int b)
{
	this->bitDepth = bitDepth;
	this->fourChnlPacked = fourChnlPacked;
	
	alphaChnl = a;
	redChnl   = r;
	greenChnl = g;
	blueChnl  = b;
}

void CFontPage::AddChar(int cx, int cy, CFontChar *ch, int channel)
{
	// Update the charInfo with the extra draw rect
	ch->m_x = cx;
	ch->m_y = cy;
	ch->m_page = pageId;
	ch->m_width += paddingLeft + paddingRight;
	ch->m_height += paddingUp + paddingDown;
	ch->m_xoffset -= paddingLeft;
	ch->m_yoffset -= paddingUp;
	if( bitDepth == 32 && fourChnlPacked )
        ch->m_chnl = !ch->m_isChar ? 0xF : 1<<channel;
	else
		ch->m_chnl = 0xF;

	// Update heights
	cImage *img = ch->m_charImg;
	for( int x = -spacingH; x < img->width + paddingLeft + paddingRight + spacingH; x++ )
	{
		int tempX = x + cx;
		if( tempX < 0 ) tempX += pageImg->width;
		if( cy + img->height + spacingV + paddingUp + paddingDown > heights[channel][tempX] )
			heights[channel][tempX] = cy + img->height + spacingV + paddingUp + paddingDown;
	}

	chars.push_back(ch);

	// Increment counter in CFontGen
	gen->counter++;
}

int CFontPage::AddChar(CFontChar *ch, int channel)
{
	int origX = currX;
	cImage *img = ch->m_charImg;

	// Iterate for each possible x position
	int i = 0;
	while( i++ < pageImg->width - img->width - paddingRight - paddingLeft - spacingH )
	{
		// Is the character narrow enough to fit?
		if( img->width + currX + paddingRight + paddingLeft > pageImg->width - spacingH )
		{
			// Start from the left side again
			currX = 0;
		}

		// Will the character fit in this place?
		int cy = 0;
		for( int n = 0; n < img->width + paddingLeft + paddingRight; n++ )
		{
			if( heights[channel][n+currX] > cy ) 
				cy = heights[channel][n+currX];
		}

		if( cy + img->height + paddingUp + paddingDown <= pageImg->height - spacingV )
		{
			// Are we creating any holes?
			for( int x = 0; x < img->width + paddingLeft + paddingRight; x++ )
			{
				int tempX = x + currX;
				if( cy - spacingV > heights[channel][tempX] )
				{
					SHole hole;
					hole.x    = tempX;
					hole.y    = heights[channel][tempX];
					hole.w    = 1;
					hole.h    = cy - spacingV - hole.y;
					hole.chnl = channel;

					// Determine the width of the hole
					for( x++; x < img->width + paddingLeft + paddingRight; x++ )
					{
						int tempX = x + currX;
						if( hole.y == heights[channel][tempX] )
							hole.w++;
						else
							break;
					}

					// TODO: Should need to search for more holes.

					holes.push_back(hole);
					break;
				}
			}

			AddChar(currX, cy, ch, channel);

			currX += img->width + spacingH + paddingLeft + paddingRight;

			return 0;
		}
		else
		{
			currX++;
		}
	}

	currX = origX;
	return -1;
}

cImage *CFontPage::GetPageImage()
{
	return pageImg;
}

void CFontPage::SetPadding(int left, int up, int right, int down)
{
	paddingLeft  = left;
	paddingUp    = up;
	paddingRight = right;
	paddingDown  = down;
}

int CFontPage::GetNextIdealImageWidth()
{
	return pageImg->width - currX - paddingRight - paddingLeft - spacingH;
}

void CFontPage::GeneratePreviewTexture(int channel)
{
	pageImg->Clear(CLR_UNUSED);

	// Copy the font char images to the texture
	for( unsigned int n = 0; n < chars.size(); n++ )
	{
		if( chars[n]->m_chnl & (1<<channel) )
		{
			int cx = chars[n]->m_x + paddingLeft;
			int cy = chars[n]->m_y + paddingUp;
			cImage *img = chars[n]->m_charImg;
	
			if( chars[n]->HasOutline() )
			{
				// Show the outline, by blending against blue background
				for( int y = 0; y < img->height; y++ )
				{
					for( int x = 0; x < img->width; x++ )
					{
						DWORD p = img->pixels[y*img->width+x];
						if( (p >> 24) < 0xFF )
							p += 255 - (p>>24);
						pageImg->pixels[(y+cy)*pageImg->width+(x+cx)] = p;
					}
				}
			}
			else
			{
				for( int y = 0; y < img->height; y++ )
				{
					for( int x = 0; x < img->width; x++ )
						pageImg->pixels[(y+cy)*pageImg->width+(x+cx)] = img->pixels[y*img->width+x];
				}
			}

			// Draw the spacing borders
			if( spacingH > 0 )
			{
				int cx1 = chars[n]->m_x - 1;
				if( cx1 < 0 ) cx1 += pageImg->width;
				int cx2 = chars[n]->m_x + chars[n]->m_width;
				if( cx2 >= pageImg->width ) cx2 -= pageImg->width;
				int cy = chars[n]->m_y;

				for( int y = 0; y < chars[n]->m_height; y++ )
				{
					pageImg->pixels[(cy+y)*pageImg->width+cx1] = CLR_BORDER;
					pageImg->pixels[(cy+y)*pageImg->width+cx2] = CLR_BORDER;
				}
			}

			if( spacingV > 0 )
			{
				int cy1 = chars[n]->m_y - 1;
				if( cy1 < 0 ) cy1 += pageImg->height;
				int cy2 = chars[n]->m_y + chars[n]->m_height;
				if( cy2 >= pageImg->height ) cy2 -= pageImg->height;
				int cx = chars[n]->m_x;

				for( int x = 0; x < chars[n]->m_width; x++ )
				{
					pageImg->pixels[cy1*pageImg->width+x+cx] = CLR_BORDER;
					pageImg->pixels[cy2*pageImg->width+x+cx] = CLR_BORDER;
				}
			}
		}
	}
}

void CFontPage::GenerateOutputTexture()
{
	// Clear the image
	DWORD color = 0;
	if( alphaChnl == e_one && !gen->IsAlphaInverted() || gen->IsAlphaInverted() )
		color = 0xFF << 24;
	if( fourChnlPacked )
	{
		// Fill all channels with the same color
		color |= (color >> 24) << 16;
		color |= (color >> 24) << 8;
		color |= (color >> 24);
	}
	else
	{
		if( redChnl == e_one && !gen->IsRedInverted() || gen->IsRedInverted() )
			color |= 0xFF << 16;
		if( greenChnl == e_one && !gen->IsGreenInverted() || gen->IsGreenInverted() )
			color |= 0xFF << 8;
		if( blueChnl == e_one && !gen->IsBlueInverted() || gen->IsBlueInverted() )
			color |= 0xFF;
	}
	pageImg->Clear(color);
	
	// Copy the font char images to the texture
	for( unsigned int n = 0; n < chars.size(); n++ )
	{
		int cx = chars[n]->m_x + paddingLeft;
		int cy = chars[n]->m_y + paddingUp;
		cImage *img = chars[n]->m_charImg;

		if( !chars[n]->m_isChar )
		{
			// Colored images are copied as is
			for( int y = 0; y < img->height; y++ )
			{
				for( int x = 0; x < img->width; x++ )
					pageImg->pixels[(y+cy)*pageImg->width+(x+cx)] = img->pixels[y*img->width+x];
			}
		}
		else
		{
			if( bitDepth == 32 && fourChnlPacked )
			{
				// When packing multiple characters we 
				// use the alpha channel to determine the content
				for( int y = 0; y < img->height; y++ )
				{
					for( int x = 0; x < img->width; x++ )
					{
						DWORD p = chars[n]->GetPixelValue(x, y, alphaChnl);
						if( gen->IsAlphaInverted() ) p = 255 - p;
						DWORD c = pageImg->pixels[(y+cy)*pageImg->width+(x+cx)];
						if( chars[n]->m_chnl == 1 )
						{
							c &= 0xFFFFFF00;
							c |= p;
						}
						else if( chars[n]->m_chnl == 2 )
						{
							c &= 0xFFFF00FF;
							c |= p << 8;
						}
						else if( chars[n]->m_chnl == 4 )
						{
							c &= 0xFF00FFFF;
							c |= p << 16;
						}
						else if( chars[n]->m_chnl == 8 )
						{
							c &= 0x00FFFFFF;
							c |= p << 24;
						}
						pageImg->pixels[(y+cy)*pageImg->width+(x+cx)] = c;
					}
				}
			}
			else
			{
				for( int y = 0; y < img->height; y++ )
				{
					for( int x = 0; x < img->width; x++ )
					{
						DWORD p = 0;
						DWORD t;
						t = (BYTE)chars[n]->GetPixelValue(x, y, blueChnl);  if( gen->IsBlueInverted() )  t = 255 - t; p |= t  << 0;
						t = (BYTE)chars[n]->GetPixelValue(x, y, greenChnl); if( gen->IsGreenInverted() ) t = 255 - t; p |= t  << 8;
						t = (BYTE)chars[n]->GetPixelValue(x, y, redChnl);   if( gen->IsRedInverted() )   t = 255 - t; p |= t  << 16;
						t = (BYTE)chars[n]->GetPixelValue(x, y, alphaChnl); if( gen->IsAlphaInverted() ) t = 255 - t; p |= t  << 24;
						pageImg->pixels[(y+cy)*pageImg->width+(x+cx)] = p;
					}
				}
			}
		}
	}
}

// This global pointer will be used by the sorting algorithm
CFontChar **g_chars = 0;

// Define a structure and comparison operator for the sorting algorithm
struct element { int index; };
bool operator<(const element &a, const element &b) 
{
	// We want to sort the characters from larger to smaller
	if( g_chars[a.index]->m_height > g_chars[b.index]->m_height ||
	    (g_chars[a.index]->m_height == g_chars[b.index]->m_height &&
	     g_chars[a.index]->m_width > g_chars[b.index]->m_width) )
		return true;
	return false;
}

void CFontPage::SortList(CFontChar **chars, int *index, int numChars)
{
	g_chars = chars;
	std::sort((element*)index, (element*)(&index[numChars]));
}

#ifdef TRACE_GENERATE
extern ofstream trace;
#endif

void CFontPage::AddChars(CFontChar **chars, int maxChars)
{
#ifdef TRACE_GENERATE
	trace << "Adding colored images\n";
	trace.flush();
#endif

	// Add the colored images first
	AddCharsToPage(chars, maxChars, true, 0);

	// Check if we should stop
	if( gen->stopWorking ) return;

#ifdef TRACE_GENERATE
	trace << "Duplicating the height array to all channels\n";
	trace.flush();
#endif

	// Duplicate the height array for the other channels
	for( int n = 1; n < 4; n++ )
	{
		heights[n] = new (std::nothrow) int[pageImg->width];
		if( heights[n] == 0 )
		{
#ifdef TRACE_GENERATE
			trace << "Out of memory while allocating height buffer\n";
			trace.flush();
#endif
			gen->stopWorking = true;
			gen->outOfMemory = true;
			return;
		}
		memcpy(heights[n], heights[0], pageImg->width*sizeof(int));
	}

	// Remove the current holes
	holes.resize(0);

#ifdef TRACE_GENERATE
	trace << "Adding monochrome images to channel 0\n";
	trace.flush();
#endif

	// Then the black & white images
	AddCharsToPage(chars, maxChars, false, 0);

	// Check if we should stop
	if( gen->stopWorking ) return;

	if( bitDepth == 32 && fourChnlPacked )
	{
		for( int n = 1; n < 4; n++ )
		{
#ifdef TRACE_GENERATE
	trace << "Adding monochrome images to channel " << n << "\n";
	trace.flush();
#endif

			AddCharsToPage(chars, maxChars, false, n);

			// Check if we should stop
			if( gen->stopWorking ) return;
		}
	}
}

void CFontPage::AddCharsToPage(CFontChar **chars, int maxChars, bool colored, int channel)
{
	static int indexA[maxUnicodeChar+1], indexB[maxUnicodeChar+1];
	int *index = indexA, *index2 = indexB;
	int numChars = 0, numChars2 = 0;

	// Add images to the list
	for( int n = 0; n < maxChars; n++ )
	{
		if( chars[n] && chars[n]->m_isChar != colored )
			index[numChars++] = n;
	}

#ifdef TRACE_GENERATE
	trace << "Sorting list of " << numChars << " candidates\n";
	trace.flush();
#endif

	// Sort the characters by height/width, largest first
	SortList(chars, index, numChars);

	// Add the images to the page
	while( numChars > 0 )
	{
		#ifdef TRACE_GENERATE
			trace << "There are " << numChars << " candidates left, and " << holes.size() << " holes to fill\n";
			trace.flush();
		#endif

		// Fill holes
		for( int h = 0; h < (signed)holes.size(); h++ )
		{
			int bestMatch;
			bestMatch = -1;

			// Find the best matching character to fill the hole
			for( int n = 0; n < numChars; n++ )
			{
				if( (holes[h].w == chars[index[n]]->m_charImg->width + paddingLeft + paddingRight) &&
					(holes[h].h == chars[index[n]]->m_charImg->height + paddingUp + paddingDown) )
				{
					bestMatch = n;
					break;
				}
				else if( (holes[h].w >= chars[index[n]]->m_charImg->width + paddingLeft + paddingRight) &&
					     (holes[h].h >= chars[index[n]]->m_charImg->height + paddingUp + paddingDown) )
				{
					if( bestMatch != -1 )
					{
						if( (chars[index[n]]->m_charImg->width > chars[index[bestMatch]]->m_charImg->width) ||
							(chars[index[n]]->m_charImg->height > chars[index[bestMatch]]->m_charImg->height) )
							bestMatch = n;
					}
					else
						bestMatch = n;
				}
			}

			if( bestMatch != -1 )
			{
				int x = holes[h].x;
				int y = holes[h].y;

				// There may still be room for more 
				if( holes[h].w - spacingH > chars[index[bestMatch]]->m_charImg->width + paddingLeft + paddingRight )
				{
					// Create a new hole to the right of the newly inserted character, with the same height of the previous hole
					SHole hole2;
					hole2.x = holes[h].x + (chars[index[bestMatch]]->m_charImg->width + paddingLeft + paddingRight + spacingH);
					hole2.y = holes[h].y;
					hole2.w = holes[h].w - (chars[index[bestMatch]]->m_charImg->width + paddingLeft + paddingRight + spacingH);
					hole2.h = holes[h].h;
					hole2.chnl = holes[h].chnl;
					holes.push_back(hole2);
				}
				if( holes[h].h - spacingV > chars[index[bestMatch]]->m_charImg->height + paddingUp + paddingDown )
				{
					// Create a new hole below the newly inserted character, with the width of the character
					SHole hole2;
					hole2.x = holes[h].x;
					hole2.y = holes[h].y + (chars[index[bestMatch]]->m_charImg->height + paddingUp + paddingDown + spacingV);
					hole2.w = (chars[index[bestMatch]]->m_charImg->width + paddingLeft + paddingRight);
					hole2.h = holes[h].h - (chars[index[bestMatch]]->m_charImg->height + paddingUp + paddingDown + spacingV);
					hole2.chnl = holes[h].chnl;
					holes.push_back(hole2);
				}

				AddChar(x, y, chars[index[bestMatch]], channel);
				chars[index[bestMatch]] = 0;

#ifdef TRACE_GENERATE
				trace << "Character [" << index[bestMatch] << "] was used to fill hole\n";
				trace.flush();
#endif

				// Check if we should stop
				if( gen->stopWorking ) return;

				// Compact the list 
				numChars--;
				for( int n = bestMatch; n < numChars; n++ )
					index[n] = index[n+1];
			}
			
			// Remove the hole
			if( h < (signed)holes.size() - 1 )
				holes[h] = holes[holes.size()-1];
			holes.pop_back();
			h--;
		}

#ifdef TRACE_GENERATE
		trace << "All holes have been filled\n";
		trace.flush();
#endif

		numChars2 = 0;
		bool allTooWide = true;
		bool drawn = false;

		// Determine if there is a large height difference anywhere, and if so start filling from that location
		// This happens for example when importing images that are out of proportion to the rest of the glyphs
		currX = DetermineStartX(chars, index, numChars, channel);

#ifdef TRACE_GENERATE
		trace << "currX is " << currX << " and heights is " << heights[channel][currX] << "\n";
		trace << "tallest char is " << chars[index[0]]->m_height << "x" << chars[index[0]]->m_width << " and shortest char is " << chars[index[numChars-1]]->m_height << "x" << chars[index[numChars-1]]->m_width << "\n";
		trace << "GetNextIdealImageWidth() = " << GetNextIdealImageWidth() << "\n";
		trace.flush();
#endif

		// Sanity check. This could become negative if previous code failed to catch out of memory
		if( GetNextIdealImageWidth() < 0 )
		{
#ifdef TRACE_GENERATE
			trace << "GetNextIdealImageWidth() < 0. Something is wrong\n";
			trace.flush();
#endif

			gen->stopWorking = true;
			gen->outOfMemory = true;
			return;
		}

		// Add one row of chars to texture
		for( int n = 0; n < numChars; n++ )
		{
			bool ok = false;
			if( chars[index[n]]->m_charImg->width <= GetNextIdealImageWidth() )
			{
				allTooWide = false;
				int r = AddChar(chars[index[n]], channel);
				if( r >= 0 )
				{
#ifdef TRACE_GENERATE
					trace << "Character [" << index[n] << "] was added\n";
					trace.flush();
#endif

					chars[index[n]] = 0;
					ok = true;
					drawn = true;

					// Check if we should stop
					if( gen->stopWorking ) return;
				}
			}

			if( !ok )
			{
#ifdef TRACE_GENERATE
//				trace << "Character [" << index[n] << "] didn't fit on this row\n";
//				trace.flush();
#endif

				// Move it to the next index list
				index2[numChars2++] = index[n];
			}
		}

		if( index == indexA )
		{
			// Swap indices
			index = indexB;
			index2 = indexA;
		}
		else
		{
			index = indexA;
			index2 = indexB;
		}

		numChars = numChars2;
		numChars2 = 0;

#ifdef TRACE_GENERATE
		trace << "allTooWide = " << allTooWide << ", drawn = " << drawn << "\n";
		trace.flush();
#endif

		if( !allTooWide && !drawn )
		{
			// Next page
			break;
		}
	}
}

int CFontPage::DetermineStartX(CFontChar **chars, int *index, int numChars, int channel)
{
	int startX = 0;

	// Determine if there is a large height difference anywhere, and if so start filling from that location
	// This happens for example when importing images that are out of proportion to the rest of the glyphs
	int thinnestChar = pageImg->width;
	for( int n = 0; n < numChars; n++ )
		if( thinnestChar > (chars[index[n]]->m_width + paddingLeft + paddingRight + spacingH) )
			thinnestChar = chars[index[n]]->m_width + paddingLeft + paddingRight + spacingH;
	for( int n = 0; n < pageImg->width - 1 - thinnestChar; n++ )
	{
		// Compare against the largest glyph that we'll add (first in list)
		if( heights[channel][n] - heights[channel][n+1] >= (chars[index[0]]->m_height + paddingUp + paddingDown + spacingV) )
		{
			startX = n+1;
			break;
		}
	}

	return startX;
}