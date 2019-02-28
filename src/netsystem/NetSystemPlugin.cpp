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

#ifdef _MSC_VER
#ifdef _DEBUG
#define new   new(_NORMAL_BLOCK, __FILE__,__LINE__)
#endif
#endif

extern "C" {
	MY_NETSYSTEM_EXTERN INetSystem *
		GetPlugin(void *servercore) {
		return new CNetSystem(servercore);
	}

	MY_NETSYSTEM_EXTERN INetSystem *
		GetClass(void *servercore) {
		return GetPlugin(servercore);
	}
}

/** -- EOF -- **/