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

#ifndef EXPORTDLG_H
#define EXPORTDLG_H

#include <string>
using std::string;

#include "acwin_dialog.h"

class CExportDlg : virtual public acWindow::CDialog
{
public:
	CExportDlg();

	int DoModal(acWindow::CWindow *parent);

	int paddingUp;
	int paddingDown;
	int paddingRight;
	int paddingLeft;
	int spacingHoriz;
	int spacingVert;
	bool fixedHeight;
	bool forceZero;

	int width;
	int height;
	int bitDepth;
	bool fourChnlPacked;
	int alphaChnl;
	int redChnl;
	int greenChnl;
	int blueChnl;
	bool invA;
	bool invR;
	bool invG;
	bool invB;

	int fontDescFormat;

	string textureFormat;
	int textureCompression;

protected:
	void OnInit();
	void GetOptions();

	void OnTextureChange();
	void OnPresetChange();

	void EnableWidgets();

	LRESULT MsgProc(UINT msg, WPARAM wParam, LPARAM lParam);
};

#endif