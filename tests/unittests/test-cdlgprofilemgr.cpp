/*! @file */
/*
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
#include <gtest/gtest.h>

#ifndef NOMINMAX
#define NOMINMAX
#endif /* #ifndef NOMINMAX */

#include <tchar.h>
#include <Windows.h>

#include "dlg/CDlgProfileMgr.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <regex>
#include <string>

#include "config/maxdata.h"
#include "basis/primitive.h"
#include "debug/Debug2.h"
#include "basis/CMyString.h"
#include "mem/CNativeW.h"
#include "env/DLLSHAREDATA.h"
#include "_main/CCommandLine.h"
#include "_main/CControlProcess.h"
#include "CDataProfile.h"
#include "util/file.h"

#include "String_define.h"

/*!
 * プロファイルマネージャ設定ファイルを使うテストのためのフィクスチャクラス
 *
 * 設定ファイルを使うテストは「設定ファイルがない状態」からの始動を想定しているので
 * 始動前に設定ファイルを削除するようにしている。
 * テスト実行後に設定ファイルを残しておく意味はないので終了後も削除している。
 */
class CDlgProfileMgrTest : public ::testing::Test {
protected:
	/*!
	 * プロファイルマネージャ設定ファイルのパス
	 */
	std::filesystem::path profileMgrIniPath;

	/*!
	 * テストが起動される直前に毎回呼ばれる関数
	 */
	void SetUp() override {
		// コマンドラインのインスタンスを用意する
		CCommandLine cCommandLine;

		// プロファイルマネージャ設定ファイルのパスを生成
		profileMgrIniPath = GetProfileMgrFileName();
		if( fexist( profileMgrIniPath.c_str() ) ){
			// プロファイルマネージャー設定を削除する
			std::filesystem::remove( profileMgrIniPath );
		}
	}

	/*!
	 * テストが実行された直後に毎回呼ばれる関数
	 */
	void TearDown() override {
		// プロファイルマネージャー設定を削除する
		std::filesystem::remove( profileMgrIniPath );
	}
};

/*!
 * @brief TrySelectProfileのテスト
 */
TEST_F( CDlgProfileMgrTest, TrySelectProfile_001 )
{
	// プロファイルマネージャ表示オプションが付いてたらプロファイルは確定しない
	CCommandLine cCommandLine;
	cCommandLine.ParseCommandLine( L"-PROFMGR", false );
	ASSERT_FALSE( CDlgProfileMgr::TrySelectProfile( &cCommandLine ) );
}

/*!
 * @brief TrySelectProfileのテスト
 */
TEST_F( CDlgProfileMgrTest, TrySelectProfile_002 )
{
	// プロファイル名が指定されていたらプロファイルは確定する
	CCommandLine cCommandLine;
	cCommandLine.ParseCommandLine( L"-PROF=執筆用", false );
	ASSERT_TRUE( CDlgProfileMgr::TrySelectProfile( &cCommandLine ) );
}

/*!
 * @brief TrySelectProfileのテスト
 */
TEST_F( CDlgProfileMgrTest, TrySelectProfile_003 )
{
	// プロファイルマネージャー設定がなかったらプロファイルは確定する
	CCommandLine cCommandLine;
	ASSERT_TRUE( CDlgProfileMgr::TrySelectProfile( &cCommandLine ) );
}

/*!
 * @brief TrySelectProfileのテスト
 */
TEST_F( CDlgProfileMgrTest, TrySelectProfile_004 )
{
	// プロファイル設定を作る
	CDataProfile cProfile;
	cProfile.SetWritingMode();
	cProfile.SetProfileData(L"Profile", L"szDllLanguage", L"");
	cProfile.SetProfileData(L"Profile", L"nDefaultIndex", L"3");
	cProfile.SetProfileData(L"Profile", L"nCount", L"3");
	cProfile.SetProfileData(L"Profile", L"P[1]", L"保存用");
	cProfile.SetProfileData(L"Profile", L"P[2]", L"鑑賞用");
	cProfile.SetProfileData(L"Profile", L"P[3]", L"使用用");
	cProfile.SetProfileData(L"Profile", L"bDefaultSelect", L"1");
	cProfile.WriteProfile(profileMgrIniPath.c_str(), L"Sakura Profile ini");

	// プロファイルマネージャー設定にデフォルト定義があればプロファイルは確定する
	CCommandLine cCommandLine;
	ASSERT_TRUE( CDlgProfileMgr::TrySelectProfile( &cCommandLine ) );
}

