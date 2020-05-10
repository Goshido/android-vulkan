//version 1.6

#ifndef GX_TYPES_POSIX
#define GX_TYPES_POSIX


#include <stdint.h>


#define GXCALL
#define GXCDECLCALL
#define GXTHREADCALL

#define GX_TRUE					true
#define GX_FALSE				false


typedef int16_t					GXShort;
typedef uint16_t				GXUShort;
typedef int32_t					GXInt;
typedef uint32_t				GXUInt;
typedef int64_t					GXBigInt;
typedef uint64_t				GXUBigInt;
typedef long					GXLong;
typedef long					GXLong;
typedef unsigned long			GXULong;
typedef long long				GXLongLong;
typedef unsigned long long		GXULongLong;
typedef unsigned long			GXDword;
typedef int8_t					GXByte;
typedef uint8_t					GXUByte;
typedef char					GXChar;
typedef unsigned char			GXUChar;
typedef char					GXUTF8;
typedef char					GXMBChar;
typedef wchar_t					GXWChar;
typedef float					GXFloat;
typedef double					GXDouble;
typedef bool					GXBool;
typedef void					GXVoid;
typedef ptrdiff_t				GXPointer;
typedef size_t					GXUPointer;


#endif // GX_TYPES_POSIX
