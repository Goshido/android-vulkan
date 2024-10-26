#ifndef ANDROID_VULKAN_PRECOMPILED_HEADERS_HPP
#define ANDROID_VULKAN_PRECOMPILED_HEADERS_HPP


#include "GXCommon/GXWarning.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <algorithm>
#include <array>
#include <atomic>
#include <bit>
#include <cassert>
#include <chrono>
#include <cinttypes>
#include <cmath>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <deque>
#include <filesystem>
#include <float.h>
#include <forward_list>
#include <fstream>
#include <limits.h>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <regex>
#include <set>
#include <span>
#include <stdint.h>
#include <string_view>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <vulkan/vulkan.h>

#ifdef _WIN64

#include <Windows.h>
#include <windowsx.h>
#include <DbgHelp.h>
#include <WinPixEventRuntime/pix3.h>
#include <strsafe.h>
#include <tchar.h>

#endif // _WIN64

GX_RESTORE_WARNING_STATE


#endif // ANDROID_VULKAN_PRECOMPILED_HEADERS_HPP
