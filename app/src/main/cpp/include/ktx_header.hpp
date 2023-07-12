#ifndef KTX_HEADER_HPP
#define KTX_HEADER_HPP


#include <GXCommon/GXWarning.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <cinttypes>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

#pragma pack ( push, 1 )

// see https://www.khronos.org/opengles/sdk/tools/KTX/file_format_spec
struct KTXHeader final
{
        uint8_t                         _identifier[ 12U ];
        uint32_t                        _endianness;
        uint32_t                        _glType;
        uint32_t                        _glTypeSize;
        uint32_t                        _glFormat;
        uint32_t                        _glInternalFormat;
        [[maybe_unused]] uint32_t       _glBaseInternalFormat;
        uint32_t                        _pixelWidth;
        uint32_t                        _pixelHeight;
        uint32_t                        _pixelDepth;
        uint32_t                        _numberOfArrayElements;
        uint32_t                        _numberOfFaces;
        uint32_t                        _numberOfMipmapLevels;
        uint32_t                        _bytesOfKeyValueData;
};

#pragma pack ( pop )

} // namespace android_vulkan


#endif // KTX_HEADER_HPP
