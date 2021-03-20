/*! @file */
/*
	Copyright (C) 2018-2021 Sakura Editor Organization

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
#include "env/CProfileManager.h"

#include <stdexcept>
#include <string>
#include <string_view>

#include "CDataProfile.h"
#include "basis/CErrorInfo.h"
#include "util/file.h"

using namespace std::literals::string_literals;

//! コマンドラインだけでプロファイルが確定するか調べる
bool CProfileManager::TrySelectProfile( CCommandLine* pcCommandLine ) noexcept
{
	bool bSettingLoaded = ReadSettings();

	bool bDialog;
	if( pcCommandLine->IsProfileMgr() ){		// コマンドラインでプロファイルマネージャの表示が指定されている
		bDialog = true;
	}else if( pcCommandLine->IsSetProfile() ){	// コマンドラインでプロファイル名が指定されている
		bDialog = false;
	}else if( !bSettingLoaded ){				// プロファイル設定がなかった
		bDialog = false;
	}else if( 0 < m_nDefaultIndex && m_nDefaultIndex <= (int)m_vProfiles.size() ){
		// プロファイル設定のデフォルトインデックス値から該当のプロファイル名が指定されたものとして動作する
		pcCommandLine->SetProfileName( GetAt(m_nDefaultIndex).c_str() );
		bDialog = false;
	}else{
		// プロファイル設定のデフォルトインデックス値が不正なのでプロファイルマネージャを表示して設定更新を促す
		bDialog = true;
	}
	return !bDialog;
}

[[nodiscard]] bool CProfileManager::IsProfileNameValid(std::wstring_view name) const noexcept
{
	if (name == L"." || name == L"..") {
		return false;
	}

	constexpr const wchar_t reservedChars[] = L"\\/*?\"<>|\t&':";
	return ::wcscspn(name.data(), reservedChars) == name.length();
}

[[nodiscard]] bool CProfileManager::IsNewProfileName(std::wstring_view name) const noexcept
{
	const auto it = std::find_if(m_vProfiles.cbegin(), m_vProfiles.cend(), [name](const auto& profile) { return 0 == ::_wcsicmp(profile.data(), name.data()); });
	return name.length() > 0 && it == m_vProfiles.cend();
}

void CProfileManager::ValidateProfileName(std::wstring_view name) const
{
	// プロファイル名チェック
	if (!IsProfileNameValid(name)) {
		// L"利用できない文字が含まれています。"
		::_com_raise_error(E_FAIL, MakeMsgError(LS(STR_DLGPROFILE_ERR_INVALID_CHAR)));
	}

	// 重複チェック
	if (!IsNewProfileName(name)) {
		// L"すでに同じ名前のプロファイルがあります。"
		::_com_raise_error(E_FAIL, MakeMsgError(LS(STR_DLGPROFILE_ERR_ALREADY)));
	}

	// ファイル不存在チェック
	auto profileDirNew = GetProfileDirectory(name.data());
	if (std::filesystem::exists(profileDirNew)) {
		// L"同名のファイルがあるため作成できません。"
		::_com_raise_error(E_FAIL, MakeMsgError(LS(STR_DLGPROFILE_ERR_FILE)));
	}
}

void CProfileManager::Add(size_t index, std::wstring_view name)
{
	// プロファイル検証
	ValidateProfileName(name);

	if (index < 1 || m_vProfiles.size() < index) {
		index = m_vProfiles.size();
	}
	m_vProfiles.insert(m_vProfiles.begin() + index, name.data());
}

void CProfileManager::Rename(size_t index, std::wstring_view name)
{
	//旧プロファイル名取得
	if (index < 1 || m_vProfiles.size() <= index) {
			throw std::out_of_range("index is out of range");
	}
	const auto nameOld = GetAt(index);

	// 移動元プロファイル存在チェック
	const auto profileDirOld = GetProfileDirectory(nameOld);
	if (const auto s = std::filesystem::status(profileDirOld);
		!std::filesystem::exists(s) || s.type() != std::filesystem::file_type::directory) {
		Add(index, name);
		return;
	}

	// プロファイル検証
	ValidateProfileName(name);

	//プロファイル移動
	const auto profileDirNew = GetProfileDirectory(name.data());
	if (!::MoveFile(profileDirOld.c_str(), profileDirNew.c_str())) {
		::_com_raise_error(E_FAIL, MakeMsgError(LS(STR_DLGPROFILE_ERR_RENAME)));
	}
	m_vProfiles[index] = name;
}

void CProfileManager::Delete(size_t index)
{
	if (index < 1 || m_vProfiles.size() <= index) {
		throw std::out_of_range("index is out of range");
	}
	if (const size_t d = GetDefault().value_or(LB_ERR)) {
		if (d == index) {
			ClearDefault();
		}
		else if (1 < m_nDefaultIndex && index < d && d < m_vProfiles.size()) {
			SetDefault(static_cast<size_t>(--m_nDefaultIndex));
		}
	}
	m_vProfiles.erase(m_vProfiles.begin() + index);
}

void CProfileManager::SetDefault(size_t index)
{
	if (m_vProfiles.size() <= index) {
		throw std::out_of_range("index is out of range");
	}
	m_nDefaultIndex = static_cast<int>(index);
}

void CProfileManager::ClearDefault()
{
	m_nDefaultIndex = LB_ERR;
}

[[nodiscard]] int CProfileManager::GetSize() const noexcept
{
	return static_cast<int>(m_vProfiles.size());
}

[[nodiscard]] std::wstring CProfileManager::GetAt(size_t index) const
{
	return m_vProfiles[index];
}

[[nodiscard]] std::optional<int> CProfileManager::GetDefault() const noexcept
{
	if (m_nDefaultIndex < 0) {
		return std::nullopt;
	}
	return m_nDefaultIndex;
}

bool CProfileManager::ReadSettings()
{
	constexpr bool forRead = false;
	return IOProfSettings(forRead);
}

bool CProfileManager::WriteSettings() const
{
	constexpr bool forWrite = true;
	return const_cast<CProfileManager&>(*this).IOProfSettings(forWrite);
}

bool CProfileManager::IOProfSettings(bool bWrite)
{
	const auto profileMgrFilePath = GetProfileMgrFileName();

	CDataProfile profile;
	if (bWrite) {
		profile.SetWritingMode();
	}
	else {
		profile.SetReadingMode();
		if (!profile.ReadProfile(profileMgrFilePath.c_str())) {
			return false;
		}
		m_vProfiles = { L"(default)"s };
	}

	int nCount = (int)m_vProfiles.size() - 1;
	profile.IOProfileData(L"Profile"s, L"nCount", nCount);

	for (int i = 0; i < nCount; i++) {
		const std::wstring key = strprintf(L"P[%d]", i + 1); // 1開始
		std::wstring name;
		if (bWrite) {
			name = GetAt(i + 1);
		}
		profile.IOProfileData(L"Profile"s, key, name);
		if (const bool bRead = !bWrite;
			bRead &&
			IsNewProfileName(name) &&
			IsProfileNameValid(name))
		{
			m_vProfiles.push_back(name);
		}
	}

	profile.IOProfileData(L"Profile"s, L"nDefaultIndex"s, m_nDefaultIndex);
	if (m_nDefaultIndex < 0 || m_nDefaultIndex >= (int)m_vProfiles.size()) {
		m_nDefaultIndex = LB_ERR;
	}

	bool bDefaultSelected = m_nDefaultIndex != LB_ERR;
	profile.IOProfileData(L"Profile"s, L"bDefaultSelected"s, bDefaultSelected);
	if (!bDefaultSelected) {
		m_nDefaultIndex = LB_ERR;
	}

	profile.IOProfileData(L"Profile"s, L"szDllLanguage"s, m_szDllLanguage);

	if (bWrite) {
		return profile.WriteProfile(profileMgrFilePath.c_str(), L"Sakura Profile ini");
	}
	return true;
}

/*!
	@brief プロファイルマネージャ設定ファイルパスを取得する
 */
std::filesystem::path GetProfileMgrFileName()
{
	auto privateIniPath = GetIniFileName();
	if (const auto* pCommandLine = CCommandLine::getInstance(); pCommandLine->IsSetProfile() && *pCommandLine->GetProfileName()) {
		auto filename = privateIniPath.filename();
		privateIniPath = privateIniPath.parent_path().parent_path().append(filename.c_str());
	}
	return privateIniPath.replace_extension().concat(L"_prof.ini");
}

/*!
	指定したプロファイルの設定保存先ディレクトリを取得する
 */
std::filesystem::path GetProfileDirectory(const std::wstring& name)
{
	auto privateIniDir = GetIniFileName().parent_path();
	if (const auto* pCommandLine = CCommandLine::getInstance(); pCommandLine && pCommandLine->IsSetProfile() && *pCommandLine->GetProfileName()) {
		privateIniDir = privateIniDir.parent_path();
	}
	return privateIniDir.append(name);
}
