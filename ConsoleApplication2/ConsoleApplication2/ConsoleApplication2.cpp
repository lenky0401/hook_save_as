// ConsoleApplication2.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include <Windows.h>
#include "HOOK.H"
#include <fstream>
#include <string>
#include <assert.h>

HHOOK g_HookProc1;
HHOOK g_HookProc2;
LRESULT CALLBACK MyProc1(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK MyProc2(int nCode, WPARAM wParam, LPARAM lParam);

std::ofstream g_OutFile("d:\\gqk_info.txt", std::ios::app);

bool g_FoundSrc = false;

void __declspec(dllexport) SetGlobalHook()
{
	g_HookProc1 = ::SetWindowsHookEx(WH_CBT, MyProc1, GetModuleHandle(TEXT("ConsoleApplication2.dll")), 0);
	g_HookProc2 = ::SetWindowsHookEx(WH_SYSMSGFILTER, MyProc2, GetModuleHandle(TEXT("ConsoleApplication2.dll")), 0);
}

void __declspec(dllexport) UnsetGlobalHook()
{
	if (NULL != g_HookProc1) {
		::UnhookWindowsHookEx(g_HookProc1);
	}
	if (NULL != g_HookProc2) {
		::UnhookWindowsHookEx(g_HookProc2);
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
			g_FoundSrc = false;
			g_OutFile << "原文件名：" << title << std::endl;
		}

		return FALSE;
	}

	return TRUE;
}

//https://learn.microsoft.com/zh-cn/windows/win32/winmsg/cbtproc
LRESULT CALLBACK MyProc1(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode == HCBT_CREATEWND || nCode == HCBT_DESTROYWND) {
		HWND hwnd = (HWND)wParam;
		char buf[256];
		GetClassNameA(hwnd, buf, sizeof(buf));

		if (strcmp(buf, "#32770") == 0) {
			if (nCode == HCBT_CREATEWND) {
				g_FoundSrc = true;
			} else {
				g_FoundSrc = false;
			}

			std::string title = GetTitle(hwnd);
			if (title.find("另存为") != -1 || title.find("Save As") != -1) {
				bool flag = (nCode == HCBT_DESTROYWND);
				EnumChildWindows(hwnd, EnumChildProc, (LPARAM)flag);
			}
		}
	}

	g_OutFile.flush();
	return CallNextHookEx(g_HookProc1, nCode, wParam, lParam);
}

LRESULT CALLBACK MyProc2(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode == MSGF_DIALOGBOX && g_FoundSrc == true) {
		HWND hwnd = ((PMSG)lParam)->hwnd;
		char buf[256];
		memset(buf, 0, sizeof(buf));
		GetClassNameA(hwnd, buf, sizeof(buf));

		if (strcmp(buf, "#32770") == 0) {
			std::string title = GetTitle(hwnd);
			if (title.find("另存为") != -1 || title.find("Save As") != -1) {
				bool flag = (nCode == HCBT_DESTROYWND);
				EnumChildWindows(hwnd, EnumChildProc, (LPARAM)flag);
			}
		}
	}

	g_OutFile.flush();
	return CallNextHookEx(g_HookProc2, nCode, wParam, lParam);
}