#include <sound_emitter_global.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

void SoundEmitterGlobal::FillPCM ( std::span<PCMStreamer::PCMType> buffer, float channelVolume ) noexcept
{
    assert ( _streamer );

    float const volume = _volume * channelVolume;
    PCMStreamer::Gain const gain ( volume, volume );
    _streamer->GetNextBuffer ( buffer, gain, gain );
}

} // namespace android_vulkan
