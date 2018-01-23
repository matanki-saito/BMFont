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
#include "exportdlg.h"
#include "resource.h"
#include "commctrl.h"

using namespace acWindow;

struct SPresets
{
	const char *name;
	int  alpha;
	int  red;
	int  green;
	int  blue;
	bool invA;
	bool invR;
	bool invG;
	bool invB;
};

// TODO: Use EChnlValues from fontgen.h
enum EChnl
{
	GLYPH         = 0,
	OUTLINE       = 1,
	GLYPH_OUTLINE = 2,
    ZERO          = 3,
	ONE           = 4
};

const SPresets presets[] =
{
	{"Custom"                               , 0,0,0,0,0,0,0,0},
	{"White text with alpha"                , 0,4,4,4,0,0,0,0},
	{"Black text with alpha"                , 0,3,3,3,0,0,0,0},
	{"White text on black (no alpha)"       , 4,0,0,0,0,0,0,0},
	{"Black text on white (no alpha)"       , 4,0,0,0,0,1,1,1},
	{"Outlined text with alpha"             , 1,0,0,0,0,0,0,0},
	{"Pack text and outline in same channel", 2,2,2,2,0,0,0,0}
};

CExportDlg::CExportDlg() : CDialog()
{

}

int CExportDlg::DoModal(CWindow *parent)
{
	return CDialog::DoModal(IDD_EXPORT, parent);
}

LRESULT CExportDlg::MsgProc(UINT msg, WPARAM wParam, LPARAM lParam)
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

		case IDC_TEXTURE_FMT:
			if( HIWORD(wParam) == CBN_SELCHANGE )
				OnTextureChange();
			break;

		case IDC_ALPHA:
		case IDC_RED:
		case IDC_GREEN:
		case IDC_BLUE:
			if( HIWORD(wParam) == CBN_SELCHANGE )
				SendDlgItemMessage(hWnd, IDC_PRESETS, CB_SETCURSEL, 0, 0);
			break;

		case IDC_INV_A:
		case IDC_INV_R:
		case IDC_INV_G:
		case IDC_INV_B:
			SendDlgItemMessage(hWnd, IDC_PRESETS, CB_SETCURSEL, 0, 0);
			break;

		case IDC_PRESETS:
			if( HIWORD(wParam) == CBN_SELCHANGE )
				OnPresetChange();
			break;

		case IDC_BIT8:
		case IDC_BIT32:
		case IDC_4CHNLPACK:
		case IDC_FORCEZERO:
			EnableWidgets();
			break;
		}
		break;
	}

	return DefWndProc(msg, wParam, lParam);
}

void CExportDlg::EnableWidgets()
{
	// If force zero is set, then fixed height is implicit
	if( IsDlgButtonChecked(hWnd, IDC_FORCEZERO) )
		EnableWindow(GetDlgItem(hWnd, IDC_FIXEDHEIGHT), FALSE);
	else
		EnableWindow(GetDlgItem(hWnd, IDC_FIXEDHEIGHT), TRUE);

	// Pack chars in multiple channels
	if( IsDlgButtonChecked(hWnd, IDC_BIT8) )
        EnableWindow(GetDlgItem(hWnd, IDC_4CHNLPACK), FALSE);
	else 
		EnableWindow(GetDlgItem(hWnd, IDC_4CHNLPACK), TRUE);

	if( IsDlgButtonChecked(hWnd, IDC_BIT8) || IsDlgButtonChecked(hWnd, IDC_4CHNLPACK) )
	{
		EnableWindow(GetDlgItem(hWnd, IDC_ALPHA), TRUE);
		EnableWindow(GetDlgItem(hWnd, IDC_RED),   FALSE);
		EnableWindow(GetDlgItem(hWnd, IDC_GREEN), FALSE);
		EnableWindow(GetDlgItem(hWnd, IDC_BLUE),  FALSE);
		EnableWindow(GetDlgItem(hWnd, IDC_INV_A), TRUE);
		EnableWindow(GetDlgItem(hWnd, IDC_INV_R), FALSE);
		EnableWindow(GetDlgItem(hWnd, IDC_INV_G), FALSE);
		EnableWindow(GetDlgItem(hWnd, IDC_INV_B), FALSE);
	}
	else
	{
		EnableWindow(GetDlgItem(hWnd, IDC_ALPHA), TRUE);
		EnableWindow(GetDlgItem(hWnd, IDC_RED),   TRUE);
		EnableWindow(GetDlgItem(hWnd, IDC_GREEN), TRUE);
		EnableWindow(GetDlgItem(hWnd, IDC_BLUE),  TRUE);
		EnableWindow(GetDlgItem(hWnd, IDC_INV_A), TRUE);
		EnableWindow(GetDlgItem(hWnd, IDC_INV_R), TRUE);
		EnableWindow(GetDlgItem(hWnd, IDC_INV_G), TRUE);
		EnableWindow(GetDlgItem(hWnd, IDC_INV_B), TRUE);
	}
}