/*!
 * @brief TrySelectProfileのテスト
 */
TEST_F( CDlgProfileMgrTest, TrySelectProfile_005 )
{
	// プロファイル設定を作る
	CDataProfile cProfile;
	cProfile.SetWritingMode();
	cProfile.SetProfileData(L"Profile", L"szDllLanguage", L"");
	cProfile.SetProfileData(L"Profile", L"nDefaultIndex", L"4");
	cProfile.SetProfileData(L"Profile", L"nCount", L"3");
	cProfile.SetProfileData(L"Profile", L"P[1]", L"保存用");
	cProfile.SetProfileData(L"Profile", L"P[2]", L"鑑賞用");
	cProfile.SetProfileData(L"Profile", L"P[3]", L"使用用");
	cProfile.SetProfileData(L"Profile", L"bDefaultSelect", L"1");
	cProfile.WriteProfile(profileMgrIniPath.c_str(), L"Sakura Profile ini");

	// プロファイルマネージャー設定のデフォルト定義がおかしればプロファイルは確定しない
	CCommandLine cCommandLine;
	ASSERT_FALSE( CDlgProfileMgr::TrySelectProfile( &cCommandLine ) );
}

/*!
 * @brief TrySelectProfileのテスト
 */
TEST_F( CDlgProfileMgrTest, TrySelectProfile_006 )
{
	// 空のプロファイル設定を作る
	CDataProfile cProfile;
	cProfile.SetWritingMode();
	cProfile.SetProfileData(L"Profile", L"szDllLanguage", L"");
	cProfile.SetProfileData(L"Profile", L"nDefaultIndex", L"-1");
	cProfile.SetProfileData(L"Profile", L"bDefaultSelect", L"0");
	cProfile.WriteProfile(profileMgrIniPath.c_str(), L"Sakura Profile ini");

	// プロファイルマネージャー設定が空定義ならプロファイルは確定しない
	CCommandLine cCommandLine;
	ASSERT_FALSE( CDlgProfileMgr::TrySelectProfile( &cCommandLine ) );
}

/*!
 * @brief 初期状態
 */
TEST(CProfileManager, ctor)
{
	CProfileManager manager;

	// 初期サイズは1
	ASSERT_EQ(1, manager.GetSize());

	// 初期プロファイル名は(default)
	ASSERT_STREQ(L"(default)", manager.GetAt(0).c_str());

	// 初期のデフォルトは未設定
	ASSERT_FALSE(manager.GetDefault().has_value());

	// 代替言語DLLのパスは未設定
	ASSERT_TRUE(manager.GetLanguageDll().empty());
}

/*!
 * @brief プロファイル名チェック
 */
TEST(CProfileManager, IsProfileNameValid)
{
	CProfileManager manager;

	//「カレントフォルダ」を意味する名前なのでダメ
	ASSERT_THROW({ manager.Add(0, L"."); }, _com_error);

	//「親フォルダ」を意味する名前なのでダメ
	ASSERT_THROW({ manager.Add(0, L".."); }, _com_error);

	//L"\\/*?\"<>|\t&':"のいずれかを含んでいたらダメ
	ASSERT_THROW({ manager.Add(0, L"bad name\\test"); }, _com_error);
	ASSERT_THROW({ manager.Add(0, L"bad name/test"); }, _com_error);
	ASSERT_THROW({ manager.Add(0, L"bad name*test"); }, _com_error);
	ASSERT_THROW({ manager.Add(0, L"bad name?test"); }, _com_error);
	ASSERT_THROW({ manager.Add(0, L"bad name\"test"); }, _com_error);
	ASSERT_THROW({ manager.Add(0, L"bad name<test"); }, _com_error);
	ASSERT_THROW({ manager.Add(0, L"bad name>test"); }, _com_error);
	ASSERT_THROW({ manager.Add(0, L"bad name|test"); }, _com_error);
	ASSERT_THROW({ manager.Add(0, L"bad name\ttest"); }, _com_error);
	ASSERT_THROW({ manager.Add(0, L"bad name&test"); }, _com_error);
	ASSERT_THROW({ manager.Add(0, L"bad name'test"); }, _com_error);
	ASSERT_THROW({ manager.Add(0, L"bad name:test"); }, _com_error);
}

