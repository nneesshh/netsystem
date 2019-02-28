#ifndef _ROOT_CONTEXT_DEF_HPP__
#define _ROOT_CONTEXT_DEF_HPP__

//------------------------------------------------------------------------------
/**
	@class CRootContextDef

	(C) 2016 n.lee
*/
#include "servercore/base/IServerCore.h"

extern "C" void netsystem_init_servercore(void *servercore);
extern "C" void netsystem_cleanup_servercore();
extern "C" IServerCore * netsystem_get_servercore();

extern "C" StdLog * netsystem_get_log();

#endif /* _ROOT_CONTEXT_DEF_HPP__ */