#ifndef ANDROID_VULKAN_API_HPP
#define ANDROID_VULKAN_API_HPP


#include <GXCommon/GXWarning.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <vulkan/vulkan.h>

GX_RESTORE_WARNING_STATE


// X macro technique:
// https://en.wikipedia.org/wiki/X_Macro
#define X(name) extern PFN_##name name;

#include <vulkan_bootstrap.in>
#include <vulkan_device.in>
#include <vulkan_instance.in>

#undef X


#endif // ANDROID_VULKAN_API_HPP