/*!
 * @brief 新規プロファイル名の検証
 */
TEST(CProfileManager, ValidateProfileName_InvalidName)
{
	CProfileManager manager;
	try {
		//「親フォルダ」を意味する名前なのでダメ
		manager.Add(0, L"..");
	}
	catch (const _com_error& ce) {
		// L"利用できない文字が含まれています。"
		ASSERT_STREQ(LS(STR_DLGPROFILE_ERR_INVALID_CHAR), ce.Description());
	}
}

/*!
 * @brief プロファイル名重複チェック
 */
TEST(CProfileManager, IsNewProfileName)
{
	CProfileManager manager;

	//デフォルトを表す名前は予約済み
	ASSERT_THROW({ manager.Add(0, L"(default)"); }, _com_error);

	//空文字は重複扱い
	ASSERT_THROW({ manager.Add(0, L""); }, _com_error);

	//上記以外は、追加されてなければ重複なし
	ASSERT_TRUE(manager.IsNewProfileName(L"profile1"));

	//当然、追加した後は重複が検知される
	manager.Add(0, L"profile1");
	ASSERT_THROW({ manager.Add(0, L"profile1"); }, _com_error);
}

/*!
 * @brief 新規プロファイル名の検証
 */
TEST(CProfileManager, ValidateProfileName_Duplicated)
{
	CProfileManager manager;
	try {
		//空文字は重複扱い
		manager.Add(0, L"");
	}
	catch (const _com_error& ce) {
		// L"すでに同じ名前のプロファイルがあります。"
		ASSERT_STREQ(LS(STR_DLGPROFILE_ERR_ALREADY), ce.Description());
	}
}

/*!
 * @brief 新規プロファイル名の検証
 */
TEST(CProfileManager, ValidateProfileName_FileExists)
{
	// 追加するプロファイル名
	constexpr const wchar_t profileName[] = L"profile1";

	// 追加するプロファイル名でINIファイル作成
	auto existingPath = GetProfileDirectory(profileName);
	if (std::filesystem::exists(existingPath)) {
		std::filesystem::remove(existingPath);
	}
	ASSERT_FALSE(fexist(existingPath.c_str()));

	CProfile cProfile;
	cProfile.WriteProfile(existingPath.c_str(), L"test profile");
	ASSERT_TRUE(fexist(existingPath.c_str()));

	// プロファイルマネージャでプロファイル追加
	CProfileManager manager;

	try {
		// プロファイルフォルダと同名のファイルが存在していたらダメ
		manager.Add(0, profileName);
	}
	catch (const _com_error& ce) {
		// 結果を評価する前に作成したファイルを削除しておく
		std::filesystem::remove(existingPath);
		
		// L"同名のファイルがあるため作成できません。"
		ASSERT_STREQ(LS(STR_DLGPROFILE_ERR_FILE), ce.Description());
	}

	ASSERT_FALSE(fexist(existingPath.c_str()));
}

/*!
 * @brief プロファイル追加
 */
TEST(CProfileManager, Add)
{
	// 追加するプロファイル名
	constexpr const wchar_t profileName[] = L"profile1";

	// 新しいプロファイルを追加
	CProfileManager manager;
	manager.Add(0, profileName);

	// 追加後はサイズが増える
	ASSERT_EQ(2, manager.GetSize());

	// 追加したプロファイル名を取得できる
	ASSERT_STREQ(profileName, manager.GetAt(1).c_str());

	// 追加しただけではデフォルトは変化しない
	ASSERT_FALSE(manager.GetDefault().has_value());
}

/*!
 * @brief デフォルト設定
 */