void CExportDlg::OnInit()
{
	TCHAR buf[256];

	// Character layout
	SetDlgItemInt(hWnd, IDC_SPACE_UP, paddingUp, FALSE);
	SetDlgItemInt(hWnd, IDC_SPACE_DOWN, paddingDown, FALSE);
	SetDlgItemInt(hWnd, IDC_SPACE_RIGHT, paddingRight, FALSE);
	SetDlgItemInt(hWnd, IDC_SPACE_LEFT, paddingLeft, FALSE);

	SetDlgItemInt(hWnd, IDC_SPACE_HORIZ, spacingHoriz, FALSE);
	SetDlgItemInt(hWnd, IDC_SPACE_VERT, spacingVert, FALSE);

	CheckDlgButton(hWnd, IDC_FIXEDHEIGHT, fixedHeight ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hWnd, IDC_FORCEZERO, forceZero ? BST_CHECKED : BST_UNCHECKED);

	// Output texture
	SetDlgItemInt(hWnd, IDC_WIDTH, width, FALSE);
	SetDlgItemInt(hWnd, IDC_HEIGHT, height, FALSE);

	if( bitDepth == 32 )
	{
		CheckDlgButton(hWnd, IDC_BIT32, BST_CHECKED);
		CheckDlgButton(hWnd, IDC_BIT8, BST_UNCHECKED);
	}
	else
	{
		CheckDlgButton(hWnd, IDC_BIT8, BST_CHECKED);
		CheckDlgButton(hWnd, IDC_BIT32, BST_UNCHECKED);
	}

	CheckDlgButton(hWnd, IDC_4CHNLPACK, fourChnlPacked ? BST_CHECKED : BST_UNCHECKED);

	// Fill in the channel content combos
	SendDlgItemMessage(hWnd, IDC_ALPHA, CB_ADDSTRING, 0, (LPARAM)__TEXT("glyph"));
	SendDlgItemMessage(hWnd, IDC_ALPHA, CB_ADDSTRING, 0, (LPARAM)__TEXT("outline"));
	SendDlgItemMessage(hWnd, IDC_ALPHA, CB_ADDSTRING, 0, (LPARAM)__TEXT("encoded glyph & outline"));
	SendDlgItemMessage(hWnd, IDC_ALPHA, CB_ADDSTRING, 0, (LPARAM)__TEXT("zero"));
	SendDlgItemMessage(hWnd, IDC_ALPHA, CB_ADDSTRING, 0, (LPARAM)__TEXT("one"));
	SendDlgItemMessage(hWnd, IDC_ALPHA, CB_SETCURSEL, alphaChnl, 0);

	SendDlgItemMessage(hWnd, IDC_RED, CB_ADDSTRING, 0, (LPARAM)__TEXT("glyph"));
	SendDlgItemMessage(hWnd, IDC_RED, CB_ADDSTRING, 0, (LPARAM)__TEXT("outline"));
	SendDlgItemMessage(hWnd, IDC_RED, CB_ADDSTRING, 0, (LPARAM)__TEXT("encoded glyph & outline"));
	SendDlgItemMessage(hWnd, IDC_RED, CB_ADDSTRING, 0, (LPARAM)__TEXT("zero"));
	SendDlgItemMessage(hWnd, IDC_RED, CB_ADDSTRING, 0, (LPARAM)__TEXT("one"));
	SendDlgItemMessage(hWnd, IDC_RED, CB_SETCURSEL, redChnl, 0);

	SendDlgItemMessage(hWnd, IDC_GREEN, CB_ADDSTRING, 0, (LPARAM)__TEXT("glyph"));
	SendDlgItemMessage(hWnd, IDC_GREEN, CB_ADDSTRING, 0, (LPARAM)__TEXT("outline"));
	SendDlgItemMessage(hWnd, IDC_GREEN, CB_ADDSTRING, 0, (LPARAM)__TEXT("encoded glyph & outline"));
	SendDlgItemMessage(hWnd, IDC_GREEN, CB_ADDSTRING, 0, (LPARAM)__TEXT("zero"));
	SendDlgItemMessage(hWnd, IDC_GREEN, CB_ADDSTRING, 0, (LPARAM)__TEXT("one"));
	SendDlgItemMessage(hWnd, IDC_GREEN, CB_SETCURSEL, greenChnl, 0);

	SendDlgItemMessage(hWnd, IDC_BLUE, CB_ADDSTRING, 0, (LPARAM)__TEXT("glyph"));
	SendDlgItemMessage(hWnd, IDC_BLUE, CB_ADDSTRING, 0, (LPARAM)__TEXT("outline"));
	SendDlgItemMessage(hWnd, IDC_BLUE, CB_ADDSTRING, 0, (LPARAM)__TEXT("encoded glyph & outline"));
	SendDlgItemMessage(hWnd, IDC_BLUE, CB_ADDSTRING, 0, (LPARAM)__TEXT("zero"));
	SendDlgItemMessage(hWnd, IDC_BLUE, CB_ADDSTRING, 0, (LPARAM)__TEXT("one"));
	SendDlgItemMessage(hWnd, IDC_BLUE, CB_SETCURSEL, blueChnl, 0);

	CheckDlgButton(hWnd, IDC_INV_A, invA ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hWnd, IDC_INV_R, invR ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hWnd, IDC_INV_G, invG ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hWnd, IDC_INV_B, invB ? BST_CHECKED : BST_UNCHECKED);

	// Font descriptor format
	CheckDlgButton(hWnd, IDC_DESC_TEXT, fontDescFormat == 0 ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hWnd, IDC_DESC_XML,  fontDescFormat == 1 ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hWnd, IDC_DESC_BIN,  fontDescFormat == 2 ? BST_CHECKED : BST_UNCHECKED);

	// Fill in the texture file format combo
	SendDlgItemMessage(hWnd, IDC_TEXTURE_FMT, CB_ADDSTRING, 0, (LPARAM)__TEXT("dds - DirectDraw Surface"));
	SendDlgItemMessage(hWnd, IDC_TEXTURE_FMT, CB_ADDSTRING, 0, (LPARAM)__TEXT("png - Portable Network Graphics"));
	SendDlgItemMessage(hWnd, IDC_TEXTURE_FMT, CB_ADDSTRING, 0, (LPARAM)__TEXT("tga - Targa"));
	ConvertUtf8ToTChar(textureFormat, buf, 256);
	SendDlgItemMessage(hWnd, IDC_TEXTURE_FMT, CB_SELECTSTRING, -1, (LPARAM)buf);
	OnTextureChange();
	SendDlgItemMessage(hWnd, IDC_TEXTURE_COMPRESSION, CB_SETCURSEL, textureCompression, 0);

	// Add presets
	int numPresets = sizeof(presets)/sizeof(SPresets);
	for( int n = 0; n < numPresets; n++ )
	{
		ConvertUtf8ToTChar(presets[n].name, buf, 256);
		SendDlgItemMessage(hWnd, IDC_PRESETS, CB_ADDSTRING, 0, (LPARAM)buf);
	}

	// TODO: Set the preset that matches the choices
	SendDlgItemMessage(hWnd, IDC_PRESETS, CB_SETCURSEL, 0, 0);

	EnableWidgets();
}

