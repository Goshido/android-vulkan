#ifndef PBR_SOUND_EMITTER_GLOBAL_COMPONENT_DESC_H
#define PBR_SOUND_EMITTER_GLOBAL_COMPONENT_DESC_H


#include "component_desc.h"
#include "primitive_types.h"


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


#endif // PBR_SOUND_EMITTER_GLOBAL_COMPONENT_DESC_H