TEST(CProfileManager, SetDefault)
{
	CProfileManager manager;

	// 初期のデフォルトは未設定
	ASSERT_FALSE(manager.GetDefault().has_value());

	// 1番目をデフォルトにする
	manager.SetDefault(0);
	ASSERT_TRUE(manager.GetDefault().has_value());
	ASSERT_EQ(0, manager.GetDefault().value());
	ASSERT_STREQ(L"(default)", manager.GetAt(manager.GetDefault().value()).c_str());

	// 範囲外はデフォルトにできない
	ASSERT_THROW({ manager.SetDefault(1); }, std::out_of_range);
}

/*!
 * @brief プロファイル削除
 */
TEST(CProfileManager, Delete)
{
	CProfileManager manager;

	// 既定プロファイルは削除できない
	ASSERT_THROW({ manager.Delete(0); }, std::out_of_range);

	// 範囲外は削除できない
	ASSERT_EQ(1, manager.GetSize());
	ASSERT_THROW({ manager.Delete(1); }, std::out_of_range);

	// 新しいプロファイルを追加
	manager.Add(0, L"保存用");
	manager.Add(2, L"観賞用");
	manager.Add(3, L"使用用");
	ASSERT_EQ(4, manager.GetSize());

	//4つ目(使用用)をデフォルトにする
	manager.SetDefault(3);
	ASSERT_TRUE(manager.GetDefault().has_value());
	ASSERT_EQ(3, manager.GetDefault().value());
	ASSERT_STREQ(L"使用用", manager.GetAt(manager.GetDefault().value()).c_str());

	//3つ目(観賞用)を破棄するとデフォルトはズレる
	manager.Delete(2);
	ASSERT_TRUE(manager.GetDefault().has_value());
	ASSERT_EQ(2, manager.GetDefault().value());
	ASSERT_STREQ(L"使用用", manager.GetAt(manager.GetDefault().value()).c_str());

	//3つ目(使用用)を破棄するとデフォルトは解除される
	manager.Delete(2);
	ASSERT_FALSE(manager.GetDefault().has_value());
}

/*!
 * @brief プロファイル名変更
 */
TEST(CProfileManager, Rename_OldProfileIsNotExist)
{
	// 変更前のプロファイル名
	constexpr const wchar_t profileNameOld[] = L"profile1";
	// 変更後のプロファイル名
	constexpr const wchar_t profileNameNew[] = L"profile2";

	// 変更前プロファイルのパスを取得
	auto existingPath = GetProfileDirectory(profileNameOld);
	ASSERT_FALSE(std::filesystem::exists(existingPath));

	// プロファイルマネージャでプロファイル追加
	CProfileManager manager;
	manager.Add(1, profileNameOld);
	ASSERT_STREQ(profileNameOld, manager.GetAt(1).c_str());

	ASSERT_FALSE(std::filesystem::exists(existingPath));

	// 内部的にAddが呼ばれる
	manager.Rename(1, profileNameNew);
	ASSERT_STREQ(profileNameNew, manager.GetAt(1).c_str());
}

/*!
 * @brief プロファイル名変更
 */
TEST(CProfileManager, Rename_OldProfileIsExistingFile)
{
	// 変更前のプロファイル名
	constexpr const wchar_t profileNameOld[] = L"profile1";
	// 変更後のプロファイル名
	constexpr const wchar_t profileNameNew[] = L"profile2";

	// プロファイルマネージャでプロファイル追加
	CProfileManager manager;
	manager.Add(1, profileNameOld);
	ASSERT_STREQ(profileNameOld, manager.GetAt(1).c_str());

	// 変更前プロファイルのパスを取得
	auto existingPath = GetProfileDirectory(profileNameOld);
	ASSERT_FALSE(std::filesystem::exists(existingPath));

	// 変更前プロファイル名でINIファイル作成
	CProfile cProfile;
	cProfile.WriteProfile(existingPath.c_str(), L"test profile");

	// 内部的にAddが呼ばれる
	manager.Rename(1, profileNameNew);
	ASSERT_STREQ(profileNameNew, manager.GetAt(1).c_str());

	std::filesystem::remove(existingPath);
}

