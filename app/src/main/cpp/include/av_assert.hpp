#ifndef ANDROID_VULKAN_ASSERT_HPP
#define ANDROID_VULKAN_ASSERT_HPP


#ifdef NDEBUG

#define AV_ASSERT(x)

#else

#include <GXCommon/GXWarning.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>

GX_RESTORE_WARNING_STATE


// Note lambda syntax is used here only for preventing unreachable code warning from static analyzer.
// Not so proud of this code. Maybe there is a more elegant compiler agnostic solution for this...
#define AV_ASSERT(x)            \
{                               \
    [ & ] () noexcept {         \
        assert ( ( x ) );       \
    } ();                       \
}

#endif // NDEBUG


#endif // ANDROID_VULKAN_ASSERT_HPP
