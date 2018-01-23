/*
   AngelCode Bitmap Font Generator
   Copyright (c) 2004-2016 Andreas Jonsson
  
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
#include <string>
#include "about.h"
#include "resource.h"

using namespace acWindow;

CAbout::CAbout() : CDialog()
{

}

int CAbout::DoModal(CWindow *parent)
{
	return CDialog::DoModal(IDD_ABOUT, parent);
}

LRESULT CAbout::MsgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch( msg )
	{
	case WM_INITDIALOG:
		urlAngelCode.Subclass(GetDlgItem(hWnd, IDC_URL1));
		urlLibPng.Subclass(GetDlgItem(hWnd, IDC_URL_LIBPNG));
		urlLibJpeg.Subclass(GetDlgItem(hWnd, IDC_LIBJPEG));
		urlZLib.Subclass(GetDlgItem(hWnd, IDC_URL_ZLIB));
		urlSquish.Subclass(GetDlgItem(hWnd, IDC_SQUISH));
		urlAngelCode.MakeUrl("http://www.angelcode.com");
		urlLibPng.MakeUrl("http://www.libpng.org");
		urlLibJpeg.MakeUrl("http://www.ijg.org");
		urlZLib.MakeUrl("http://www.zlib.net");
		urlSquish.MakeUrl("http://sourceforge.net/projects/libsquish/");
		break;

	case WM_COMMAND:
		switch( LOWORD(wParam) )
		{
		case IDOK:
			EndDialog(hWnd, IDOK);
			break;

		case IDCANCEL:
			EndDialog(hWnd, IDCANCEL);
			break;
		}
		break;
	}

	return DefWndProc(msg, wParam, lParam);
}


