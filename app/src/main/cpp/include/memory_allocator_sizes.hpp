#ifndef ANDROID_VULKAN_MEMORY_ALLOCATOR_SIZES_HPP
#define ANDROID_VULKAN_MEMORY_ALLOCATOR_SIZES_HPP


#include <GXCommon/GXWarning.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <vulkan/vulkan_core.h>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

namespace {

constexpr VkDeviceSize BYTES_PER_KILOBYTE = 1024U;
constexpr VkDeviceSize BYTES_PER_MEGABYTE = 1024U * BYTES_PER_KILOBYTE;
constexpr VkDeviceSize BYTES_PER_GIGABYTE = 1024U * BYTES_PER_MEGABYTE;

constexpr VkDeviceSize MEGABYTES_PER_CHUNK = 128U;
constexpr VkDeviceSize BYTES_PER_CHUNK = MEGABYTES_PER_CHUNK * BYTES_PER_MEGABYTE;

constexpr size_t SNAPSHOT_INITIAL_SIZE_MEGABYTES = 16U;

} // end of anonymous namespace

} // namespace android_vulkan


#endif // ANDROID_VULKAN_MEMORY_ALLOCATOR_SIZES_HPP
