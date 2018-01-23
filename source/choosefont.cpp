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
#include <Strsafe.h>

#include "choosefont.h"
#include "resource.h"
#include "commctrl.h"
#include "unicode.h"
#include "acwin_filedialog.h"

using namespace acWindow;

typedef DWORD (_stdcall *GetGlyphIndices_t)(HDC hdc, LPCTSTR lpstr, int c, LPWORD pgi, DWORD fl);
extern GetGlyphIndices_t fGetGlyphIndicesA;
extern GetGlyphIndices_t fGetGlyphIndicesW;

int CALLBACK ChooseFontCallback(
  ENUMLOGFONTEX *lpelfe,     // logical-font data
  NEWTEXTMETRICEX *lpntme,   // physical-font data
  DWORD FontType,            // type of font
  LPARAM lParam              // application-defined data
);

int CALLBACK ChooseFontCallback2(
  ENUMLOGFONTEX *lpelfe,    // logical-font data
  NEWTEXTMETRICEX *lpntme,  // physical-font data
  DWORD FontType,           // type of font
  LPARAM lParam             // application-defined data
);

CChooseFont::CChooseFont() : CDialog()
{

}

int CChooseFont::DoModal(CWindow *parent)
{
	return CDialog::DoModal(IDD_CHOOSEFONT, parent);
}

LRESULT CChooseFont::MsgProc(UINT msg, WPARAM wParam, LPARAM lParam)
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

		case IDC_FONT:
			if( HIWORD(wParam) == CBN_SELCHANGE )
				OnFontChange();
			break;

		case IDC_BROWSEFONT:
			OnBrowseFont();
			break;

		case IDC_FONTFILE:
			if( HIWORD(wParam) == EN_KILLFOCUS )
				EnumFonts();
			break;

		case IDC_ENABLEAA:
		case IDC_USEUNICODE:
		case IDC_USEOEM:
		case IDC_RENDERFROMOUTLINE:
		case IDC_SMOOTH:
			EnableWidgets();
			break;
		}
		break;
	}

	return DefWndProc(msg, wParam, lParam);
}

void CChooseFont::EnableWidgets()
{
	// ASCII charset combo box
	EnableWindow(GetDlgItem(hWnd, IDC_CHARSET), !IsDlgButtonChecked(hWnd, IDC_USEUNICODE));

	// Super sampling
	EnableWindow(GetDlgItem(hWnd, IDC_ANTIALIASING), IsDlgButtonChecked(hWnd, IDC_ENABLEAA));

	// Disable hinting is only available when rendering from outline
	EnableWindow(GetDlgItem(hWnd, IDC_HINTING), IsDlgButtonChecked(hWnd, IDC_RENDERFROMOUTLINE));

	// Clear type is only available when using font smoothing and native renderer
	EnableWindow(GetDlgItem(hWnd, IDC_CLEARTYPE), !IsDlgButtonChecked(hWnd, IDC_RENDERFROMOUTLINE) && IsDlgButtonChecked(hWnd, IDC_SMOOTH));
}

void CChooseFont::OnFontChange()
{
	TCHAR buf[256];
	GetDlgItemText(hWnd, IDC_CHARSET, buf, 256);

	string charsetName;
	ConvertTCharToUtf8(buf, charsetName);

	if( charsetName != "" )
		charSet = GetCharSet(charsetName.c_str());

	SendDlgItemMessage(hWnd, IDC_CHARSET, CB_RESETCONTENT, 0, 0);

	int idx = (int)SendDlgItemMessage(hWnd, IDC_FONT, CB_GETCURSEL, 0, 0);
	if( idx != CB_ERR )
	{
		// Enumerate the charsets for the font
		LOGFONT lf;
		lf.lfCharSet = DEFAULT_CHARSET;
		lf.lfPitchAndFamily = 0;

		TCHAR buf[256];
		SendDlgItemMessage(hWnd, IDC_FONT, CB_GETLBTEXT, idx, (LPARAM)buf);
		StringCchCopy(lf.lfFaceName, LF_FACESIZE, buf);
		lf.lfFaceName[LF_FACESIZE-1] = 0;

		HDC dc = GetDC(0);
		EnumFontFamiliesEx(dc, &lf, (FONTENUMPROC)ChooseFontCallback2, (LPARAM)this, 0);
		ReleaseDC(0, dc);
	}

	string str = GetCharSetName(charSet);
	ConvertUtf8ToTChar(str, buf, 256);
	int r = (int)SendDlgItemMessage(hWnd, IDC_CHARSET, CB_SELECTSTRING, -1, (LPARAM)buf);
	if( r == CB_ERR )
		SendDlgItemMessage(hWnd, IDC_CHARSET, CB_SETCURSEL, 0, 0);
}

