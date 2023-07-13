//version 1.10

#ifndef GX_TYPES_WINDOWS_HPP
#define GX_TYPES_WINDOWS_HPP


#define WIN32_LEAN_AND_MEAN            // to correct include WinSock2.h

#include "GXWarning.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <Windows.h>
#include <stdint.h>

GX_RESTORE_WARNING_STATE


#define GXCALL                                  WINAPI
#define GXCDECLCALL
#define GXTHREADCALL                            WINAPI

#define GX_TRUE                                 true
#define GX_FALSE                                false

[[maybe_unused]] typedef int16_t                GXShort;
typedef uint16_t                                GXUShort;
typedef int32_t                                 GXInt;
typedef uint32_t                                GXUInt;
[[maybe_unused]] typedef int64_t                GXBigInt;
typedef uint64_t                                GXUBigInt;
[[maybe_unused]] typedef long                   GXLong;
[[maybe_unused]] typedef long                   GXLong;
[[maybe_unused]] typedef unsigned long          GXULong;
[[maybe_unused]] typedef long long              GXLongLong;
[[maybe_unused]] typedef unsigned long long     GXULongLong;
[[maybe_unused]] typedef unsigned long          GXDword;
[[maybe_unused]] typedef int8_t                 GXByte;
typedef uint8_t                                 GXUByte;
[[maybe_unused]] typedef char                   GXChar;
[[maybe_unused]] typedef unsigned char          GXUChar;
[[maybe_unused]] typedef char                   GXUTF8;
[[maybe_unused]] typedef uint16_t               GXUTF16;
[[maybe_unused]] typedef char                   GXMBChar;
typedef wchar_t                                 GXWChar;
typedef float                                   GXFloat;
typedef double                                  GXDouble;
typedef bool                                    GXBool;
typedef void                                    GXVoid;
[[maybe_unused]] typedef ptrdiff_t              GXPointer;
typedef size_t                                  GXUPointer;


#endif // GX_TYPES_WINDOWS_HPP
