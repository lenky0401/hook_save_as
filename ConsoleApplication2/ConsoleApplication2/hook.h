#pragma once

extern "C" {
	void __declspec(dllexport) SetGlobalHook();
	void __declspec(dllexport) UnsetGlobalHook();
}
