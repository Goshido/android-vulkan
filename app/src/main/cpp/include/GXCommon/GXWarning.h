// version 1.2
// No include guard allowed


#ifdef _MSC_VER

#include "Windows/GXWarning.h"

#elif __GNUC__

#include "Posix/GXWarning.h"

#else

#error Unsupported compiler detected!

#endif
