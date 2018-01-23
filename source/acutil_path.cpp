/*
   AngelCode Tool Box Library
   Copyright (c) 2012-2016 Andreas Jonsson
  
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
// 2014-06-16  Updated to support build both for unicode and multibyte applications
// 2013-06-15  Fixed GetFullPath() to handle relative base paths
// 2013-06-15  Fixed crash in GetRelativePath() when both paths refer to same directory

#include "acutil_path.h"
#include <windows.h>
#include "acwin_window.h"

using namespace std;

namespace acUtility
{

string ReplacePathSlashes(const string &path)
{
	string tmp(path);

	// Replace all backslashes with forward slashes
	size_t pos;
	while( (pos = tmp.find("\\")) != string::npos )
		tmp[pos] = '/';

	return tmp;
}

string GetApplicationPath()
{
	// Get the full path of the application
	TCHAR buffer[300];
	GetModuleFileName(0, buffer, 300);
	string path;
	acWindow::ConvertTCharToUtf8(buffer, path);

	// Replace all backslashes with forward slashes
	path = ReplacePathSlashes(path);

	// Cut of the name of the application
	size_t n = path.rfind("/");
	path = path.substr(0, n+1);

	return path;
}

string GetFullPath(const string &base, const string &relative)
{
	string b = ReplacePathSlashes(base);
	string r = ReplacePathSlashes(relative);

	// Make sure the base path doesn't contain a filename
	if( b[b.size()-1] != '/' )
	{
		size_t pos = b.rfind("/");
		b = b.substr(0, pos+1);
	}

	// Get the drive letter from the base path
	string drive, path;
	size_t pos = b.find(":");
	if( pos != string::npos )
	{
		drive = b.substr(0, pos);
		path = b.substr(pos+1);
	}
	else
	{
		// The base itself is a relative path so get the current working directory
		TCHAR buf[MAX_PATH];
		GetCurrentDirectory(MAX_PATH, buf);
		string cwd;
		acWindow::ConvertTCharToUtf8(buf, cwd);

		if( b.length() > 0 && b[0] == '/' )
		{
			// The base is an absolute path in current drive
			path = cwd.substr(0, 2);
			path += b;
		}
		else
			path = ReplacePathSlashes(cwd) + '/' + b;

		drive = path.substr(0, 1);
		path = path.substr(2);
	}

	// Does the relative path contain a drive letter?
	if( (pos = r.find(":")) != string::npos )
	{
		// The relative path is already a full path
		drive = r.substr(0, pos);
		path = r.substr(pos+1);
	}
	else if( r[0] == '/' )
	{
		// Relative path is a full path on the current drive
		path = r;
	}
	else
	{
		// Add the relative path to the base path
		path += r;
	}

	// Remove any ../ in the path
	for(;;)
	{
		pos = path.find("../");
		if( pos != string::npos && pos > 1 )
		{
			size_t p2 = path.rfind("/", pos-2);
			path = path.substr(0, p2+1) + path.substr(pos+3);
		}
		else
			break;
	}

	// Remove any ./ in the path
	for(;;)
	{
		pos = path.find("./");
		if( pos != string::npos && pos > 0 )
			path = path.substr(0, pos-1) + path.substr(pos+2);
		else
			break;
	}

	return drive + ":" + path;
}

string GetRelativePath(const string &base, const string &relative)
{
	string b = ReplacePathSlashes(base);
	string r = ReplacePathSlashes(relative);

	// Make sure the base path doesn't contain a filename and ends with /
	if( b[b.size()-1] != '/' )
	{
		size_t pos = b.rfind("/");
		b = b.substr(0, pos+1);
	}

	// Get the drive letter from the base path
	string drive, path;
	size_t pos = b.find(":");
	if( pos != string::npos )
	{
		drive = b.substr(0, pos);
		path = b.substr(pos+1);
	}
	else
		path = b;

	string driveRel, pathRel;
	if( (pos = r.find(":")) != string::npos )
	{
		// The relative path is a full path
		driveRel = r.substr(0, pos);
		pathRel = r.substr(pos+1);

		// Caseless comparison on Windows
		// TODO: Linux uses case sensitive file names
		if( _stricmp(driveRel.c_str(), drive.c_str()) != 0 )
		{
			// The drive is different, so the relative path must be taken as a whole
			drive = "";
			path = "";
		}
		else
		{
			// The drive letter won't be used anymore
			drive = "";
			driveRel = "";
		}
	}
	else if( r[0] == '/' )
	{
		// The relative path doesn't specify drive, but is an absolute path.
		// We'll assume it is the same drive as the base

		// The drive letter won't be used anymore
		drive = "";

		pathRel = r;
	}
	else
	{
		// If the relative path is already a relative path
		// then assume it is relative to the base path already
		
		// The base path and drive won't be used anymore
		drive = "";
		path = "";

		pathRel = r;
	}

	// TODO: Clean up the paths to remove unnecessary ../ and ./

	if( path != "" )
	{
		// Remove the common path

		// base   = /a/b/c/d/
		// rel    = /a/b/e/f/
		for(;;)
		{
			size_t pbase = path.find("/");
			size_t prel  = pathRel.find("/");

			// Caseless comparison on Windows
			// TODO: Linux uses case sensitive file names
			if( pbase != string::npos && prel != string::npos && 
				pbase == prel && _strnicmp(&path[0], &pathRel[0], pbase) == 0 )
			{
				path    = path.substr(pbase+1);
				pathRel = pathRel.substr(pbase+1);
			}
			else
				break;
		}

		// Add the necessary ../

		// base = c/d/
		// rel  = e/f/
		int count = 0;
		size_t pos = 0;
		for(;;)
		{
			pos = path.find("/", pos);
			if( pos++ != string::npos )
				count++;
			else
				break;
		}

		path = "";
		for( int n = 0; n < count; n++ )
			path += "../";

		pathRel = path + pathRel;
	}
	else if( driveRel != "" )
	{
		pathRel = driveRel + ":" + pathRel;
	}

	return pathRel;
}

}
