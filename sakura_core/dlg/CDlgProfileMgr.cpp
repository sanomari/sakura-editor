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
#include "StdAfx.h"
#include "dlg/CDlgProfileMgr.h"
#include "dlg/CDlgInput1.h"
#include "CDataProfile.h"
#include "util/file.h"
#include "util/shell.h"
#include "util/window.h"
#include "sakura_rc.h"
#include "sakura.hh"

const DWORD p_helpids[] = {
	IDC_LIST_PROFILE,				HIDC_LIST_PROFILE,				//プロファイル一覧
	IDC_CHECK_PROF_DEFSTART,		HIDC_CHECK_PROF_DEFSTART,		//デフォルト設定にして起動
	IDOK,							HIDOK_PROFILEMGR,				//起動
	IDCANCEL,						HIDCANCEL_PROFILEMGR,			//キャンセル
	IDC_BUTTON_HELP,				HIDC_PROFILEMGR_BUTTON_HELP,	//ヘルプ
	IDC_BUTTON_PROF_CREATE,			HIDC_BUTTON_PROF_CREATE,		//新規作成
	IDC_BUTTON_PROF_RENAME,			HIDC_BUTTON_PROF_RENAME,		//名前変更
	IDC_BUTTON_PROF_DELETE,			HIDC_BUTTON_PROF_DELETE,		//削除
	IDC_BUTTON_PROF_DEFSET,			HIDC_BUTTON_PROF_DEFSET,		//デフォルト設定
	IDC_BUTTON_PROF_DEFCLEAR,		HIDC_BUTTON_PROF_DEFCLEAR,		//デフォルト解除
	0, 0
};

std::wstring GetInputText(
	HWND hWnd,
	const std::wstring& strTitle,
	const std::wstring& strMessage,
	const size_t cchMaxText
)
{
	std::wstring strText(cchMaxText + 1, L'\0');
	CDlgInput1 cDlgInput1;
	cDlgInput1.DoModal(nullptr, hWnd, strTitle.c_str(), strMessage.c_str(), static_cast<int>(cchMaxText), strText.data());
	return strText.data();
}

//! コマンドラインだけでプロファイルが確定するか調べる
bool CDlgProfileMgr::TrySelectProfile( CCommandLine* pcCommandLine ) noexcept
{
	CProfileManager manager;
	return manager.TrySelectProfile(pcCommandLine);
}

CDlgProfileMgr::CDlgProfileMgr()
	: CDialog(false, false)
	, manager_()
	, currentIndex_(LB_ERR)
	, startAfterClose(false)
{
}

/*! モーダルダイアログの表示 */
int CDlgProfileMgr::DoModal( HINSTANCE, HWND hwndParent, LPARAM lParam )
{
	if (const auto dllPath = manager_.GetLanguageDll(); !dllPath.empty()) {
		CSelectLang::ChangeLang(dllPath.wstring().data());
	}

	int ret = CDialog::DoModal( nullptr, hwndParent, IDD_PROFILEMGR, lParam );

	m_strProfileName = manager_.GetAt(currentIndex_);

	return ret;
}

/*!
	@breaf ダイアログデータの取得

	@retval > 0  正常
	@retval == 0 準正常(操作対象なし)
	@retval < 0 入力エラー
 */
int CDlgProfileMgr::GetData()
{
	currentIndex_ = List_GetCurSel(GetItemHwnd(IDC_LIST_PROFILE));
	startAfterClose = IsDlgButtonCheckedBool(GetHwnd(), IDC_CHECK_PROF_DEFSTART);
	return 1;
}

/*!
	@brief ダイアログデータの設定
 */
void CDlgProfileMgr::SetData() /*const*/
{
	HWND hwndList = GetItemHwnd(IDC_LIST_PROFILE);

	List_ResetContent(hwndList);

	const int defaultIndex = manager_.GetDefault().has_value() ? manager_.GetDefault().value() : LB_ERR;

	CTextWidthCalc calc(hwndList);
	calc.SetDefaultExtend(CTextWidthCalc::WIDTH_MARGIN_SCROLLBER);
	const int count = manager_.GetSize();
	for (int i = 0; i < count; i++) {
		auto str = manager_.GetAt(i);
		if (i == defaultIndex) {
			str += L"*";
		}
		List_AddString(hwndList, str.c_str());
		calc.SetTextWidthIfMax(str.c_str());
	}
	List_SetHorizontalExtent(hwndList, calc.GetCx());

	List_SetCurSel(hwndList, currentIndex_);

	DlgItem_Enable(GetHwnd(), IDC_BUTTON_PROF_DELETE, currentIndex_ != 0);
	DlgItem_Enable(GetHwnd(), IDC_BUTTON_PROF_RENAME, currentIndex_ != 0);
	DlgItem_Enable(GetHwnd(), IDC_BUTTON_PROF_DEFCLEAR, defaultIndex != LB_ERR);

	CheckDlgButtonBool(GetHwnd(), IDC_CHECK_PROF_DEFSTART, startAfterClose);
}

