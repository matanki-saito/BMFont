/*
   AngelCode Bitmap Font Generator
   Copyright (c) 2004-2017 Andreas Jonsson
  
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

#ifndef CHARWIN_H
#define CHARWIN_H

#include <string>
using std::string;
#include <vector>
using std::vector;

#include "acwin_window.h"
#include "acwin_listview.h"
#include "acwin_statusbar.h"
#include "imagewnd.h"
#include "fontgen.h"

class CImageMgr;
class CInspectFont;

class CCharWin : public acWindow::CWindow
{
public:
	CCharWin();
	~CCharWin();

	int Create(int width, int height, const string &configFile);
	void Draw();

	void GetCharGridRect(RECT *rc);
	static string GetDefaultConfig();

protected:
	friend class CImageMgr;
	friend class CInspectFont;

	void OnChooseFont();
	void OnExportOptions();
	void OnVisualize();
	void OnSaveAs();
	void OnLoadConfiguration();
	void OnSaveConfiguration();
	void OnAbout();
	void OnSize();
	void OnInitMenuPopup(HMENU menu, int pos, BOOL isWindowMenu);
	void OnRButtonDown(int x, int y);
	void OnSelectCharsFromFile();
	void OnTimer();

	void LoadConfig(const string &configFile);
	void PrepareView();

	void DrawGlyphs(HDC dc, RECT &rc, TEXTMETRIC &tm);

	void UpdateStatus();
	void UpdateSubsetsSelection();

	void VisualizeAfterFinishedGenerating();
	void SaveFontAfterFinishedGenerating();

	int GetCharFromPos(int x, int y);

	LRESULT MsgProc(UINT msg, WPARAM wParam, LPARAM lParam);

	cImageWnd            *wnd;
	acWindow::CListView  *listView;
	acWindow::CStatusBar *statusBar;
	CImageMgr            *imageMgr;
	CInspectFont         *inspectFont;

	bool isDragging;
	bool isPreparing;
	bool selectMode;
	int unicodeSubset;
	CFontGen *fontGen;
	bool isGenerating;
	int whenGenerateIsFinished;
	string saveFontName;
};

#endif