/*!
 * @brief プロファイル名変更
 */
TEST(CProfileManager, Rename)
{
	// 変更前のプロファイル名
	constexpr const wchar_t profileNameOld[] = L"profile1";
	// 変更後のプロファイル名
	constexpr const wchar_t profileNameNew[] = L"profile2";

	CProfileManager manager;

	// 既定プロファイルはリネームできない
	ASSERT_THROW({ manager.Rename(0, profileNameNew); }, std::out_of_range);

	// 範囲外はリネームできない
	ASSERT_EQ(1, manager.GetSize());
	ASSERT_THROW({ manager.Rename(1, profileNameNew); }, std::out_of_range);

	// 変更前プロファイルのパスを取得
	auto existingPath = GetProfileDirectory(profileNameOld).append(GetIniFileName().filename().c_str());
	ASSERT_FALSE(std::filesystem::exists(existingPath.parent_path()));

	// 変更前プロファイルを追加
	manager.Add(1, profileNameOld);
	ASSERT_STREQ(profileNameOld, manager.GetAt(1).c_str());

	// 変更前プロファイル名でINIファイル作成
	CProfile cProfile;
	cProfile.WriteProfile(existingPath.c_str(), L"test profile");

	// 旧プロファイルがリネームされる
	manager.Rename(1, profileNameNew);
	ASSERT_STREQ(profileNameNew, manager.GetAt(1).c_str());

	std::filesystem::remove_all(GetProfileDirectory(profileNameNew));
}

/*!
 * @brief 設定ファイル入出力
 */
TEST(CProfileManager, IOProfSettings)
{
	// コマンドラインのインスタンスを用意する
	CCommandLine cCommandLine;
	// プロファイルマネージャのインスタンスを用意する
	CProfileManager manager;

	// 新しいプロファイルを追加
	manager.Add(0, L"保存用");
	manager.Add(2, L"観賞用");
	manager.Add(3, L"使用用");
	ASSERT_EQ(4, manager.GetSize());

	//4つ目(使用用)をデフォルトにする
	manager.SetDefault(3);
	ASSERT_TRUE(manager.GetDefault().has_value());
	ASSERT_EQ(3, manager.GetDefault().value());
	ASSERT_STREQ(L"使用用", manager.GetAt(manager.GetDefault().value()).c_str());

	// 設定ファイルに保存
	manager.WriteSettings();

	// プロファイルセットを空にする
	for (int n = manager.GetSize() - 1; n > 0; --n) {
		manager.Delete(n);
	}
	ASSERT_EQ(1, manager.GetSize());
	ASSERT_FALSE(manager.GetDefault().has_value());

	// 設定ファイルから読取
	manager.ReadSettings();

	ASSERT_EQ(4, manager.GetSize());
	ASSERT_STREQ(L"(default)", manager.GetAt(0).c_str());
	ASSERT_STREQ(L"保存用", manager.GetAt(1).c_str());
	ASSERT_STREQ(L"観賞用", manager.GetAt(2).c_str());
	ASSERT_STREQ(L"使用用", manager.GetAt(3).c_str());
	ASSERT_TRUE(manager.GetDefault().has_value());
	ASSERT_EQ(3, manager.GetDefault().value());

	std::filesystem::remove(GetProfileMgrFileName());
}

/*!
 * @brief プロファイルマネージャ設定ファイルパスの取得
 */
TEST(file, GetProfileMgrFileName_NoArg1)
{
	// コマンドラインのインスタンスを用意する
	CCommandLine cCommandLine;
	auto pCommandLine = &cCommandLine;
	pCommandLine->ParseCommandLine(LR"(-PROF="")", false);

	// プロセスのインスタンスを用意する
	CControlProcess dummy(nullptr, LR"(-PROF="")");

	// iniファイルの拡張子を_prof.iniに変えたパスが返る
	const auto profileMgrIniPath = GetIniFileName().replace_extension().concat(L"_prof.ini");
	ASSERT_STREQ(profileMgrIniPath.c_str(), GetProfileMgrFileName().c_str());
}

