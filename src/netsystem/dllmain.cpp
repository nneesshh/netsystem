#include <objbase.h>
//#include <iostream>

BOOL APIENTRY DllMain(HANDLE hModule, DWORD dwReason, void* lpReserved) {
	HANDLE g_hModule;
	switch (dwReason) {
	case DLL_PROCESS_ATTACH:
		//std::cout << "Dll is attached!" << std::endl;
		g_hModule = (HINSTANCE)hModule;
		break;
	
	case DLL_PROCESS_DETACH:
		//std::cout << "Dll is detached!" << std::endl;
		g_hModule = NULL;
		break;
	}
	return true;
}
