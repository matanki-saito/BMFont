/*
   AngelCode Tool Box Library
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

// 2014-06-16  Prepared the code to work for both unicode and multibyte applications
// 2012-08-12  Added SetInitialDir

#ifndef ACWIN_FILEDIALOG_H
#define ACWIN_FILEDIALOG_H

#include <string>
#include "acwin_window.h"

namespace acWindow
{
class CFileDialog
{
public:
	CFileDialog();

	int         AskForOpenFileName(CWindow *owner);
	int         AskForSaveFileName(CWindow *owner);
	void        AddFilter(const char *desc, const char *ext, bool isDefault = false);
	void        SetDefaultFilter(int index);
	int         GetSelectedFilter();
	void        SetFileName(const char *name);
	std::string GetFileName();

	// Should be the name of the directory, without a trailing /
	void        SetInitialDir(const char *dir);

protected:
	TCHAR       szFile[260];
	std::string fileFilter;
	int         filterIndex;
	int         numFilters;
	std::string initialDir;
};

}

#endif