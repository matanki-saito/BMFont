/*
   AngelCode Tool Box Library
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

// 2017-05-28 Included SetColumnWidth and DeleteAllItems
// 2016-02-21 Fixes for Win64
// 2014-06-15 Support compiling for both unicode and multibyte Windows
// 2011-08-23 Added column parameter to Get/SetItemText
// 2011-08-23 Added SetEditColumn

#include <Windows.h>
#include "acwin_listview.h"
#include <assert.h>

using namespace std;

namespace acWindow
{

CListView::CListView() : CWindow()
{
	doPassRightButtonToParent = false;
	editColumn = 0;
}

int CListView::Create(DWORD style, DWORD exStyle, RECT *rc, CWindow *parent, UINT id)
{
	// Initialize the common control
	INITCOMMONCONTROLSEX icc;
	icc.dwSize = sizeof(icc);
	icc.dwICC = ICC_LISTVIEW_CLASSES;
	InitCommonControlsEx(&icc);

	// style = LVS_SINGLESEL | WS_CHILD | LVS_REPORT | WS_VISIBLE | 
	//         LVS_SHOWSELALWAYS | WS_CLIPSIBLINGS | LVS_NOSORTHEADER;

	HWND parentWnd = parent ? parent->GetHandle() : 0;
	HWND listView = CreateWindow(WC_LISTVIEW, __TEXT(""), 
		style, rc->left, rc->top, rc->right - rc->left, rc->bottom - rc->top, 
		parentWnd, (HMENU)size_t(id), GetModuleHandle(0), 0); 
	if( Subclass(listView) < 0 ) 
		return -1; 

	// exStyle = LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT;

	ListView_SetExtendedListViewStyle(listView, exStyle);

	return 0;
}

int CListView::FindItem(int start, const string &text)
{
	assert( text.length() < 256 );

	TCHAR buf[256];
	ConvertUtf8ToTChar(text, buf, 256);

	LVFINDINFO fi;
	fi.flags = LVFI_STRING|LVFI_WRAP;
	fi.psz = buf;

	return ListView_FindItem(hWnd, start, &fi);
}

int CListView::FindItemByParam(int start, LPARAM lparam)
{
	LVFINDINFO fi;
	fi.flags = LVFI_PARAM|LVFI_WRAP;
	fi.lParam = lparam;

	return ListView_FindItem(hWnd, start, &fi);
}

// TODO: The list view class needs to override the WM_PAINT message to properly render the cells when editing
//       http://www.codeguru.com/cpp/controls/listview/editingitemsandsubitem/article.php/c4175/Simplified-Subitem-Editing.htm

LRESULT CListView::MsgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch( msg )
	{
	case WM_DESTROY:
		DefWndProc(msg, wParam, lParam);
		Detach();
		return 0;

	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
		if( doPassRightButtonToParent )
		{
			// Convert the mouse position to the parent window client coordinates
			POINT pt = {LOWORD(lParam), HIWORD(lParam)};
			ClientToScreen(hWnd, &pt);
			ScreenToClient((HWND)GetWindowLongPtr(hWnd, GWLP_HWNDPARENT), &pt);
			lParam = (pt.x&0xFFFF) + ((pt.y&0xFFFF)<<16);
			return SendMessage((HWND)GetWindowLongPtr(hWnd, GWLP_HWNDPARENT), msg, wParam, lParam);
		}
		return 0;

	case WM_PAINT:
		if( editColumn )
		{
			RECT rc;
			ListView_GetSubItemRect(hWnd, editItem, editColumn, LVIR_LABEL, &rc);

			HWND edit = (HWND)SendMessage(hWnd, LVM_GETEDITCONTROL, 0, 0);
			RECT editRect;
			GetWindowRect(edit, &editRect);
			ScreenToClient(hWnd, (POINT*)&editRect.left);
			ScreenToClient(hWnd, (POINT*)&editRect.right);

			// Don't allow redrawing the text under the edit control
			// otherwise the original text may show if the text in the edit
			// control is removed
			if( editRect.right < rc.right )
			{
				rc.left = editRect.right;
				ValidateRect(hWnd, &rc);
			}

			// Also disallow redrawing the leftmost cell so it cannot be erased
			ListView_GetSubItemRect(hWnd, editItem, 0, LVIR_LABEL, &rc);
			ValidateRect(hWnd, &rc);
		}
		// Allow the original process do the rest of the painting
		break;
	}

	return DefWndProc(msg, wParam, lParam);
}

void CListView::PassRightButtonToParent(bool pass)
{
	doPassRightButtonToParent = pass;
}

int CListView::InsertColumn(UINT col, const string &name, UINT width)
{
	assert( width < 256 );

	TCHAR buf[256];
	ConvertUtf8ToTChar(name, buf, 256);

	LVCOLUMN lvc; 
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM; 
	lvc.iSubItem = col;
	lvc.pszText = buf;
	lvc.cx = width;         // width of column in pixels
	lvc.fmt = LVCFMT_LEFT;  // left-aligned column

	if( ListView_InsertColumn(hWnd, col, &lvc) == -1 ) 
		return -1; 

	return 0;
}

int CListView::SetColumnWidth(UINT col, UINT width)
{
	if (ListView_SetColumnWidth(hWnd, col, width) != TRUE)
		return -1;

	return 0;
}

int CListView::InsertItem(UINT item)
{
	LVITEM lvi;
	lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE; 
	lvi.state = 0; 
	lvi.stateMask = 0; 
	lvi.iSubItem = 0;
	lvi.lParam = 0; 
	lvi.pszText = LPSTR_TEXTCALLBACK;
	lvi.iItem = item;

	if( ListView_InsertItem(hWnd, &lvi) == -1 )
		return -1;

	return 0;
}

int CListView::InsertItem(UINT item, const string &text, long param)
{
	assert( text.length() < 256 );

	TCHAR buf[256];
	ConvertUtf8ToTChar(text, buf, 256);

	LVITEM lvi;
	lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE; 
	lvi.stateMask = 0;
	lvi.state = 0;
	lvi.iSubItem = 0;
	lvi.lParam = param; 
	lvi.pszText = buf;
	lvi.iItem = item;

	if( ListView_InsertItem(hWnd, &lvi) == -1 )
		return -1;

	return 0;
}

int CListView::DeleteItem(UINT item)
{
	return ListView_DeleteItem(hWnd, item) ? 0 : -1;
}

int CListView::DeleteAllItems()
{
	return ListView_DeleteAllItems(hWnd) ? 0 : -1;
}

int CListView::GetItemParam(UINT item, LPARAM *param)
{
	LVITEM lvi;
	lvi.mask = LVIF_PARAM; 
	lvi.state = 0; 
	lvi.stateMask = 0; 
	lvi.iSubItem = 0;
	lvi.lParam = 0; 
	lvi.pszText = 0;
	lvi.iItem = item;

	int r = ListView_GetItem(hWnd, &lvi);

	*param = lvi.lParam;

	return r ? 0 : -1;
}

int CListView::GetNextItem(int start, UINT flags)
{
	return (int)SendMessage(hWnd, LVM_GETNEXTITEM, start, flags);
}

void CListView::GetItemSubRect(UINT item, UINT col, RECT *rc)
{
	rc->top = col; // Value column
	rc->left = LVIR_BOUNDS;
	SendMessage(hWnd, LVM_GETSUBITEMRECT, (WPARAM)item, (LPARAM)rc);
}

void CListView::SetItemState(int item, UINT mask, UINT state)
{
	LVITEM lvi;
	lvi.iItem = item;
	lvi.mask = LVIF_STATE;
	lvi.stateMask = mask;
	lvi.state = state;
	int ret = (int)SendMessage(hWnd, LVM_SETITEMSTATE, item, (LPARAM)&lvi);
}

void CListView::EnsureVisible(UINT item)
{
	ListView_EnsureVisible(hWnd, item, FALSE);
}

void CListView::SetItemImage(UINT item, int image)
{
	LVITEM lvi;
	lvi.iItem = item;
	lvi.iSubItem = 0;
	lvi.iImage = image;
	lvi.mask = LVIF_IMAGE;
	SendMessage(hWnd, LVM_SETITEM, 0, (LPARAM)&lvi);
}

int CListView::GetItemImage(UINT item)
{
	LVITEM lvi;
	lvi.iItem = item;
	lvi.iSubItem = 0;
	lvi.mask = LVIF_IMAGE;
	SendMessage(hWnd, LVM_SETITEM, 0, (LPARAM)&lvi);

	return lvi.iImage;
}

int CListView::GetItemText(UINT item, UINT column, string *text)
{
	TCHAR buffer[256] = {0};
	LVITEM lvi;
	lvi.iSubItem = column;
	lvi.pszText = buffer;
	lvi.cchTextMax = 256;
	int r = (int)SendMessage(hWnd, LVM_GETITEMTEXT, item, (LPARAM)&lvi);

	ConvertTCharToUtf8(buffer, *text);

	return r > 0 ? 0 : -1;
}

void CListView::SetItemText(UINT item, UINT column, const string &text)
{
	assert( text.length() < 256 );

	TCHAR buf[256];
	ConvertUtf8ToTChar(text, buf, 256);

	LVITEM lvi;
	lvi.iSubItem = column;
	lvi.pszText = buf;
	SendMessage(hWnd, LVM_SETITEMTEXT, item, (LPARAM)&lvi);
}

int CListView::GetItemStateImage(UINT item)
{
	return ListView_GetItemState(hWnd, item, LVIS_STATEIMAGEMASK) >> 12;
}

void CListView::SetItemStateImage(UINT item, int image)
{
	LVITEM lvi;
	lvi.stateMask = LVIS_STATEIMAGEMASK;
	lvi.state = INDEXTOSTATEIMAGEMASK(image);
	SendMessage(hWnd, LVM_SETITEMSTATE, item, (LPARAM)&lvi);
}

void CListView::BeginLabelEdit(NMLVDISPINFO *info, UINT column)
{
	editColumn = column;
	editItem = info->item.iItem;

	// Sub class the edit control to override the position
	HWND edit = (HWND)SendMessage(hWnd, LVM_GETEDITCONTROL, 0, 0);
	originalEditProc = (WNDPROC)SetWindowLongPtr(edit, GWLP_WNDPROC, (LONG_PTR)EditMsgProc);

	// Set the text from the column
	string text;
	GetItemText(info->item.iItem, column, &text);

	assert( text.length() < 256 );

	TCHAR buf[256];
	ConvertUtf8ToTChar(text, buf, 256);

	SetWindowText(edit, buf);
}

UINT CListView::GetEditColumn()
{
	return editColumn;
}

void CListView::EndLabelEdit(NMLVDISPINFO *info)
{
	if( info->item.pszText )
	{
		// Manually update the subitem text
		string text;
		ConvertTCharToUtf8(info->item.pszText, text);

		SetItemText(info->item.iItem, editColumn, text);
	}

	// Reset the edit column
	editColumn = 0;
}

// static
LRESULT CALLBACK CListView::EditMsgProc(HWND hWnd, UINT msg, 
							            WPARAM wParam, LPARAM lParam)
{
	HWND parent = GetParent(hWnd);
	CListView *lv = (CListView*)FromHandle(parent);
	if( lv )
	{
		if( msg == WM_WINDOWPOSCHANGING )
		{
			// Override the position to place the edit control on the right spot
			RECT rc;
			lv->GetItemSubRect(0, lv->editColumn, &rc);
			WINDOWPOS *pos = (WINDOWPOS*)lParam;
			pos->x = rc.left;
			return 0;
		}

		// Call the original process
		return CallWindowProc(lv->originalEditProc, hWnd, msg, wParam, lParam);
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}


}

// 2008-05-09 Added FindItemByParam