/*!
 * @brief プロファイルマネージャ設定ファイルパスの取得
 */
TEST(file, GetProfileMgrFileName_NoArg2)
{
	// コマンドラインのインスタンスを用意する
	CCommandLine cCommandLine;
	auto pCommandLine = &cCommandLine;
	pCommandLine->ParseCommandLine(LR"(-PROF="profile1")", false);

	// プロセスのインスタンスを用意する
	CControlProcess dummy(nullptr, LR"(-PROF="profile1")");

	// iniファイルの拡張子を_prof.iniに変えたパスが返る
	const auto profileMgrIniPath = GetIniFileName().parent_path().parent_path().append(GetIniFileName().replace_extension().concat(L"_prof.ini").filename().c_str());
	ASSERT_STREQ(profileMgrIniPath.c_str(), GetProfileMgrFileName().c_str());
}

/*!
 * @brief 指定したプロファイルの設定保存先ディレクトリの取得(プロファイル名が空の時)
 */
TEST(file, GetProfileMgrFileName_DefaultProfile1)
{
	// コマンドラインのインスタンスを用意する
	CCommandLine cCommandLine;
	auto pCommandLine = &cCommandLine;
	pCommandLine->ParseCommandLine(LR"(-PROF="")", false);

	// プロセスのインスタンスを用意する
	CControlProcess dummy(nullptr, LR"(-PROF="")");

	// 設定フォルダのパスが返る
	const auto iniDir = GetExeFileName().replace_filename(L"").append("a.txt").remove_filename();
	ASSERT_STREQ(iniDir.c_str(), GetProfileDirectory(L"").c_str());
}

/*!
 * @brief 指定したプロファイルの設定保存先ディレクトリの取得(プロファイル名が空の時)
 */
TEST(file, GetProfileMgrFileName_DefaultProfile2)
{
	// コマンドラインのインスタンスを用意する
	CCommandLine cCommandLine;
	auto pCommandLine = &cCommandLine;
	pCommandLine->ParseCommandLine(LR"(-PROF="profile1")", false);

	// プロセスのインスタンスを用意する
	CControlProcess dummy(nullptr, LR"(-PROF="profile1")");

	// 設定フォルダのパスが返る
	const auto iniDir = GetIniFileName().parent_path().parent_path().append("a.txt").remove_filename();
	ASSERT_STREQ(iniDir.c_str(), GetProfileDirectory(L"").c_str());
}

/*!
 * @brief 指定したプロファイルの設定保存先ディレクトリの取得(プロファイル名が空でない時)
 */
TEST(file, GetProfileMgrFileName_NamedProfile1)
{
	// コマンドラインのインスタンスを用意する
	CCommandLine cCommandLine;
	auto pCommandLine = &cCommandLine;
	pCommandLine->ParseCommandLine(LR"(-PROF="")", false);

	// プロセスのインスタンスを用意する
	CControlProcess dummy(nullptr, LR"(-PROF="")");

	// テスト用プロファイル名
	constexpr auto profile = L"profile1";

	// 指定したプロファイルの設定保存先フォルダのパスが返る
	const auto profileDir = GetExeFileName().replace_filename(profile);
	ASSERT_STREQ(profileDir.c_str(), GetProfileDirectory(profile).c_str());
}

/*!
 * @brief 指定したプロファイルの設定保存先ディレクトリの取得(プロファイル名が空でない時)
 */
TEST(file, GetProfileMgrFileName_NamedProfile2)
{
	// コマンドラインのインスタンスを用意する
	CCommandLine cCommandLine;
	auto pCommandLine = &cCommandLine;
	pCommandLine->ParseCommandLine(LR"(-PROF="profile1")", false);

	// プロセスのインスタンスを用意する
	CControlProcess dummy(nullptr, LR"(-PROF="profile1")");

	// テスト用プロファイル名
	constexpr auto profile = L"profile1";

	// 指定したプロファイルの設定保存先フォルダのパスが返る
	const auto profileDir = GetIniFileName().parent_path().parent_path().append(profile);
	ASSERT_STREQ(profileDir.c_str(), GetProfileDirectory(profile).c_str());
}
