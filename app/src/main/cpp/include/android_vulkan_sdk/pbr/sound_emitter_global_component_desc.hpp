#ifndef PBR_SOUND_EMITTER_GLOBAL_COMPONENT_DESC_HPP
#define PBR_SOUND_EMITTER_GLOBAL_COMPONENT_DESC_HPP


#include "component_desc.hpp"
#include <android_vulkan_sdk/primitive_types.hpp>


namespace pbr {

#pragma pack ( push, 1 )

struct SoundEmitterGlobalComponentDesc final : public ComponentDesc
{
    android_vulkan::eSoundChannel       _channel;
    android_vulkan::Boolean             _looped;
    android_vulkan::UTF8Offset          _soundAsset;
    float                               _volume;
};

#pragma pack ( pop )

} // namespace pbr


#endif // PBR_SOUND_EMITTER_GLOBAL_COMPONENT_DESC_HPP
