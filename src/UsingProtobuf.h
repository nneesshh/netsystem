#pragma once
//------------------------------------------------------------------------------
/**
    @class CUsingProtobuf
    
    (C) 2016 n.lee
*/
#ifdef _WIN32
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN 1
#  endif
#  include <Windows.h>	// include Windows.h to avoid "GetMessage" conflict between windows and protobuf
#endif

#include "thirdparty/g/protobuf/src/google/protobuf/message.h"
#include "thirdparty/g/protobuf/src/google/protobuf/descriptor.h"
#include "thirdparty/g/protobuf/src/google/protobuf/dynamic_message.h"
#include "thirdparty/g/protobuf/src/google/protobuf/io/zero_copy_stream_impl.h"
#include "thirdparty/g/protobuf/src/google/protobuf/io/tokenizer.h"
#include "thirdparty/g/protobuf/src/google/protobuf/compiler/parser.h"
#include "thirdparty/g/protobuf/src/google/protobuf/compiler/importer.h"
#include "thirdparty/g/protobuf/src/google/protobuf/reflection.h"

/*EOF*/