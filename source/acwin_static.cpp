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

#include <Windows.h>
#include "acwin_static.h"

namespace acWindow
{

CStatic::CStatic() : CWindow()
{
	isUrl = false;
}

LRESULT CStatic::MsgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_LBUTTONUP:
		if( isUrl )
		{
			GoUrl();
			return 0;
		}
		break;

	case WM_PAINT:
		if( isUrl )
		{
			OnPaint();
			return 0;
		}
		break;
	}

	return DefWndProc(msg, wParam, lParam);
}

void CStatic::MakeUrl(const char *url)
{
	this->url = url;
	isUrl = true;
}

void CStatic::GoUrl()
{
	TCHAR urlBuf[1024];
	ConvertUtf8ToTChar(url, urlBuf, 1024);

	ShellExecute(NULL, __TEXT("open"), urlBuf, NULL, NULL, SW_SHOWNORMAL);
}

void CStatic::OnPaint()
{
	TCHAR text[256];

	PAINTSTRUCT ps;
	HDC dc = BeginPaint(hWnd, &ps);

	HFONT font = (HFONT)SendMessage(hWnd, WM_GETFONT, 0, 0);
	HFONT oldFont = (HFONT)SelectObject(dc, font);

	GetWindowText(hWnd, text, 256);
	RECT rc;
	GetClientRect(hWnd, &rc);

	SetBkMode(dc, TRANSPARENT);
	SetTextColor(dc, RGB(0,0,255));
	
	DrawText(dc, text, -1, &rc, 0);
	DrawText(dc, text, -1, &rc, DT_CALCRECT);

	HPEN pen = CreatePen(PS_SOLID, 1, RGB(0,0,255));
	HPEN oldPen = (HPEN)SelectObject(dc, pen);

	MoveToEx(dc, rc.left, rc.bottom-1, 0);
	LineTo(dc, rc.right, rc.bottom-1);

	SelectObject(dc, oldPen);
	DeleteObject(pen);

	SelectObject(dc, oldFont);

	EndPaint(hWnd, &ps);
}

}
