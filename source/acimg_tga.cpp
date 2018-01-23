/*
   AngelCode Tool Box Library
   Copyright (c) 2007-2011 Andreas Jonsson
  
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

// 2011-06-05 - Added support for loading colormapped TGA's
// 2011-04-08 - Fixed bug with loading RLE encoded TGA's

#include <stdio.h>
#include <string.h>
#include "acimg.h"

namespace acImage
{

#pragma pack(push)
#pragma pack(1)
struct TargaHeader
{
	BYTE idLength;
	BYTE colormapType;
	BYTE imageType;
	WORD colorMapSpecification_startIdx;
	WORD colorMapSpecification_length;
	BYTE colorMapSpecification_colorBitDepth;
	WORD xOrigin;
	WORD yOrigin;
	WORD imageWidth;
	WORD imageHeight;
	BYTE pixelDepth;
	BYTE imageDescriptor;
};

struct TargaFooter
{
	DWORD extensionAreaOffset;
	DWORD developerDirectoryOffset;
	char signatureString[18];
};
#pragma pack(pop)

int SaveTga(const char *filename, Image &image, DWORD flags)
{
	// Validate the image
	if( image.format != PF_A8R8G8B8 &&
		image.format != PF_R8G8B8 &&
		image.format != PF_A8 )
	{
		return E_FORMAT_NOT_SUPPORTED;
	}

#if defined(_MSC_VER) && _MSC_VER >= 1500
	FILE *f = 0;
	fopen_s(&f, filename, "wb");
#else
	FILE *f = fopen(filename, "wb");
#endif
	if( f == 0 )
		return E_FILE_ERROR;

	// Save the image description
	TargaHeader tga;
	memset(&tga, 0, sizeof(tga));
	tga.idLength        = 0; // No id chunk
	tga.colormapType    = 0; // No palette
	if( image.format == PF_A8R8G8B8 )
	{
		if( flags & TGA_RLE )
			tga.imageType = 10; // True color, RLE
		else
			tga.imageType = 2; // True color, not RLE
		tga.pixelDepth = 32; // 32 bits per pixel
	}
	else if( image.format == PF_R8G8B8 )
	{
		if( flags & TGA_RLE )
			tga.imageType = 10; // True color, RLE
		else
			tga.imageType  = 2; // True color, not RLE
		tga.pixelDepth = 24;
	}
	else if( image.format == PF_A8 )
	{
		if( flags & TGA_RLE )
			tga.imageType = 11; // Gray scale, RLE
		else
			tga.imageType  = 3; // Gray scale, not RLE
		tga.pixelDepth = 8; // 8 bits per pixel
	}

	tga.imageWidth      = image.width;
	tga.imageHeight     = image.height;
	tga.xOrigin         = 0;
	tga.yOrigin         = 0;
	tga.imageDescriptor = 1<<5; // first pixel is the top left

	fwrite(&tga, 18, 1, f);

	// TODO: Improve RLE compression by wrapping around borders

	// TODO: Write a common algorithm for all pixel sizes, use the 24bit as pattern

	// Save image data
	if( image.format == PF_A8 )
	{
		if( flags & TGA_RLE )
		{
			for( UINT y = 0; y < image.height; y++ )
			{
				BYTE *pixels = &image.data[y*image.pitch];
				UINT width = image.width;

				while( width > 0 )
				{
					// Determine the span of equal pixels
					UINT count = 1;
					BYTE pixel = pixels[0];
					for( count = 1; count < width; count++ )
					{
						if( pixel != pixels[count] )
							break;
					}

					// TODO: If the span of equals is too small, e.g. 2 pixels, it's 
					// probably better to simply include them in a span of unequal pixels

					if( count > 1 )
					{
						// We can handle at most a span of 128
						if( count > 128 ) count = 128;

						// Save the packet info
						fputc(0x80+(count-1), f);
						fputc(pixel, f);
					}
					else
					{
						// Determine the span of unequal pixels
						for( ; count < width; count++ )
						{
							if( pixel == pixels[count] )
								break;

							pixel = pixels[count];
						}

						// We can handle at most a span of 128
						if( count > 128 ) count = 128;

						// Save the packet info
						fputc(count-1, f);
						fwrite(pixels, count, 1, f);
					}

					// Move to next packet
					width -= count;
					pixels += count;
				}
			}
		}
		else
		{
			// Write image data
			for( UINT y = 0; y < image.height; y++ )
				fwrite(&image.data[y*image.pitch], image.width, 1, f);
		}
	}
	else if( image.format == PF_R8G8B8 )
	{
		if( flags & TGA_RLE )
		{
			for( UINT y = 0; y < image.height; y++ )
			{
				BYTE *pixels = &image.data[y*image.pitch];
				UINT width = image.width;

				while( width > 0 )
				{
					// Determine the span of equal pixels
					UINT count = 1;
					DWORD pixel = (*(DWORD*)&pixels[0])&0xFFFFFF;
					for( count = 1; count < width; count++ )
					{
						DWORD p2 = *(DWORD*)&pixels[count*3]&0xFFFFFF;
						if( pixel != p2 )
							break;
					}

					if( count > 1 )
					{
						// We can handle at most a span of 128
						if( count > 128 ) count = 128;

						// Save the packet info
						fputc(0x80+(count-1), f);
						fwrite(&pixel, 3, 1, f);
					}
					else
					{
						// Determine the span of unequal pixels
						for( ; count < width; count++ )
						{
							DWORD p2 = *(DWORD*)&pixels[count*3]&0xFFFFFF;
							if( pixel == p2 )
								break;

							pixel = p2;
						}

						// We can handle at most a span of 128
						if( count > 128 ) count = 128;

						// Save the packet info
						fputc(count-1, f);
						fwrite(pixels, count*3, 1, f);
					}

					// Move to next packet
					width -= count;
					pixels += count*3;
				}
			}
		}
		else
		{
			// Write image data
			for( UINT y = 0; y < image.height; y++ )
				fwrite(&image.data[y*image.pitch], image.width*3, 1, f);
		}
	}
	else if( image.format == PF_A8R8G8B8 )
	{
		if( flags & TGA_RLE )
		{
			for( UINT y = 0; y < image.height; y++ )
			{
				DWORD *pixels = (DWORD*)&image.data[y*image.pitch];
				UINT width = image.width;

				while( width > 0 )
				{
					// Determine the span of equal pixels
					UINT count = 1;
					DWORD pixel = pixels[0];
					for( count = 1; count < width; count++ )
					{
						if( pixel != pixels[count] )
							break;
					}

					if( count > 1 )
					{
						// We can handle at most a span of 128
						if( count > 128 ) count = 128;

						// Save the packet info
						fputc(0x80+(count-1), f);
						fwrite(&pixel, 4, 1, f);
					}
					else
					{
						// Determine the span of unequal pixels
						for( ; count < width; count++ )
						{
							if( pixel == pixels[count] )
								break;

							pixel = pixels[count];
						}

						// We can handle at most a span of 128
						if( count > 128 ) count = 128;

						// Save the packet info
						fputc(count-1, f);
						fwrite(pixels, count*4, 1, f);
					}

					// Move to next packet
					width -= count;
					pixels += count;
				}
			}
		}
		else
		{
			// Write image data
			for( UINT y = 0; y < image.height; y++ )
				fwrite(&image.data[y*image.pitch], image.width*4, 1, f);
		}
	}

	// Write image footer
	TargaFooter foot;

	foot.extensionAreaOffset = 0;
	foot.developerDirectoryOffset = 0;
	memcpy(foot.signatureString, "TRUEVISION-XFILE.", 18);

	fwrite(&foot, 26, 1,f);

	fclose(f);

	return E_SUCCESS;
}

static void MoveToNextPixel(int &x, int &y, BYTE *&data, Image &image, const TargaHeader &tga)
{
	if( ++x == tga.imageWidth )
	{
		y++;
		x = 0;
		if( 0 == ( tga.imageDescriptor & 0x20 ) )
			data = &image.data[(tga.imageHeight-y-1) * image.pitch];
		else
			data = &image.data[y*image.pitch];
	}
}

int LoadTga(const char *filename, Image &image)
{
	image.data = 0;

	// Open the file
#if defined(_MSC_VER) && _MSC_VER >= 1500
	FILE *f = 0;
	fopen_s(&f, filename, "rb");
#else
	FILE *f = fopen(filename, "rb");
#endif
	if( f == 0 ) 
		return E_FILE_ERROR;

	// Read in the header
 	TargaHeader tga;
    fread(&tga, sizeof(TargaHeader), 1, f);

    // We don't support all formats
    if( !(
		// Color mapped, run-length encoded or not, with 8bit pixel depth
		((tga.imageType == 1 || tga.imageType == 9) && (tga.pixelDepth == 8)) ||
		// True color, run-length encoded or not, with or without alpha channel
		((tga.imageType == 2 || tga.imageType == 10) && (tga.pixelDepth == 24 || tga.pixelDepth == 32)) ||   
		// Gray scale, run-length encoded or not, without alpha channel
		((tga.imageType == 3 || tga.imageType == 11) && tga.pixelDepth ==  8)) 
		)
	{
		fclose(f);
		return E_FORMAT_NOT_SUPPORTED;
	}

	// If we're loading a colormapped TGA, we need to make sure we support the colormap specification
	if( tga.colormapType == 1 && (tga.imageType == 1 || tga.imageType == 9) )
	{
		if( !tga.colorMapSpecification_startIdx == 0 ||
			tga.colorMapSpecification_length > 256 ||
			!(tga.colorMapSpecification_colorBitDepth == 24 || tga.colorMapSpecification_colorBitDepth == 32) )
		{
			fclose(f);
			return E_FORMAT_NOT_SUPPORTED;
		}
	}

    // Skip the ID field. The first byte of the 
	// header is the length of this field
    if( tga.idLength )
        fseek(f, tga.idLength, SEEK_CUR);

	// Read the color map
	if( tga.colormapType == 1 )
	{
		image.numColours = tga.colorMapSpecification_length;
		image.palette = new DWORD[image.numColours];
		if( image.palette == 0 )
		{
			fclose(f);
			return E_OUT_OF_MEMORY;
		}

		if( tga.colorMapSpecification_colorBitDepth == 24 )
		{
			for( unsigned int n = 0; n < image.numColours; n++ )
			{
				DWORD b = fgetc(f);
				DWORD g = fgetc(f);
				DWORD r = fgetc(f);
				image.palette[n] = (0xFF<<24) | (r<<16) | (g<<8) | (b);
			}
		}
		else if( tga.colorMapSpecification_colorBitDepth == 32 )
		{
			for( unsigned int n = 0; n < image.numColours; n++ )
			{	
				DWORD b = fgetc(f);
				DWORD g = fgetc(f);
				DWORD r = fgetc(f);
				DWORD a = fgetc(f);
				image.palette[n] = (a<<24) | (r<<16) | (g<<8) | (b);
			}
		}
	}

	// Allocate the memory
	image.pitch = tga.imageWidth*tga.pixelDepth/8;
	if( image.pitch % 4 ) image.pitch += 4 - (image.pitch % 4);
 
	if( tga.imageType == 1 || tga.imageType == 9 )
		image.format = PF_COLORMAP;
	else if( tga.pixelDepth == 8 )
		image.format = PF_A8;
	else if( tga.pixelDepth == 24 )
		image.format = PF_R8G8B8;
	else if( tga.pixelDepth == 32 )
		image.format = PF_A8R8G8B8;

	image.width  = tga.imageWidth;
	image.height = tga.imageHeight;

	image.data = new BYTE[image.pitch * image.height];
	if( image.data == 0 ) 
	{
		fclose(f);
		return E_OUT_OF_MEMORY;
	}

	// Load the image data
	int y = 0, x = 0;
	BYTE *data;

	// Is the image upside down?
    if( 0 == ( tga.imageDescriptor & 0x20 ) )
        data = &image.data[(tga.imageHeight-y-1) * image.pitch];
	else
		data = &image.data[y*image.pitch];

    while( y < tga.imageHeight )
    {
		// Is the data runlength encoded?
        if( tga.imageType == 10 || tga.imageType == 11 )
        {
            BYTE packetInfo = fgetc(f);
            WORD packetType = 0x80 & packetInfo;
            WORD pixelCount = ( 0x007f & packetInfo ) + 1;

            if( packetType )
            {
				if( tga.imageType == 10 )
				{
					if( tga.pixelDepth == 32 )
					{
						BYTE b = fgetc(f);
						BYTE g = fgetc(f);
						BYTE r = fgetc(f);
						BYTE a = fgetc(f);

						while( pixelCount-- )
						{
							*data++ = b;
							*data++ = g;
							*data++ = r;
							*data++ = a;
							MoveToNextPixel(x, y, data, image, tga);
						}
					}
					else
					{
						BYTE b = fgetc(f);
						BYTE g = fgetc(f);
						BYTE r = fgetc(f);

						while( pixelCount-- )
						{
							*data++ = b;
							*data++ = g;
							*data++ = r;
							MoveToNextPixel(x, y, data, image, tga);
						}
					}
				}
				else
				{
					BYTE w = fgetc(f);
					while( pixelCount-- )
					{
						*data++ = w;
						MoveToNextPixel(x, y, data, image, tga);
					}
				}
            }
            else
            {
				if( tga.imageType == 10 )
				{
					if( tga.pixelDepth == 32 )
					{
						while( pixelCount-- )
						{
							*data++ = fgetc(f);
							*data++ = fgetc(f);
							*data++ = fgetc(f);
							*data++ = fgetc(f);
							MoveToNextPixel(x, y, data, image, tga);
						}
					}
					else
					{
						while( pixelCount-- )
						{
							*data++ = fgetc(f);
							*data++ = fgetc(f);
							*data++ = fgetc(f);
							MoveToNextPixel(x, y, data, image, tga);
						}
					}
				}
				else
				{
					while( pixelCount-- )
					{
						BYTE w = fgetc(f);
						*data++ = w;
						MoveToNextPixel(x, y, data, image, tga);
					}
				}
            }
        }
        else
        {
			if( tga.pixelDepth == 8 )
			{
				BYTE w = fgetc(f);

				*data++ = w;
				MoveToNextPixel(x, y, data, image, tga);
			}
			else if( tga.pixelDepth == 24 )
			{
				*data++ = fgetc(f);
				*data++ = fgetc(f);
				*data++ = fgetc(f);
				MoveToNextPixel(x, y, data, image, tga);
			}
			else
			{
				*data++ = fgetc(f);
				*data++ = fgetc(f);
				*data++ = fgetc(f);
				*data++ = fgetc(f);
				MoveToNextPixel(x, y, data, image, tga);
			}
        }
    }

	// Close the file
    fclose(f);

	return E_SUCCESS;
}

} // namespace acImage