void CExportDlg::OnPresetChange()
{
	int idx = (int)SendDlgItemMessage(hWnd, IDC_PRESETS, CB_GETCURSEL, 0, 0);
	if( idx == 0 ) // Custom 
		return;

	SendDlgItemMessage(hWnd, IDC_ALPHA, CB_SETCURSEL, presets[idx].alpha, 0);
	SendDlgItemMessage(hWnd, IDC_RED, CB_SETCURSEL, presets[idx].red, 0);
	SendDlgItemMessage(hWnd, IDC_GREEN, CB_SETCURSEL, presets[idx].green, 0);
	SendDlgItemMessage(hWnd, IDC_BLUE, CB_SETCURSEL, presets[idx].blue, 0);

	CheckDlgButton(hWnd, IDC_INV_A, presets[idx].invA ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hWnd, IDC_INV_R, presets[idx].invR ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hWnd, IDC_INV_G, presets[idx].invG ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hWnd, IDC_INV_B, presets[idx].invB ? BST_CHECKED : BST_UNCHECKED);
}

void CExportDlg::OnTextureChange()
{
	// Clear the compression combo
	SendDlgItemMessage(hWnd, IDC_TEXTURE_COMPRESSION, CB_RESETCONTENT, 0, 0);

	// Which texture format is chosen?
	int idx = (int)SendDlgItemMessage(hWnd, IDC_TEXTURE_FMT, CB_GETCURSEL, 0, 0);
	if( idx != CB_ERR )
	{
		TCHAR buf[256];
		SendDlgItemMessage(hWnd, IDC_TEXTURE_FMT, CB_GETLBTEXT, idx, (LPARAM)buf);
		ConvertTCharToUtf8(buf, textureFormat);
		textureFormat.resize(3);
	}

	// Add the compression options
	if( textureFormat == "tga" )
	{
		SendDlgItemMessage(hWnd, IDC_TEXTURE_COMPRESSION, CB_ADDSTRING, 0, (LPARAM)__TEXT("None"));
		SendDlgItemMessage(hWnd, IDC_TEXTURE_COMPRESSION, CB_ADDSTRING, 0, (LPARAM)__TEXT("Run-length encoded"));
	}
	else if( textureFormat == "png" )
	{
		SendDlgItemMessage(hWnd, IDC_TEXTURE_COMPRESSION, CB_ADDSTRING, 0, (LPARAM)__TEXT("Deflate"));
	}
	else if( textureFormat == "dds" )
	{
		SendDlgItemMessage(hWnd, IDC_TEXTURE_COMPRESSION, CB_ADDSTRING, 0, (LPARAM)__TEXT("None"));
		SendDlgItemMessage(hWnd, IDC_TEXTURE_COMPRESSION, CB_ADDSTRING, 0, (LPARAM)__TEXT("DXT1"));
		SendDlgItemMessage(hWnd, IDC_TEXTURE_COMPRESSION, CB_ADDSTRING, 0, (LPARAM)__TEXT("DXT3"));
		SendDlgItemMessage(hWnd, IDC_TEXTURE_COMPRESSION, CB_ADDSTRING, 0, (LPARAM)__TEXT("DXT5"));
	}

	SendDlgItemMessage(hWnd, IDC_TEXTURE_COMPRESSION, CB_SETCURSEL, 0, 0);
}

