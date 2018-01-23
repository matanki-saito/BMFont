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

#ifndef IMAGEWND_H
#define IMAGEWND_H

#include "acwin_window.h"
#include "ac_image.h"

class CFontGen;

class cImageWnd : public acWindow::CWindow
{
public:
	cImageWnd();

	int Create(acWindow::CWindow *parent, CFontGen *fontGen);
	void CopyImage(cImage *img);

protected:
	LRESULT MsgProc(UINT msg, WPARAM wParam, LPARAM lParam);

	void OnPaint();
	void OnSaveAs();
	void OnSize();
	void OnScroll(UINT scrollBar, UINT action);
	void OnScale(float newScale);
	void OnInitMenuPopup(HMENU hMenu, int pos, BOOL isWindowMenu);
	void OnChangePage(int direction);

	void UpdateScrollBars();
	void ProcessImage();

	int GetHorzScroll();
	int GetVertScroll();

	void Draw();
	void DrawImage(int x, int y);

	cImage buffer;
	cImage originalImage;
	cImage image;
	float scale;
	bool viewAlpha;
	bool viewTiled;

	acWindow::CWindow *parent;
	CFontGen *fontGen;

	int page;
	int chnl;
};

#endif