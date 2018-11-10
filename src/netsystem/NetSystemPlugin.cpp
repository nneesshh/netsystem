//------------------------------------------------------------------------------
//  NetSystemPlugin.cpp
//  (C) 2016 n.lee
//------------------------------------------------------------------------------
#include "NetSystem.h"

#ifdef _MSC_VER
# ifdef _DEBUG
#  include <vld.h>
# endif
#endif

#ifdef _WIN32
#pragma comment(lib, "IPHlpApi.Lib")
#pragma comment(lib, "Wldap32.Lib")
#pragma comment(lib, "Psapi.Lib")
#pragma comment(lib, "UserEnv.Lib")
#endif

#ifdef _MSC_VER
#ifdef _DEBUG
#define new   new(_NORMAL_BLOCK, __FILE__,__LINE__)
#endif
#endif

extern "C" {
	MY_EXTERN INetSystem *
		GetPlugin() {
		return new CNetSystem();
	}

	MY_EXTERN INetSystem *
		GetClass() {
		return GetPlugin();
	}
}

/** -- EOF -- **/