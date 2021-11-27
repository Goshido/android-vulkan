//version 1.7

#ifndef GX_TYPES_POSIX
#define GX_TYPES_POSIX


#include <stdint.h>


#define GXCALL
#define GXCDECLCALL
#define GXTHREADCALL

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
[[maybe_unused]] typedef char                   GXMBChar;
typedef wchar_t                                 GXWChar;
typedef float                                   GXFloat;
typedef double                                  GXDouble;
typedef bool                                    GXBool;
typedef void                                    GXVoid;
[[maybe_unused]] typedef ptrdiff_t              GXPointer;
typedef size_t                                  GXUPointer;


#endif // GX_TYPES_POSIX
