//------------------------------------------------------------------------------
//  RootContextDef.cpp
//  (C) 2016 n.lee
//------------------------------------------------------------------------------
#include "RootContextDef.hpp"

static IServerCore *s_servercore;

void
netsystem_init_servercore(void *servercore) {
	if (!s_servercore) {
		s_servercore = static_cast<IServerCore *>(servercore);
	}
}

void
netsystem_cleanup_servercore() {
	s_servercore = nullptr;
}

IServerCore *
netsystem_get_servercore() {
	return s_servercore;
}

StdLog *
netsystem_get_log() {
	return s_servercore->GetLogHandler();
}

/* -- EOF -- */