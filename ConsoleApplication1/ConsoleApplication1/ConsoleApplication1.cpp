// ConsoleApplication1.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <windows.h>
#include <fstream>
#include <string>

int _tmain(int argc, _TCHAR* argv[])
{
	typedef void(*typedef_SetGlobalHook)();
	typedef void(*typedef_UnsetGlobalHook)();
	HMODULE hDll = NULL;
	typedef_SetGlobalHook SetGlobalHook = NULL;
	typedef_UnsetGlobalHook UnsetGlobalHook = NULL;
	BOOL bRet = FALSE;

	do
	{
		hDll = ::LoadLibraryA("D:\\ConsoleApplication2.dll");
		if (NULL == hDll)
		{
			printf("LoadLibrary Error[%d]\n", ::GetLastError());
			break;
		}

		SetGlobalHook = (typedef_SetGlobalHook)::GetProcAddress(hDll, "SetGlobalHook");
		if (NULL == SetGlobalHook)
		{
			printf("GetProcAddress Error[%d]\n", ::GetLastError());
			break;
		}

		(void)SetGlobalHook();

		printf("SetGlobalHook ok.\n");
		system("pause");

		UnsetGlobalHook = (typedef_UnsetGlobalHook)::GetProcAddress(hDll, "UnsetGlobalHook");
		if (NULL == UnsetGlobalHook)
		{
			printf("GetProcAddress Error[%d]\n", ::GetLastError());
			break;
		}
		UnsetGlobalHook();

		printf("UnsetGlobalHook OK.\n");
		Sleep(3000);

	} while (FALSE);

	return 0;
}
