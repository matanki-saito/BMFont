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

#include <Windows.h>
#include "iconimagedlg.h"
#include "resource.h"
#include "acwin_filedialog.h"
#include "commctrl.h"

using namespace acWindow;

CIconImageDlg::CIconImageDlg() : CDialog()
{
	int id = 0;
	int xoffset = 0;
	int yoffset = 0;
	int advance = 0;
}

int CIconImageDlg::DoModal(CWindow *parent)
{
	return CDialog::DoModal(IDD_ICONIMAGE, parent);
}

LRESULT CIconImageDlg::MsgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch( msg )
	{
	case WM_INITDIALOG:
		OnInit();
		return TRUE;

	case WM_COMMAND:
		switch( LOWORD(wParam) )
		{
		case IDOK:
			GetOptions();
			EndDialog(hWnd, IDOK);
			break;

		case IDCANCEL:
			EndDialog(hWnd, IDCANCEL);
			break;

		case IDC_BROWSE:
			OnBrowse();
			break;
		}
		break;
	}

	return DefWndProc(msg, wParam, lParam);
}

void CIconImageDlg::OnBrowse()
{
	CFileDialog dlg;
	dlg.AddFilter("All files (*.*)", "*.*");
	dlg.AddFilter("Supported image files (*.bmp;*.jpg;*.tga;*.dds;*.png)", "*.bmp;*.jpg;*.tga;*.dds;*.png", true);

	if( dlg.AskForOpenFileName(this) )
	{
		TCHAR buf[512];
		ConvertUtf8ToTChar(dlg.GetFileName(), buf, 512);
		SetDlgItemText(hWnd, IDC_FILE, buf);
	}
}

void CIconImageDlg::OnInit()
{
	TCHAR buf[512];
	ConvertUtf8ToTChar(fileName, buf, 512);
	SetDlgItemText(hWnd, IDC_FILE, buf);
	SetDlgItemInt(hWnd, IDC_ID, id, FALSE);

	SetDlgItemInt(hWnd, IDC_XOFFSET, xoffset, TRUE);
	SendDlgItemMessage(hWnd, IDC_SPIN_XOFFSET, UDM_SETRANGE, 0, (LPARAM)MAKELONG(32767, -32768));

	SetDlgItemInt(hWnd, IDC_YOFFSET, yoffset, TRUE);
	SendDlgItemMessage(hWnd, IDC_SPIN_YOFFSET, UDM_SETRANGE, 0, (LPARAM)MAKELONG(32767, -32768));

	SetDlgItemInt(hWnd, IDC_ADVANCE, advance, TRUE);
	SendDlgItemMessage(hWnd, IDC_SPIN_ADVANCE, UDM_SETRANGE, 0, (LPARAM)MAKELONG(32767, -32768));
}


void CIconImageDlg::GetOptions()
{
	TCHAR buf[260];

	GetDlgItemText(hWnd, IDC_FILE, buf, 260);
	ConvertTCharToUtf8(buf, fileName);

	id = GetDlgItemInt(hWnd, IDC_ID, 0, FALSE);
	xoffset = GetDlgItemInt(hWnd, IDC_XOFFSET, 0, TRUE);
	yoffset = GetDlgItemInt(hWnd, IDC_YOFFSET, 0, TRUE);
	advance = GetDlgItemInt(hWnd, IDC_ADVANCE, 0, TRUE);
}




