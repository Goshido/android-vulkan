#ifndef PBR_SCENE_DESC_HPP
#define PBR_SCENE_DESC_HPP


#include <android_vulkan_sdk/primitive_types.hpp>


namespace pbr {

#pragma pack ( push, 1 )

struct SceneDesc final
{
    uint32_t                        _formatVersion;
    uint64_t                        _actorCount;
    [[maybe_unused]] uint64_t       _componentCount;
    uint64_t                        _envMapCount;
    uint64_t                        _textureCount;
    uint64_t                        _meshCount;
    android_vulkan::Vec3            _viewerLocation;

    // Value is stored in radians in range [-pi / 2, pi / 2].
    float                           _viewerPitch;

    // Value is stored in radians in range [-pi, pi].
    float                           _viewerYaw;
};

#pragma pack ( pop )

} // namespace pbr


#endif // PBR_SCENE_DESC_HPP
