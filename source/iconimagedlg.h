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

#ifndef ICONIMAGEDLG_H
#define ICONIMAGEDLG_H

#include <string>
using std::string;

#include "acwin_dialog.h"

class CIconImageDlg : virtual public acWindow::CDialog
{
public:
	CIconImageDlg();

	int DoModal(acWindow::CWindow *parent);

	string fileName;
	int id;
	int xoffset;
	int yoffset;
	int advance;

protected:
	void OnInit();
	void GetOptions();
	void OnBrowse();

	LRESULT MsgProc(UINT msg, WPARAM wParam, LPARAM lParam);
};

#endif