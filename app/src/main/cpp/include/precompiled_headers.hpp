#ifndef ANDROID_VULKAN_PRECOMPILED_HEADERS_HPP
#define ANDROID_VULKAN_PRECOMPILED_HEADERS_HPP


#include "GXCommon/GXWarning.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <algorithm>
#include <array>
#include <atomic>
#include <bit>
#include <cassert>
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
#include <ctime>
#include <deque>
#include <filesystem>
#include <forward_list>
#include <fstream>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <regex>
#include <set>
#include <span>
#include <string_view>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#ifdef VK_NO_PROTOTYPES

#include <vulkan/vulkan.h>

#endif // VK_NO_PROTOTYPES

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

GX_RESTORE_WARNING_STATE


#endif // ANDROID_VULKAN_PRECOMPILED_HEADERS_HPP
