//------------------------------------------------------------------------------
//  KjRootContextDef.cpp
//  (C) 2016 n.lee
//------------------------------------------------------------------------------
#include "KjRootContextDef.hpp"

kj::Own<KjSimpleIoContext> g_rootContext = kj::refcounted<KjSimpleIoContext>();

/* -- EOF -- */