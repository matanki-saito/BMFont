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

#ifndef IMAGEMGR_H
#define IMAGEMGR_H

#include "acwin_window.h"
#include "acwin_listview.h"

class CCharWin;
class CFontGen;

class CImageMgr : public acWindow::CWindow
{
public:
	CImageMgr();
	~CImageMgr();

	int Create(CCharWin *parent, CFontGen *gen);

	void RefreshList();

protected:
    LRESULT MsgProc(UINT msg, WPARAM wParam, LPARAM lParam);

	void OnSize();
	void OnImportImage();
	void OnInitMenuPopup(HMENU menu, int pos, BOOL isWindowMenu);
	void OnDeleteSelected();
	void OnEditImage();

	CCharWin *parent;
	CFontGen *fontGen;
	acWindow::CListView  *listView;
};

#endif