void CExportDlg::GetOptions()
{
	// Character layout
	paddingUp = GetDlgItemInt(hWnd, IDC_SPACE_UP, 0, FALSE);
	paddingDown = GetDlgItemInt(hWnd, IDC_SPACE_DOWN, 0, FALSE);
	paddingRight = GetDlgItemInt(hWnd, IDC_SPACE_RIGHT, 0, FALSE);
	paddingLeft = GetDlgItemInt(hWnd, IDC_SPACE_LEFT, 0, FALSE);

	spacingHoriz = GetDlgItemInt(hWnd, IDC_SPACE_HORIZ, 0, FALSE);
	spacingVert = GetDlgItemInt(hWnd, IDC_SPACE_VERT, 0, FALSE);

	fixedHeight = IsDlgButtonChecked(hWnd, IDC_FIXEDHEIGHT) ? true : false;
	forceZero = IsDlgButtonChecked(hWnd, IDC_FORCEZERO) ? true : false;

	// Output texture
	width = GetDlgItemInt(hWnd, IDC_WIDTH, 0, FALSE);
	height = GetDlgItemInt(hWnd, IDC_HEIGHT, 0, FALSE);

	if( IsDlgButtonChecked(hWnd, IDC_BIT8) ) 
		bitDepth = 8; 
	else 
		bitDepth = 32;
	fourChnlPacked = IsDlgButtonChecked(hWnd, IDC_4CHNLPACK) ? true : false;

	if( IsDlgButtonChecked(hWnd, IDC_DESC_TEXT) ) fontDescFormat = 0;
	if( IsDlgButtonChecked(hWnd, IDC_DESC_XML)  ) fontDescFormat = 1;
	if( IsDlgButtonChecked(hWnd, IDC_DESC_BIN)  ) fontDescFormat = 2;

	// Get the file extension from combo box
	TCHAR buf[256];
	GetDlgItemText(hWnd, IDC_TEXTURE_FMT, buf, 256);
	ConvertTCharToUtf8(buf, textureFormat);
	textureFormat.resize(3);

	textureCompression = (int)SendDlgItemMessage(hWnd, IDC_TEXTURE_COMPRESSION, CB_GETCURSEL, 0, 0);

	alphaChnl = (int)SendDlgItemMessage(hWnd, IDC_ALPHA, CB_GETCURSEL, 0, 0);
	redChnl   = (int)SendDlgItemMessage(hWnd, IDC_RED,   CB_GETCURSEL, 0, 0);
	greenChnl = (int)SendDlgItemMessage(hWnd, IDC_GREEN, CB_GETCURSEL, 0, 0);
	blueChnl  = (int)SendDlgItemMessage(hWnd, IDC_BLUE,  CB_GETCURSEL, 0, 0);

	invA = IsDlgButtonChecked(hWnd, IDC_INV_A) ? true : false;
	invR = IsDlgButtonChecked(hWnd, IDC_INV_R) ? true : false;
	invG = IsDlgButtonChecked(hWnd, IDC_INV_G) ? true : false;
	invB = IsDlgButtonChecked(hWnd, IDC_INV_B) ? true : false;
}






