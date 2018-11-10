#pragma once
//------------------------------------------------------------------------------
/**
    @class CUsingMyToolkitMini
    
    (C) 2016 n.lee
*/
#include "toolkit/UsingProtobuf.h"
#include "toolkit/UsingCapnp.h"

#include "toolkit/log/StdLog.h"
#include "toolkit/platform/types.h"

#include "toolkit/core/MyMacros.h"
#include "toolkit/core/SimpleTimer.h"
#include "toolkit/core/snprintf/mysnprintf.h"

#include "toolkit/cityhash/city.h"

#include "toolkit/utils/MyUtil.h"

#ifdef _WIN32
#pragma comment(lib, "WS2_32.Lib")
#endif

/*EOF*/