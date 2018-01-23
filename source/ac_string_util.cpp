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

// 2016-02-21  Fixes for 64bit

#include <stdarg.h>     // va_list, va_start(), etc
#include <stdlib.h>     // strtod(), strtol()
#include <assert.h>     // assert()
#include <stdio.h>      // _vsnprintf()
#include "ac_string_util.h"

string acStringFormat(const char *format, ...)
{
	string ret;

	va_list args;
	va_start(args, format);

	char tmp[256];
	int r = _vsnprintf_s(tmp, 255, 256, format, args);

	if( r > 0 )
	{
		ret = tmp;
	}
	else
	{
		int n = 512;
		string str; 
		str.resize(n);

		while( (r = _vsnprintf_s(&str[0], n, n, format, args)) < 0 )
		{
			n *= 2;
			str.resize(n);
		}

		ret = str.c_str();
	}

	va_end(args);

	return ret;
}

double acStringScanDouble(const char *string, int *numScanned)
{
	char *end;

	double res = ::strtod(string, &end);

	if( numScanned )
		*numScanned = int(end - string);

	return res;
}

int acStringScanInt(const char *string, int base, int *numScanned)
{
	assert(base > 0);

	char *end;

	int res = ::strtol(string, &end, base);

	if( numScanned )
		*numScanned = int(end - string);

	return res;
}

acUINT acStringScanUInt(const char *string, int base, int *numScanned)
{
	assert(base > 0);

	char *end;

	acUINT res = ::strtoul(string, &end, base);

	if( numScanned )
		*numScanned = int(end - string);

	return res;
}

// Algorithm presented by Dan Berstein in comp.lang.c
acUINT acStringHash(const char *string)
{
	acUINT hash = 5381;
	acUINT c;

	while(c = (unsigned)*string++)
		hash = ((hash << 5) + hash) + c; // hash * 33 + c

	return hash;
}