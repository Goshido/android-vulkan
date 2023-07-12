// version 1.2
// No include guard allowed


#ifdef _MSC_VER

#include "Windows/GXWarning.hpp"

#elif __GNUC__

#include "Posix/GXWarning.hpp"

#else

#error Unsupported compiler detected!

#endif
