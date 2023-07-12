#ifndef ANDROID_VULKAN_BITWISE_HPP
#define ANDROID_VULKAN_BITWISE_HPP


// Main purpose of this macro is to allow bitwise logic operations for bool values.
// clang 14.0.6 is more strict compiler than before.
#define AV_BITWISE(x) static_cast<uint8_t> ( ( x ) )


#endif // ANDROID_VULKAN_BITWISE_HPP
