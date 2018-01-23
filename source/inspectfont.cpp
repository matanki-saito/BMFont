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

#include <Windows.h>
#include <assert.h>
#include <sstream>

#include "inspectfont.h"
#include "charwin.h"
#include "fontgen.h"
#include "resource.h"
#include "acwin_filedialog.h"
#include "iconimagedlg.h"
#include "unicode.h"

using namespace std;
using namespace acWindow;

CInspectFont::CInspectFont() : CWindow()
{
	parent = 0;
	fontGen = 0;
	listView = 0;
	view = 0;
}

CInspectFont::~CInspectFont()
{
	if( listView )
		delete listView;
}

int CInspectFont::Create(CCharWin *parent, CFontGen *gen)
{
	this->parent = parent;
	this->fontGen = gen;

	DWORD style = WS_OVERLAPPED|WS_SYSMENU|WS_THICKFRAME;
	int r = CWindow::Create("Inspect Font", 300, 400, style, 0, parent, 0);
	if( r < 0 ) return r;

	HideSystemMenuButton();

	HMENU hMenu = GetSystemMenu(hWnd, FALSE);
	DeleteMenu(hMenu, SC_MINIMIZE, MF_BYCOMMAND);
	DeleteMenu(hMenu, SC_MAXIMIZE, MF_BYCOMMAND);
	DeleteMenu(hMenu, SC_RESTORE,  MF_BYCOMMAND);

	SetMenu(IDR_INSPECT_FONT_MENU);

	RECT rc;
	GetClientRect(hWnd, &rc);

	style = WS_CHILD | LVS_REPORT | WS_VISIBLE | 
			LVS_SHOWSELALWAYS | WS_CLIPSIBLINGS /* | LVS_NOCOLUMNHEADER*/;
	DWORD exStyle = LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT;

	listView = new CListView();
	listView->Create(style, exStyle, &rc, this, 0);
	listView->PassRightButtonToParent(true);

	listView->InsertColumn(0, "Attribute", 100);
	listView->InsertColumn(1, "Value", rc.right-100);

	OnViewBasic();

	ShowWindow(hWnd, SW_SHOW);

	return 0;
}

LRESULT CInspectFont::MsgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch( msg )
	{
	case WM_SIZE:
		OnSize();
		return 0;

	case WM_CLOSE:
		BringWindowToTop(parent->GetHandle());
		DestroyWindow(hWnd);
		return 0;

	case WM_DESTROY:
		// Tell the parent that we're no longer available
		parent->inspectFont = 0;
		delete this;
		return 0;

	case WM_INITMENUPOPUP:
		OnInitMenuPopup((HMENU)wParam, LOWORD(lParam), HIWORD(lParam));
		return 0;

	case WM_COMMAND:
		switch( LOWORD(wParam) )
		{
		case ID_VIEW_BASIC:
			OnViewBasic();
			return 0;

		case ID_VIEW_KERN:
			OnViewKERN();
			return 0;

		case ID_VIEW_GPOS:
			OnViewGPOS();
			return 0;

		case ID_VIEW_GSUB:
			OnViewGSUB();
			return 0;
		}
		break;
	}

	return DefWndProc(msg, wParam, lParam);
}

void CInspectFont::OnSize()
{
	if( listView )
	{
		RECT rc;
		GetClientRect(hWnd, &rc);
		MoveWindow(listView->GetHandle(), rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, TRUE);

		listView->SetColumnWidth(1, rc.right - 100);
	}
}

void CInspectFont::OnViewBasic()
{
	view = 0;
	if (!listView) return;
	listView->DeleteAllItems();

	int countItem = 0;

	// Read basic info and add to list view
	listView->InsertItem(countItem, "Font name");
	listView->SetItemText(countItem++, 1, fontGen->GetFontName());

	// Identify the font file name
	listView->InsertItem(countItem, "File name");
	listView->SetItemText(countItem++, 1, GetFontFileName(fontGen->GetFontName(), fontGen->IsBold(), fontGen->IsItalic()));
}

void CInspectFont::OnViewKERN()
{
	view = 1;
	if (!listView) return;
	listView->DeleteAllItems();

	// TODO: Read KERN table and add to list view
}

void CInspectFont::OnViewGPOS()
{
	view = 2;
	if (!listView) return;
	listView->DeleteAllItems();

	// TODO: Read GPOS table and add to list view

}

