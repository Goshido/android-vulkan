// version 1.10

#ifndef GX_TYPES
#define GX_TYPES


#ifdef __GNUC__

#include "Posix/GXTypes.h"

#else

#include "Windows/GXTypes.h"

#endif // __GNUC__


enum class eGXCompareResult : GXByte
{
    Less = -1,
    Equal = 0,
    Greater = 1
};


#endif // GX_TYPES