BOOL CDlgProfileMgr::OnInitDialog(HWND hwndDlg, WPARAM wParam, LPARAM lParam)
{
	manager_.ReadSettings();

	return CDialog::OnInitDialog(hwndDlg, wParam, lParam);
}

BOOL CDlgProfileMgr::OnBnClicked( int wID )
{
	switch (wID) {
	case IDC_BUTTON_PROF_CREATE:
		CreateProf();
		return TRUE;

	case IDC_BUTTON_PROF_RENAME:
		RenameProf();
		return TRUE;

	case IDC_BUTTON_PROF_DELETE:
		DeleteProf();
		return TRUE;

	case IDC_BUTTON_PROF_DEFSET:
		SetDefaultProf();
		return TRUE;

	case IDC_BUTTON_PROF_DEFCLEAR:
		GetData();
		manager_.ClearDefault();
		SetData();
		return TRUE;

	case IDC_BUTTON_HELP:
		MyWinHelp(GetHwnd(), HELP_CONTEXT, ::FuncID_To_HelpContextID(F_PROFILEMGR));
		return TRUE;

	case IDOK:
		GetData();
		if (startAfterClose) {
			manager_.SetDefault(currentIndex_);
		}
		manager_.WriteSettings();
		break;
	}
	return CDialog::OnBnClicked(wID);
}

BOOL CDlgProfileMgr::OnLbnSelChange(HWND hwndCtl, int wID)
{
	if (wID == IDC_LIST_PROFILE) {
		GetData();
		SetData();
		return TRUE;
	}
	return CDialog::OnLbnSelChange(hwndCtl, wID);
}

void CDlgProfileMgr::CreateProf()
{
	GetData();
	std::wstring strText = GetInputText(
		GetHwnd(),
		LS(STR_DLGPROFILE_NEW_PROF_TITLE),
		LS(STR_DLGPROFILE_NEW_PROF_MSG),
		_MAX_PATH - GetProfileMgrFileName().wstring().length()
		);
	if (strText.empty()) {
		return;
	}
	try {
		manager_.Add(currentIndex_, strText);
		++currentIndex_;
		SetData();
	}
	catch (const _com_error& ce) {
		ErrorMessage(nullptr, ce.Description());
	}
}

void CDlgProfileMgr::DeleteProf()
{
	GetData();
	if (currentIndex_ < 1 || currentIndex_ >= manager_.GetSize()) {
		return;
	}
	try {
		manager_.Delete(currentIndex_);
		--currentIndex_;
		SetData();
	}
	catch (const _com_error& ce) {
		ErrorMessage(nullptr, ce.Description());
	}
}

void CDlgProfileMgr::RenameProf()
{
	GetData();
	if (currentIndex_ < 1 || currentIndex_ >= manager_.GetSize()) {
		return;
	}
	std::wstring strTextOld = manager_.GetAt(currentIndex_);
	std::wstring strText = GetInputText(
		GetHwnd(),
		LS(STR_DLGPROFILE_RENAME_TITLE),
		LS(STR_DLGPROFILE_RENAME_MSG),
		_MAX_PATH - GetProfileMgrFileName().wstring().length()
	);
	if (strText.empty()) {
		return;
	}
	try {
		manager_.Rename(currentIndex_, strText);
		SetData();
	}
	catch (const _com_error& ce) {
		ErrorMessage(nullptr, ce.Description());
	}
}

void CDlgProfileMgr::SetDefaultProf()
{
	GetData();
	if (currentIndex_ != LB_ERR && (currentIndex_ < 0 || currentIndex_ >= manager_.GetSize())) {
		return;
	}
	try {
		manager_.SetDefault(currentIndex_);
		SetData();
	}
	catch (const _com_error& ce) {
		ErrorMessage(nullptr, ce.Description());
	}
}

LPVOID CDlgProfileMgr::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
