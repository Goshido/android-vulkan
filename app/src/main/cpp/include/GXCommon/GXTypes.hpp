// version 1.12

#ifndef GX_TYPES_HPP
#define GX_TYPES_HPP


#ifdef __GNUC__

#include "Posix/GXTypes.h"

#else

#include "Windows/GXTypes.h"

#endif // __GNUC__


enum class eGXCompareResult : GXByte
{
    Less [[maybe_unused]] = -1,
    Equal [[maybe_unused]] = 0,
    Greater [[maybe_unused]] = 1
};


#endif // GX_TYPES_HPP