void CInspectFont::OnViewGSUB()
{
	UINT listViewCount = 0;

	view = 3;
	if (!listView) return;
	listView->DeleteAllItems();

	// Read GSUB table and add to list view
	// ref: https://www.microsoft.com/typography/otspec/gsub.htm
	HDC dc = GetDC(NULL);
	HFONT font = fontGen->CreateFont(20);
	HFONT oldFont = (HFONT)SelectObject(dc, font);

	vector<BYTE> buffer;
	DWORD GSUB = TAG('G', 'S', 'U', 'B');
	DWORD size = GetFontData(dc, GSUB, 0, 0, 0);
	if (size != GDI_ERROR)
	{
		buffer.resize(size);
		size = GetFontData(dc, GSUB, 0, &buffer[0], size);
		if (size == GDI_ERROR)
		{
			SelectObject(dc, oldFont);
			DeleteObject(font);
			return;
		}
	}

	WORD majorVersion = GETUSHORT(&buffer[0]);
	assert(majorVersion == 0x0001);

	WORD minorVersion = GETUSHORT(&buffer[2]);
	assert(minorVersion == 0 || minorVersion == 1);

	WORD offsetScriptList = GETUSHORT(&buffer[4]);
	WORD offsetFeatureList = GETUSHORT(&buffer[6]);
	WORD offsetLookupList = GETUSHORT(&buffer[8]);
	DWORD offsetFeatureVariations = minorVersion == 1 ? GETUINT(&buffer[10]) : 0;

	// Determine how many different scripts are supported
	WORD scriptCount = GETUSHORT(&buffer[offsetScriptList]);

	{
		listView->InsertItem(listViewCount, "Script count");
		stringstream s;
		s << scriptCount;

		listView->SetItemText(listViewCount++, 1, s.str());
	}
	
	{
		stringstream s;
		for (UINT n = 0; n < scriptCount; n++)
		{
			if (n > 0)
				s << ", ";

			DWORD tag = GETUINT(&buffer[offsetScriptList + 2 + n * 6]);
			char name[5] = { 0 };
			name[3] = tag & 0xFF;
			name[2] = (tag >> 8) & 0xFF;
			name[1] = (tag >> 16) & 0xFF;
			name[0] = (tag >> 24) & 0xFF;

			s << name;

			WORD offset = GETUSHORT(&buffer[offsetScriptList + 2 + n * 6 + 4]);
		}

		listView->InsertItem(listViewCount, "Script tags");
		listView->SetItemText(listViewCount++, 1, s.str());
	}

	SelectObject(dc, oldFont);
	DeleteObject(font);
}

void CInspectFont::OnInitMenuPopup(HMENU menu, int pos, BOOL isWindowMenu)
{
	CheckMenuItem(menu, ID_VIEW_BASIC, MF_BYCOMMAND | (view == 0 ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(menu, ID_VIEW_KERN, MF_BYCOMMAND  | (view == 1 ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(menu, ID_VIEW_GPOS, MF_BYCOMMAND  | (view == 2 ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(menu, ID_VIEW_GSUB, MF_BYCOMMAND  | (view == 3 ? MF_CHECKED : MF_UNCHECKED));

	// Check if KERN, GPOS, GSUB tables are available and disable those that are not
	HDC dc = GetDC(NULL);
	HFONT font = fontGen->CreateFont(20);
	HFONT oldFont = (HFONT)SelectObject(dc, font);

	DWORD KERN = TAG('k', 'e', 'r', 'n');
	DWORD size = GetFontData(dc, KERN, 0, 0, 0);
	EnableMenuItem(menu, ID_VIEW_KERN, MF_BYCOMMAND | (size != GDI_ERROR ? MF_ENABLED : MF_GRAYED));
	DWORD GPOS = TAG('G', 'P', 'O', 'S');
	size = GetFontData(dc, GPOS, 0, 0, 0);
	EnableMenuItem(menu, ID_VIEW_GPOS, MF_BYCOMMAND | (size != GDI_ERROR ? MF_ENABLED : MF_GRAYED));
	DWORD GSUB = TAG('G', 'S', 'U', 'B');
	size = GetFontData(dc, GSUB, 0, 0, 0);
	EnableMenuItem(menu, ID_VIEW_GSUB, MF_BYCOMMAND | (size != GDI_ERROR ? MF_ENABLED : MF_GRAYED));

	SelectObject(dc, oldFont);
	DeleteObject(font);
}

