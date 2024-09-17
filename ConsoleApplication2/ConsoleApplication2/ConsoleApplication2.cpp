// ConsoleApplication2.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include <Windows.h>
#include "HOOK.H"
#include <fstream>
#include <string>
#include <assert.h>

HHOOK g_HookProc;
LRESULT CALLBACK MyProc(int nCode, WPARAM wParam, LPARAM lParam);

std::ofstream g_OutFile("d:\\gqk_info.txt", std::ios::app);

enum FoundStat
{
	STAT_NULL = 0,
	STAT_SAVE_AS_DIALOG,
	STAT_SOURCE_FILE,
};

struct FoundData {
	FoundStat stat;
	HWND hwnd;
};

FoundData g_FoundData;

void __declspec(dllexport) SetGlobalHook()
{
	g_HookProc = ::SetWindowsHookEx(WH_CBT, MyProc, GetModuleHandle(TEXT("ConsoleApplication2.dll")), 0);
}

void __declspec(dllexport) UnsetGlobalHook()
{
	if (NULL != g_HookProc) {
		::UnhookWindowsHookEx(g_HookProc);
	}
}

//wstring 转 string
static std::string Wstr2Str(const TCHAR* tstr)
{
	int len = WideCharToMultiByte(CP_ACP, 0, tstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len];
	WideCharToMultiByte(CP_ACP, 0, tstr, -1, str, len, NULL, NULL);

	std::string ret = str;
	delete[] str;
	return ret;
}

//获取窗口标题
static std::string GetTitle(HWND hwnd)
{
	TCHAR buf_w[256];
	memset(buf_w, 0, sizeof(buf_w));
	if (GetWindowTextW(hwnd, buf_w, sizeof(buf_w)) <= 0) {
		return "";
	}
	return Wstr2Str(buf_w);
}

//遍历所有子孙窗口，试了FindWindowEx，仅搜索直接子窗口，
//不搜索其他后代，导致找不到Edit输入框。
static BOOL CALLBACK EnumChildProc(HWND hwnd, LPARAM lParam)
{
	char buf[256];
	GetClassNameA(hwnd, buf, sizeof(buf));

	if (strcmp(buf, "Edit") == 0) {
		std::string title = GetTitle(hwnd);
		if (title == "") {
			return TRUE;
		}

		bool flag = (bool)lParam;
		if (flag) {
			g_OutFile << "新文件名：" << title << std::endl;
		}
		else {
			g_FoundData.stat = STAT_SOURCE_FILE;
			g_OutFile << "原文件名：" << title << std::endl;
		}

		return FALSE;
	}

	return TRUE;
}

//https://learn.microsoft.com/zh-cn/windows/win32/winmsg/cbtproc
LRESULT CALLBACK MyProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	do {
		if (nCode == HCBT_SETFOCUS || nCode == HCBT_DESTROYWND) {
			HWND hwnd = (HWND)wParam;
			char buf[256];
			GetClassNameA(hwnd, buf, sizeof(buf));

			g_OutFile << "code：" << nCode << ", buf1：" << buf << std::endl;

			std::string title = GetTitle(hwnd);
			g_OutFile << "title0：" << title << std::endl;

			if (strcmp(buf, "#32770") == 0) {
				

				std::string title = GetTitle(hwnd);
				g_OutFile << "title1：" << title << std::endl;
				if (title.find("另存为") != -1) {

					if (g_FoundData.stat == STAT_NULL) {
						g_FoundData.hwnd = hwnd;
						g_FoundData.stat = STAT_SAVE_AS_DIALOG;
					}
					assert(g_FoundData.hwnd == hwnd);

					bool flag = (nCode == HCBT_DESTROYWND);
					if (flag) {
						g_FoundData.stat = STAT_NULL;
					}
					if (g_FoundData.stat == STAT_SOURCE_FILE) {
						break;
					}

					if (EnumChildWindows(hwnd, EnumChildProc, (LPARAM)flag) == FALSE) {
						break;
					}
				}
			}
		}
	} while (false);

	g_OutFile.flush();
	
	return CallNextHookEx(g_HookProc, nCode, wParam, lParam);
}