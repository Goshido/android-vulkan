#ifndef PBR_SOUND_EMITTER_SPATIAL_COMPONENT_DESC_HPP
#define PBR_SOUND_EMITTER_SPATIAL_COMPONENT_DESC_HPP


#include "component_desc.hpp"
#include "primitive_types.hpp"


namespace pbr {

#pragma pack ( push, 1 )

struct SoundEmitterSpatialComponentDesc final : public ComponentDesc
{
    android_vulkan::eSoundChannel       _channel;
    float                               _distance;
    android_vulkan::Vec3                _location;
    android_vulkan::Boolean             _looped;
    android_vulkan::UTF8Offset          _soundAsset;
    float                               _volume;
};

#pragma pack ( pop )

} // namespace pbr


#endif // PBR_SOUND_EMITTER_SPATIAL_COMPONENT_DESC_HPP
