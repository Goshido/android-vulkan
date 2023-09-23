#ifndef PBR_MATERIAL_INFO_HPP
#define PBR_MATERIAL_INFO_HPP


#include <android_vulkan_sdk/primitive_types.hpp>


namespace pbr {

#pragma pack ( push, 1 )

enum class eMaterialTypeDesc : uint32_t
{
    Opaque = 0U,
    Stipple = 1U,

    COUNT = 2U
};

struct MaterialHeader
{
    eMaterialTypeDesc       _type;
    uint32_t                _formatVersion;
};

struct OpaqueMaterialHeader final : public MaterialHeader
{
    android_vulkan::UTF8Offset      _diffuseOffset;
    android_vulkan::UTF8Offset      _emissionOffset;
    android_vulkan::UTF8Offset      _maskOffset;
    android_vulkan::UTF8Offset      _normalOffset;
    android_vulkan::UTF8Offset      _paramOffset;

    [[maybe_unused]] float          _roughnessScale;
};

struct StippleMaterialHeader final : public MaterialHeader
{
    android_vulkan::UTF8Offset      _diffuseOffset;
    android_vulkan::UTF8Offset      _emissionOffset;
    android_vulkan::UTF8Offset      _maskOffset;
    android_vulkan::UTF8Offset      _normalOffset;
    android_vulkan::UTF8Offset      _paramOffset;

    [[maybe_unused]] float          _roughnessScale;
};

#pragma pack ( pop )

} // namespace pbr


#endif // PBR_MATERIAL_INFO_HPP
