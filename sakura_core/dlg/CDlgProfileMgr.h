/*!	@file
	@brief プロファイルマネージャ

	@author Moca
	@date 2013.12.31
*/
/*
	Copyright (C) 2013, Moca
	Copyright (C) 2018-2021, Sakura Editor Organization

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose, 
	including commercial applications, and to alter it and redistribute it 
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such, 
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
#ifndef SAKURA_CDLGPROFILEMGR_E77A329C_4D06_436A_84E3_01B4D8F34A9A_H_
#define SAKURA_CDLGPROFILEMGR_E77A329C_4D06_436A_84E3_01B4D8F34A9A_H_
#pragma once

#include "env/CProfileManager.h"

#include "dlg/CDialog.h"
#include "_main/CCommandLine.h"
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

/*!
	@brief プロファイルマネージャダイアログ
 */
class CDlgProfileMgr final : public CDialog
{
	CProfileManager manager_;
	int currentIndex_;
	bool startAfterClose;

public:
	//! コマンドラインだけでプロファイルが確定するか調べる
	static bool TrySelectProfile( CCommandLine* pcCommandLine ) noexcept;

	CDlgProfileMgr();
	virtual ~CDlgProfileMgr() = default;

	int		DoModal(HINSTANCE hInstance, HWND hwndParent, LPARAM lParam);	/* モーダルダイアログの表示 */

	std::wstring m_strProfileName;

protected:
	int		GetData() override;
	void	SetData() /*const*/ override;

	BOOL	OnInitDialog(HWND hwndDlg, WPARAM wParam, LPARAM lParam) override;
	BOOL	OnBnClicked(int wID) override;
	BOOL	OnLbnSelChange(HWND hwndCtl, int wID) override;

	void	CreateProf();
	void	DeleteProf();
	void	RenameProf();
	void	SetDefaultProf();

	LPVOID	GetHelpIdTable(void) override;
};

#endif /* SAKURA_CDLGPROFILEMGR_E77A329C_4D06_436A_84E3_01B4D8F34A9A_H_ */
