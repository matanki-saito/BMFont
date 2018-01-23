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

#include <Windows.h>
#include <assert.h>
#include <sstream>

#include "imagemgr.h"
#include "charwin.h"
#include "fontgen.h"
#include "resource.h"
#include "acwin_filedialog.h"
#include "iconimagedlg.h"

using namespace std;
using namespace acWindow;

CImageMgr::CImageMgr() : CWindow()
{
	parent = 0;
	fontGen = 0;
	listView = 0;
}

CImageMgr::~CImageMgr()
{
	if( listView )
		delete listView;
}

int CImageMgr::Create(CCharWin *parent, CFontGen *gen)
{
	this->parent = parent;
	this->fontGen = gen;

	DWORD style = WS_OVERLAPPED|WS_SYSMENU|WS_THICKFRAME;
	int r = CWindow::Create("Image Manager", 300, 400, style, 0, parent, 0);
	if( r < 0 ) return r;

	HideSystemMenuButton();

	HMENU hMenu = GetSystemMenu(hWnd, FALSE);
	DeleteMenu(hMenu, SC_MINIMIZE, MF_BYCOMMAND);
	DeleteMenu(hMenu, SC_MAXIMIZE, MF_BYCOMMAND);
	DeleteMenu(hMenu, SC_RESTORE,  MF_BYCOMMAND);

	SetMenu(IDR_IMG_MGR_MENU);

	RECT rc;
	GetClientRect(hWnd, &rc);

	style = WS_CHILD | LVS_REPORT | WS_VISIBLE | 
			LVS_SHOWSELALWAYS | WS_CLIPSIBLINGS | LVS_NOCOLUMNHEADER;
	DWORD exStyle = LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT;

	listView = new CListView();
	listView->Create(style, exStyle, &rc, this, 0);
	listView->PassRightButtonToParent(true);

	listView->InsertColumn(0, "Id", 50);
	listView->InsertColumn(1, "File", rc.right-50);

	RefreshList();

	ShowWindow(hWnd, SW_SHOW);

	return 0;
}

LRESULT CImageMgr::MsgProc(UINT msg, WPARAM wParam, LPARAM lParam)
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
		parent->imageMgr = 0;
		delete this;
		return 0;

	case WM_INITMENUPOPUP:
		OnInitMenuPopup((HMENU)wParam, LOWORD(lParam), HIWORD(lParam));
		return 0;

	case WM_COMMAND:
		switch( LOWORD(wParam) )
		{
		case ID_IMAGE_IMPORTIMAGE:
			OnImportImage();
			return 0;

		case ID_IMAGE_DELETESELECTED:
			OnDeleteSelected();
			return 0;

		case ID_IMAGE_EDIT:
			OnEditImage();
			return 0;
		}
		break;
	}

	return DefWndProc(msg, wParam, lParam);
}

void CImageMgr::OnSize()
{
	if( listView )
	{
		RECT rc;
		GetClientRect(hWnd, &rc);
		MoveWindow(listView->GetHandle(), rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, TRUE);

		listView->SetColumnWidth(1, rc.right-50);
	}
}

void CImageMgr::RefreshList()
{
	if( listView )
	{
		listView->DeleteAllItems();

		for( int n = 0; n < fontGen->GetIconImageCount(); n++ )
		{
			int id, xoffset, yoffset, advance;
			string filename;
			fontGen->GetIconImageInfo(n, filename, id, xoffset, yoffset, advance);

			stringstream id_stream;
			id_stream << id;

			listView->InsertItem(n, id_stream.str(), id);
			listView->SetItemText(n, 1, filename);
		}
	}

	// Invalidate the parent so the image indicators are updated
	parent->Invalidate(FALSE);
}

void CImageMgr::OnImportImage()
{
	if( fontGen->GetStatus() != 0 ) return;

	CFileDialog dlg;
	dlg.AddFilter("All files (*.*)", "*.*");
	dlg.AddFilter("Supported image files (*.bmp;*.jpg;*.tga;*.dds;*.png)", "*.bmp;*.jpg;*.tga;*.dds;*.png", true);

	if( dlg.AskForOpenFileName(this) )
	{
		CIconImageDlg iconDlg;
		iconDlg.fileName = dlg.GetFileName();
		iconDlg.id = 0;
		iconDlg.xoffset = 0;
		iconDlg.yoffset = 0;
		iconDlg.advance = 0;

		if( iconDlg.DoModal(this) == IDOK )
		{
			// Create the icon image
			int r = fontGen->AddIconImage(iconDlg.fileName.c_str(), 
			                              iconDlg.id,
										  iconDlg.xoffset,
										  iconDlg.yoffset,
										  iconDlg.advance);
			if( r < 0 )
			{
				MessageBox(hWnd, __TEXT("Failed to load image file"), __TEXT("File error"), MB_OK);
				return;
			}

			RefreshList();
		}
	}
}

void CImageMgr::OnDeleteSelected()
{
	if( fontGen->GetStatus() != 0 ) return;
	if( !listView ) return;

	bool deleted = false;
	
	int item = -1;
	while( (item = listView->GetNextItem(item, LVNI_SELECTED)) != -1 )
	{
		int id;
		listView->GetItemParam(item, (LPARAM*)&id);
		if( fontGen->DeleteIconImage(id) == 0 )
			deleted = true;
	}

	if( deleted )
		RefreshList();
}

void CImageMgr::OnEditImage()
{
	if( fontGen->GetStatus() != 0 ) return;
	if( !listView ) return;

	int item = -1;
	if( (item = listView->GetNextItem(item, LVNI_FOCUSED)) != -1 )
	{
		int oldId;
		listView->GetItemParam(item, (LPARAM*)&oldId);

		CIconImageDlg iconDlg;
		fontGen->GetIconImageInfo(item, iconDlg.fileName, iconDlg.id, iconDlg.xoffset, iconDlg.yoffset, iconDlg.advance);

		if( iconDlg.DoModal(this) == IDOK )
		{
			int r = fontGen->UpdateIconImage(oldId, iconDlg.id, iconDlg.fileName.c_str(), iconDlg.xoffset, iconDlg.yoffset, iconDlg.advance);
			if( r >= 0 )
				RefreshList();
		}
	}
}

void CImageMgr::OnInitMenuPopup(HMENU menu, int pos, BOOL isWindowMenu)
{
	if( fontGen->GetStatus() == 0 )
	{
		EnableMenuItem(menu, ID_IMAGE_IMPORTIMAGE       , MF_BYCOMMAND | MF_ENABLED);
		EnableMenuItem(menu, ID_IMAGE_DELETESELECTED    , MF_BYCOMMAND | MF_ENABLED);
		EnableMenuItem(menu, ID_IMAGE_EDIT              , MF_BYCOMMAND | MF_ENABLED);
	}
	else
	{
		EnableMenuItem(menu, ID_IMAGE_IMPORTIMAGE       , MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(menu, ID_IMAGE_DELETESELECTED    , MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(menu, ID_IMAGE_EDIT              , MF_BYCOMMAND | MF_GRAYED);
	}
}

