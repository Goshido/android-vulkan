#include <av_assert.h>
#include <sound_emitter_global.h>


namespace android_vulkan {

void SoundEmitterGlobal::FillPCM ( std::span<PCMStreamer::PCMType> buffer, float channelVolume ) noexcept
{
    AV_ASSERT ( _streamer )

    float const volume = _volume * channelVolume;
    PCMStreamer::Gain const gain ( volume, volume );
    _streamer->GetNextBuffer ( buffer, gain, gain );
}

} // namespace android_vulkan