void CChooseFont::OnBrowseFont()
{
	CFileDialog dlg;
	dlg.AddFilter("Windows font files", "*.fon;*.fnt;*.ttf;*.ttc;*.fot;*.otf;", true);
	if( dlg.AskForOpenFileName(this) )
	{
		TCHAR buf[1024];
		ConvertUtf8ToTChar(dlg.GetFileName(), buf, 1024);

		SetDlgItemText(hWnd, IDC_FONTFILE, buf);

		EnumFonts();
	}
}

void CChooseFont::EnumFonts()
{
	// Store the currently selected font
	TCHAR origFont[256];
	GetDlgItemText(hWnd, IDC_FONT, origFont, 256);

	// Clear the content
	SendDlgItemMessage(hWnd, IDC_FONT, CB_RESETCONTENT, 0, 0);

	// Add the font file before enumerating
	TCHAR fontFile[256];
	GetDlgItemText(hWnd, IDC_FONTFILE, fontFile, 256);
	if( fontFile[0] != 0 && AddFontResourceEx(fontFile, FR_PRIVATE, 0) == 0 )
	{
		string tmp;
		ConvertTCharToUtf8(fontFile, tmp);
		string msg = "Failed to add font file: \n\n\"";
		msg += tmp;
		msg += "\"\n\nThe file is probably not a Windows font. Try another file.";

		TCHAR message[1024];
		ConvertUtf8ToTChar(msg, message, 1024);
		MessageBox(hWnd, message, __TEXT("Invalid font file"), MB_OK);
	}

	// Enumerate all fonts and types
	HDC dc;
	dc = GetDC(0);

	LOGFONT lf;
	lf.lfCharSet = DEFAULT_CHARSET; 
	lf.lfFaceName[0] = __TEXT('\0');
	lf.lfPitchAndFamily = 0;

	EnumFontFamiliesEx(dc, &lf, (FONTENUMPROCW)ChooseFontCallback, (LPARAM)this, 0);
	ReleaseDC(0, dc);

	// Remove the font file again
	RemoveFontResourceEx(fontFile, FR_PRIVATE, 0);

	// Reselect the previously selected font
	SendDlgItemMessage(hWnd, IDC_FONT, CB_SELECTSTRING, -1, (LPARAM)origFont);
}

void CChooseFont::OnInit()
{
	string str;

	TCHAR buf[1024];
	ConvertUtf8ToTChar(fontFile, buf, 1024);

	SetDlgItemText(hWnd, IDC_FONTFILE, buf);

	EnumFonts();

	ConvertUtf8ToTChar(font, buf, 1024);
	SendDlgItemMessage(hWnd, IDC_FONT, CB_SELECTSTRING, -1, (LPARAM)buf);

	OnFontChange();

	str = GetCharSetName(charSet);
	ConvertUtf8ToTChar(str, buf, 1024);
	SendDlgItemMessage(hWnd, IDC_CHARSET, CB_SELECTSTRING, -1, (LPARAM)buf);

	if( fontSize < 0 )
	{
		SetDlgItemInt(hWnd, IDC_FONTSIZE, -fontSize, FALSE);
		CheckDlgButton(hWnd, IDC_MATCHCHARHEIGHT, BST_CHECKED);
	}
	else
	{
		SetDlgItemInt(hWnd, IDC_FONTSIZE, fontSize, FALSE);
	}

	SendDlgItemMessage(hWnd, IDC_SIZESPIN, UDM_SETRANGE, 0, (LPARAM)MAKELONG(255, 1));
	SendDlgItemMessage(hWnd, IDC_SPINSAMPLING, UDM_SETRANGE, 0, (LPARAM)MAKELONG(4, 2)); 
	SendDlgItemMessage(hWnd, IDC_SCALEH_SPIN, UDM_SETRANGE, 0, (LPARAM)MAKELONG(200, 50)); 

	CheckDlgButton(hWnd, IDC_BOLD, isBold ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hWnd, IDC_ITALIC, isItalic ? BST_CHECKED : BST_UNCHECKED);

	CheckDlgButton(hWnd, IDC_SMOOTH, useSmoothing ? BST_CHECKED : BST_UNCHECKED);

	CheckDlgButton(hWnd, IDC_INVALIDCHAR, outputInvalidCharGlyph ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hWnd, IDC_NOKERN, dontIncludeKerningPairs ? BST_CHECKED : BST_UNCHECKED);

	CheckDlgButton(hWnd, IDC_USEUNICODE, useUnicode ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hWnd, IDC_USEOEM, !useUnicode ? BST_CHECKED : BST_UNCHECKED);

	SetDlgItemInt(hWnd, IDC_ANTIALIASING, antiAliasing == 1 ? 2 : antiAliasing, FALSE);
	CheckDlgButton(hWnd, IDC_ENABLEAA, antiAliasing > 1 ? BST_CHECKED : BST_UNCHECKED);

	SetDlgItemInt(hWnd, IDC_SCALEH, scaleH, FALSE);


	CheckDlgButton(hWnd, IDC_RENDERFROMOUTLINE, renderFromOutline ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hWnd, IDC_HINTING, useHinting ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hWnd, IDC_CLEARTYPE, useClearType ? BST_CHECKED : BST_UNCHECKED);


	// Outline options
	SetDlgItemInt(hWnd, IDC_OUTLINETHICKNESS, outlineThickness, FALSE);

	SendDlgItemMessage(hWnd, IDC_SPINTHICKNESS, UDM_SETRANGE, 0, (LPARAM)MAKELONG(32, 0)); 

	EnableWidgets();
}

