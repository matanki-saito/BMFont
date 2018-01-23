/*
   AngelCode Bitmap Font Generator
   Copyright (c) 2017 Andreas Jonsson
  
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

#ifndef INSPECTFONT_H
#define INSPECTFONT_H

#include "acwin_window.h"
#include "acwin_listview.h"

class CCharWin;
class CFontGen;

class CInspectFont : public acWindow::CWindow
{
public:
	CInspectFont();
	~CInspectFont();

	int Create(CCharWin *parent, CFontGen *gen);

protected:
    LRESULT MsgProc(UINT msg, WPARAM wParam, LPARAM lParam);

	void OnSize();
	void OnInitMenuPopup(HMENU menu, int pos, BOOL isWindowMenu);
	void OnViewBasic();
	void OnViewKERN();
	void OnViewGPOS();
	void OnViewGSUB();

	int view;

	CCharWin *parent;
	CFontGen *fontGen;
	acWindow::CListView  *listView;
};

#endif