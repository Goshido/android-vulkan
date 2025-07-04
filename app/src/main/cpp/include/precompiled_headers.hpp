#ifndef ANDROID_VULKAN_PRECOMPILED_HEADERS_HPP
#define ANDROID_VULKAN_PRECOMPILED_HEADERS_HPP


#include "GXCommon/GXWarning.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <algorithm>
#include <array>
#include <assert.h>
#include <atomic>
#include <bit>
#include <cassert>
#include <cctype>
#include <cfloat>
#include <chrono>
#include <cinttypes>
#include <climits>
#include <cmath>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <deque>
#include <filesystem>
#include <float.h>
#include <forward_list>
#include <fstream>
#include <functional>
#include <list>
#include <locale>
#include <map>
#include <math.h>
#include <memory>
#include <mutex>
#include <numeric>
#include <optional>
#include <random>
#include <ranges>
#include <regex>
#include <set>
#include <shared_mutex>
#include <span>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string_view>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

#ifdef __ANDROID__

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/keycodes.h>
#include <android/log.h>
#include <android/native_window_jni.h>
#include <android/trace.h>
#include <dlfcn.h>
#include <unistd.h>

#endif // __ANDROID__

#ifdef VK_NO_PROTOTYPES

#include <vulkan/vulkan.h>

#endif // VK_NO_PROTOTYPES

#ifdef AV_ARM_NEON

#include <arm_neon.h>

#endif // AV_ARM_NEON

#ifdef AV_3DS_MAX

#include <IGame/IGame.h>
#include <IGame/IGameModifier.h>
#include <IGame/IGameObject.h>
#include <iparamb2.h>
#include <maxapi.h>
#include <plugapi.h>
#include <plugin.h>
#include <utilapi.h>

#endif // AV_3DS_MAX

#ifdef _WIN64

#include <Windows.h>
#include <windowsx.h>
#include <DbgHelp.h>
#include <Shobjidl.h>
#include <shtypes.h>
#include <strsafe.h>
#include <tchar.h>

#endif // _WIN64

#ifdef AV_PIX

#include <WinPixEventRuntime/pix3.h>

#endif // AV_PIX

#ifdef AV_FREETYPE

#include <ft2build.h>
#include FT_FREETYPE_H
#include <freetype/tttables.h>

#endif // AV_FREETYPE

GX_RESTORE_WARNING_STATE


#endif // ANDROID_VULKAN_PRECOMPILED_HEADERS_HPP