int CALLBACK ChooseFontCallback(
  ENUMLOGFONTEX *lpelfe,     // logical-font data
  NEWTEXTMETRICEX *lpntme,   // physical-font data
  DWORD FontType,            // type of font
  LPARAM lParam              // application-defined data
)
{
	CDialog *dlg = (CChooseFont *)lParam;

	// Skip the rotated fonts, i.e. the ones prefixed with @
	if( lpelfe->elfLogFont.lfFaceName[0] == __TEXT("@")[0] )
		return 1;

	// Add font name to combobox
	int idx = (int)SendDlgItemMessage(dlg->GetHandle(), IDC_FONT, CB_FINDSTRINGEXACT, 0, (LPARAM)lpelfe->elfLogFont.lfFaceName);
	if( idx == CB_ERR )
		SendDlgItemMessage(dlg->GetHandle(), IDC_FONT, CB_ADDSTRING, 0, (LPARAM)lpelfe->elfLogFont.lfFaceName);

	return 1;
}

int CALLBACK ChooseFontCallback2(
  ENUMLOGFONTEX *lpelfe,
  NEWTEXTMETRICEX *lpntme,
  DWORD FontType,
  LPARAM lParam
)
{
	CChooseFont *dlg = (CChooseFont *)lParam;

	string str = GetCharSetName(lpelfe->elfLogFont.lfCharSet);

	// Add charset to combobox
	TCHAR buf[256];
	ConvertUtf8ToTChar(str, buf, 256);
	int idx = (int)SendDlgItemMessage(dlg->GetHandle(), IDC_CHARSET, CB_FINDSTRINGEXACT, 0, (LPARAM)buf);
	if( idx == CB_ERR )
		SendDlgItemMessage(dlg->GetHandle(), IDC_CHARSET, CB_ADDSTRING, 0, (LPARAM)buf);

	return 1;
}

void CChooseFont::GetOptions()
{
	TCHAR buf[256];

	GetDlgItemText(hWnd, IDC_FONT, buf, 256);
	ConvertTCharToUtf8(buf, font);

	GetDlgItemText(hWnd, IDC_FONTFILE, buf, 256);
	ConvertTCharToUtf8(buf, fontFile);

	GetDlgItemText(hWnd, IDC_CHARSET, buf, 256);
	string charsetName;
	ConvertTCharToUtf8(buf, charsetName);
	charSet = GetCharSet(charsetName.c_str());

	fontSize = GetDlgItemInt(hWnd, IDC_FONTSIZE, 0, FALSE);
	if( IsDlgButtonChecked(hWnd, IDC_MATCHCHARHEIGHT) == BST_CHECKED )
		fontSize = -fontSize;

	if( IsDlgButtonChecked(hWnd, IDC_ENABLEAA) == BST_CHECKED )
	{
		antiAliasing = GetDlgItemInt(hWnd, IDC_ANTIALIASING, 0, FALSE);
		if( antiAliasing < 2 ) antiAliasing = 2;
		if( antiAliasing > 4 ) antiAliasing = 4;
	}
	else
		antiAliasing = 1;

	scaleH = GetDlgItemInt(hWnd, IDC_SCALEH, 0, FALSE);
	if( scaleH < 1 ) scaleH = 1;

	isBold = IsDlgButtonChecked(hWnd, IDC_BOLD) == BST_CHECKED;
	isItalic = IsDlgButtonChecked(hWnd, IDC_ITALIC) == BST_CHECKED;

	useSmoothing = IsDlgButtonChecked(hWnd, IDC_SMOOTH) == BST_CHECKED;
	
	renderFromOutline = IsDlgButtonChecked(hWnd, IDC_RENDERFROMOUTLINE) == BST_CHECKED;
	useHinting = IsDlgButtonChecked(hWnd, IDC_HINTING) == BST_CHECKED;
	useClearType = IsDlgButtonChecked(hWnd, IDC_CLEARTYPE) == BST_CHECKED;

	outputInvalidCharGlyph = IsDlgButtonChecked(hWnd, IDC_INVALIDCHAR) == BST_CHECKED;
	dontIncludeKerningPairs = IsDlgButtonChecked(hWnd, IDC_NOKERN) == BST_CHECKED;

	if( IsDlgButtonChecked(hWnd, IDC_USEUNICODE) ) useUnicode = true; else useUnicode = false;

	outlineThickness = GetDlgItemInt(hWnd, IDC_OUTLINETHICKNESS, 0, FALSE);
}






