#include <logger.h>
#include <pcm_streamer_ogg.h>
#include <sound_mixer.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>
#include <fstream>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

namespace {

constexpr size_t DECOMPRESSED_SAMPLES_HINT = 1U * 1024U * 1024U / sizeof ( PCMStreamer::PCMType );
constexpr auto DENOMINATOR = static_cast<int32_t> ( PCMStreamer::INTEGER_DIVISION_SCALE );

constexpr uint8_t OGG_BYTES_PER_CHANNEL = 2U;
static_assert ( OGG_BYTES_PER_CHANNEL == sizeof ( PCMStreamer::PCMType ) );

//----------------------------------------------------------------------------------------------------------------------

class VorbisCloser final
{
    private:
        OggVorbis_File&       _file;

    public:
        VorbisCloser () = delete;

        VorbisCloser ( VorbisCloser const & ) = delete;
        VorbisCloser& operator = ( VorbisCloser const & ) = delete;

        VorbisCloser ( VorbisCloser && ) = delete;
        VorbisCloser& operator = ( VorbisCloser && ) = delete;

        explicit VorbisCloser ( OggVorbis_File &file ) noexcept;

        ~VorbisCloser () noexcept;
};

VorbisCloser::VorbisCloser ( OggVorbis_File &file ) noexcept:
    _file ( file )
{
    // NOTHING
}

VorbisCloser::~VorbisCloser () noexcept
{
    PCMStreamerOGG::CheckVorbisResult ( ov_clear ( &_file ),
        "VorbisCloser::~VorbisCloser",
        "Can't close file"
    );
}

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

PCMStreamerOGG::PCMStreamerOGG ( SoundEmitter &soundEmitter, OnStopRequest callback ) noexcept:
    PCMStreamer ( soundEmitter, callback, true )
{
    // NOTHING
}

PCMStreamerOGG::~PCMStreamerOGG ()
{
    if ( _isFileOpen )
    {
        VorbisCloser const vorbisCloser ( _fileOGG );
    }
}

bool PCMStreamerOGG::CheckVorbisResult ( int result,
    char const* from,
    char const* message
) noexcept
{
    if ( result == 0 )
        return true;

    constexpr size_t size = 128U;
    char unknown[ size ];

    // IIFE pattern.
    char const* err = [] ( int result, char* unknown ) noexcept -> char const* {
        static std::unordered_map<int, char const*> const map
        {
            { OV_EFAULT, "OV_EFAULT" },
            { OV_EIMPL, "OV_EIMPL" },
            { OV_EINVAL, "OV_EINVAL" },
            { OV_ENOTVORBIS, "OV_ENOTVORBIS" },
            { OV_EBADHEADER, "OV_EBADHEADER" },
            { OV_EVERSION, "OV_EVERSION" },
            { OV_ENOTAUDIO, "OV_ENOTAUDIO" },
            { OV_EBADPACKET, "OV_EBADPACKET" },
            { OV_EBADLINK, "OV_EBADLINK" },
            { OV_ENOSEEK, "OV_ENOSEEK" }
        };

        if ( auto const findResult = map.find ( result ); findResult != map.cend () )
            return findResult->second;

        std::snprintf ( unknown, size, "UNKNOWN [%d]", result );
        return unknown;
    } ( result, unknown );

    LogError ( "%s - %s. Error: %s.", from, message, err );
    return false;
}

void PCMStreamerOGG::GetNextBuffer ( std::span<PCMType> buffer,
    float leftChannelVolume,
    float rightChannelVolume
) noexcept
{
    auto const leftGain = static_cast<int32_t> ( leftChannelVolume * static_cast<float> ( INTEGER_DIVISION_SCALE ) );
    auto const rightGain = static_cast<int32_t> ( rightChannelVolume * static_cast<float> ( INTEGER_DIVISION_SCALE ) );

    PCMType* target = buffer.data ();
    size_t const bufferSamples = buffer.size ();

    DecompressData const &data = _decompressedBuffers[ _activeBufferIndex ];
    PCMType const* pcm = data._samples.data ();

    // Spinlock to decompress data.
    while ( data._sampleCount == 0U );

    _sampleCount = data._sampleCount;

    // C++ calling method by pointer syntax.
    Consume const consume = ( this->*_readHandler ) ( target,
        bufferSamples,
        pcm + _offset,
        leftGain,
        rightGain
    );

    if ( _offset + consume._pcmSampleCount >= _sampleCount )
    {
        --_readyBuffers;
        _activeBufferIndex = ( _activeBufferIndex + 1U ) % BUFFER_COUNT;
        LogError ( "~~~ %zu", _activeBufferIndex );
    }

    // C++ calling method by pointer syntax.
    ( this->*_loopHandler ) ( target, bufferSamples, pcm, consume, leftGain, rightGain );
}

void PCMStreamerOGG::OnDecompress () noexcept
{
    if ( _readyBuffers == BUFFER_COUNT )
        return;

    auto const bufferBytes = static_cast<long> ( sizeof ( PCMType ) * _decompressedBuffers->_samples.size () );

    constexpr int BIG_ENDIAN = 0;
    constexpr int WORD_SIZE_16BIT = 2;
    constexpr int SIGNED_SAMPLES = 1;
    constexpr long END_OF_FILE = 0;

    LogError ( "PCMStreamerOGG::OnDecompress" );

    auto const fill = [ this ] ( DecompressData &data, long bufferBytes ) noexcept  -> bool {
        int bitstream;
        long read = 0;
        char* buffer = reinterpret_cast<char*> ( data._samples.data () );

        while ( read < bufferBytes )
        {
            long const produced = ov_read ( &_fileOGG,
                buffer + static_cast<size_t> ( read ),
                static_cast<int> ( bufferBytes - read ),
                BIG_ENDIAN,
                WORD_SIZE_16BIT,
                SIGNED_SAMPLES,
                &bitstream
            );

            if ( produced < 0 )
            {
                CheckVorbisResult ( static_cast<int> ( produced ),
                    "PCMStreamerOGG::OnDecompress",
                    "Can't decompress PCM"
                );

                assert ( false );
                return false;
            }

            read += produced;

            if ( produced != END_OF_FILE )
                continue;

            if ( !_looped )
                break;

            int const result = ov_pcm_seek ( &_fileOGG, 0 );

            if ( !CheckVorbisResult ( result, "PCMStreamerOGG::OnDecompress", "Can't perform seek operation" ) )
            {
                assert ( false );
                return false;
            }
        }

        data._sampleCount = static_cast<size_t> ( read ) / sizeof ( PCMType );
        return true;
    };

    if ( _readyBuffers == 0U )
    {
        if ( !fill ( _decompressedBuffers[ 0U ], bufferBytes ) )
            return;

        _readyBuffers = 1U;
    }

    if ( !fill ( _decompressedBuffers[ ( _activeBufferIndex + 1U ) % BUFFER_COUNT ], bufferBytes ) )
        return;

    ++_readyBuffers;
}

std::optional<PCMStreamer::Info> PCMStreamerOGG::ResolveInfo ( bool looped, size_t samplesPerBurst ) noexcept
{
    if ( _isFileOpen )
    {
        VorbisCloser const vorbisCloser ( _fileOGG );
        _isFileOpen = false;
    }

    int const res = ov_open_callbacks ( this,
        &_fileOGG,
        nullptr,
        0,

        ov_callbacks
        {
            .read_func = &PCMStreamerOGG::Read,
            .seek_func = &PCMStreamerOGG::Seek,
            .close_func = &PCMStreamerOGG::Close,
            .tell_func = &PCMStreamerOGG::Tell
        }
    );

    _isFileOpen = true;

    if ( !CheckVorbisResult ( res, "PCMStreamerOGG::ResolveInfo", "Can't open asset" ) )
    {
        VorbisCloser const vorbisCloser ( _fileOGG );
        _isFileOpen = false;
        return std::nullopt;
    }

    vorbis_info const* info = ov_info ( &_fileOGG, 0 );
    int const result = ov_pcm_seek ( &_fileOGG, 0 );

    if ( !CheckVorbisResult ( result, "PCMStreamerOGG::ResolveInfo", "Can't perform seek operation" ) )
    {
        VorbisCloser const vorbisCloser ( _fileOGG );
        _isFileOpen = false;
        return std::nullopt;
    }

    bool const isStereo = info->channels > 1;
    constexpr ReadHandler const readHandlers[] = { &PCMStreamerOGG::HandleMono, &PCMStreamerOGG::HandleStereo };
    _readHandler = readHandlers[ static_cast<size_t> ( isStereo ) ];

    constexpr LoopHandler const loopHandlers[] =
    {
        &PCMStreamerOGG::HandleNonLooped,
        &PCMStreamerOGG::HandleNonLooped,
        &PCMStreamerOGG::HandleLoopedMono,
        &PCMStreamerOGG::HandleLoopedStereo
    };

    _loopHandler = loopHandlers[ ( static_cast<size_t> ( looped ) << 1U ) | static_cast<size_t> ( isStereo ) ];
    _looped = looped;
    AllocateDecompressBuffers ( static_cast<size_t> ( info->channels ), samplesPerBurst );

//    Info inf
//    {
//        ._bytesPerChannelSample = OGG_BYTES_PER_CHANNEL,
//        ._channelCount = static_cast<uint8_t> ( info->channels ),
//        ._sampleRate = static_cast<uint32_t> ( info->rate )
//    };
//
//    {
//        VorbisCloser const vorbisCloser ( _fileOGG );
//    }
//
//    ov_open_callbacks ( this,
//        &_fileOGG,
//        nullptr,
//        0,
//
//        ov_callbacks
//        {
//            .read_func = &PCMStreamerOGG::Read,
//            .seek_func = &PCMStreamerOGG::Seek,
//            .close_func = &PCMStreamerOGG::Close,
//            .tell_func = &PCMStreamerOGG::Tell
//        }
//    );

//    OnDecompress ();

//    std::ofstream report ( "/data/data/com.goshidoInc.androidVulkan/cache/q.wav",
//        std::ios::binary | std::ios::trunc | std::ios::out
//    );
//
//    size_t const sz = _decompressedBuffers[ 0U ]._sampleCount * sizeof ( PCMType );
//    report.write ( reinterpret_cast<char const*> ( _decompressedBuffers[ 0U ]._samples.data () ), sz );

//    return inf;

    return Info
    {
        ._bytesPerChannelSample = OGG_BYTES_PER_CHANNEL,
        ._channelCount = static_cast<uint8_t> ( info->channels ),
        ._sampleRate = static_cast<uint32_t> ( info->rate )
    };
}

void PCMStreamerOGG::AllocateDecompressBuffers ( size_t channels, size_t samplesPerBurst ) noexcept
{
    static_assert ( SoundMixer::GetChannelCount () == 2U );

    // 2 channels - samplesPerBurst equal to decompressed sample count.
    // 1 channel - samplesPerBurst should be divided by 2 to make optimal decompressed sample count.
    size_t const optimal = samplesPerBurst / ( 3U - channels );
    size_t const sampleCount = DECOMPRESSED_SAMPLES_HINT / optimal * optimal;

    for ( auto& buffer : _decompressedBuffers )
        buffer._samples.resize ( sampleCount );

    _activeBufferIndex = 0U;
    _readyBuffers = 0U;
}

void PCMStreamerOGG::HandleLoopedMono ( PCMType* buffer,
    size_t bufferSamples,
    PCMType const* pcm,
    Consume consume,
    int32_t leftGain,
    int32_t rightGain
) noexcept
{
    size_t const restInBufferSamples = bufferSamples - consume._bufferSampleCount;
    _offset = ( _offset + consume._pcmSampleCount ) % _sampleCount;

    if ( restInBufferSamples == 0U )
        return;

    buffer += bufferSamples - restInBufferSamples;
    size_t const restInPCMSamples = restInBufferSamples >> 1U;

    for ( size_t i = 0U; i < restInPCMSamples; ++i )
    {
        size_t const leftIdx = i << 1U;
        size_t const rightIdx = leftIdx + 1U;

        auto const sample = static_cast<int32_t> ( pcm[ i ] );

        buffer[ leftIdx ] = static_cast<PCMType> ( sample * leftGain / DENOMINATOR );
        buffer[ rightIdx ] = static_cast<PCMType> ( sample * rightGain / DENOMINATOR );
    }

    _offset = restInPCMSamples;
}

void PCMStreamerOGG::HandleLoopedStereo ( PCMType* buffer,
    size_t bufferSamples,
    PCMType const* pcm,
    Consume consume,
    int32_t leftGain,
    int32_t rightGain
) noexcept
{
    size_t const rest = bufferSamples - consume._bufferSampleCount;
    _offset = ( _offset + consume._pcmSampleCount ) % _sampleCount;

    if ( rest == 0U )
        return;

    buffer += bufferSamples - rest;

    for ( size_t i = 0U; i < rest; i += 2U )
    {
        size_t const rightIdx = i + 1U;

        auto const leftSample = static_cast<int32_t> ( pcm[ i ] );
        auto const rightSample = static_cast<int32_t> ( pcm[ rightIdx ] );

        buffer[ i ] = static_cast<PCMType> ( leftSample * leftGain / DENOMINATOR );
        buffer[ rightIdx ] = static_cast<PCMType> ( rightSample * rightGain / DENOMINATOR );
    }

    _offset = rest;
}

void PCMStreamerOGG::HandleNonLooped ( PCMType* buffer,
    size_t bufferSamples,
    PCMType const* /*pcm*/,
    Consume consume,
    int32_t /*leftGain*/,
    int32_t /*rightGain*/
) noexcept
{
    size_t const rest = bufferSamples - consume._bufferSampleCount;
    _offset += consume._pcmSampleCount;

    if ( rest > 0U )
    {
        std::memset ( buffer + ( bufferSamples - rest ), 0, rest * sizeof ( PCMType ) );
        return;
    }

    if ( _offset < _sampleCount )
        return;

    _onStopRequest ( _soundEmitter );
    _offset = 0U;
}

PCMStreamerOGG::Consume PCMStreamerOGG::HandleMono ( PCMType* buffer,
    size_t bufferSamples,
    PCMType const* pcm,
    int32_t leftGain,
    int32_t rightGain
) noexcept
{
    size_t const bufferFrames = bufferSamples >> 1U;

    // Buffer size should be smaller than total samples. Otherwise it's needed a more complicated implementation.
    assert ( bufferFrames <= _sampleCount );

    size_t const cases[] = { bufferFrames, _sampleCount - _offset };
    size_t const canRead = cases[ static_cast<size_t> ( _offset + bufferFrames > _sampleCount ) ];

    for ( size_t i = 0U; i < canRead; ++i )
    {
        size_t const leftIdx = i << 1U;
        size_t const rightIdx = leftIdx + 1U;

        auto const sample = static_cast<int32_t> ( pcm[ i ] );

        buffer[ leftIdx ] = static_cast<PCMType> ( sample * leftGain / DENOMINATOR );
        buffer[ rightIdx ] = static_cast<PCMType> ( sample * rightGain / DENOMINATOR );
    }

    return Consume
    {
        ._bufferSampleCount = canRead << 1U,
        ._pcmSampleCount = canRead
    };
}

PCMStreamerOGG::Consume PCMStreamerOGG::HandleStereo ( PCMType* buffer,
    size_t bufferSamples,
    PCMType const* pcm,
    int32_t leftGain,
    int32_t rightGain
) noexcept
{
    // Buffer size should be smaller than total samples. Otherwise it's needed a more complicated implementation.
    assert ( _sampleCount >= bufferSamples );

    size_t const cases[] = { bufferSamples, _sampleCount - _offset };
    size_t const canRead = cases[ static_cast<size_t> ( _offset + bufferSamples > _sampleCount ) ];

    for ( size_t i = 0U; i < canRead; i += 2U )
    {
        size_t const rightIdx = i + 1U;

        auto const leftSample = static_cast<int32_t> ( pcm[ i ] );
        auto const rightSample = static_cast<int32_t> ( pcm[ rightIdx ] );

        buffer[ i ] = static_cast<PCMType> ( leftSample * leftGain / DENOMINATOR );
        buffer[ rightIdx ] = static_cast<PCMType> ( rightSample * rightGain / DENOMINATOR );
    }

    return Consume
    {
        ._bufferSampleCount = canRead,
        ._pcmSampleCount = canRead
    };
}

int PCMStreamerOGG::CloseInternal () noexcept
{
    _positionOGG = 0;
    return 0;
}

size_t PCMStreamerOGG::ReadInternal ( void* ptr, size_t size, size_t count ) noexcept
{
    size_t const requested = size * count;
    size_t const rest = _soundFile->size () - static_cast<size_t> ( _positionOGG );

    size_t const cases[] = { requested, rest };
    size_t const read = cases[ static_cast<size_t> ( requested > rest ) ];

    std::memcpy ( ptr, _soundFile->data () + static_cast<size_t> ( _positionOGG ), read );
    _positionOGG += static_cast<ogg_int64_t> ( read );
    return read / size;
}

int PCMStreamerOGG::SeekInternal ( ogg_int64_t offset, int whence ) noexcept
{
    auto const total = static_cast<ogg_int64_t> ( _soundFile->size () );

    switch ( whence )
    {
        case SEEK_SET:
            _positionOGG = offset;
        break;

        case SEEK_CUR:
            _positionOGG += offset;
        break;

        case SEEK_END:
            _positionOGG = offset + total;
        break;

        default:
            // IMPOSSIBLE
            assert ( false );
        break;
    }

    _positionOGG = std::clamp ( _positionOGG, static_cast<ogg_int64_t> ( 0 ), total );
    return 0;
}

long PCMStreamerOGG::TellInternal () const noexcept
{
    return static_cast<long> ( _positionOGG );
}

int PCMStreamerOGG::Close ( void* datasource )
{
    auto& streamer = *static_cast<PCMStreamerOGG*> ( datasource );
    return streamer.CloseInternal ();
}

size_t PCMStreamerOGG::Read ( void* ptr, size_t size, size_t count, void* datasource )
{
    // Note return value should be similar to std::fread
    // https://en.cppreference.com/w/cpp/io/c/fread
    auto& streamer = *static_cast<PCMStreamerOGG*> ( datasource );
    return streamer.ReadInternal ( ptr, size, count );
}

int PCMStreamerOGG::Seek ( void* datasource, ogg_int64_t offset, int whence )
{
    auto& streamer = *static_cast<PCMStreamerOGG*> ( datasource );
    return streamer.SeekInternal ( offset, whence );
}

long PCMStreamerOGG::Tell ( void* datasource )
{
    auto const& streamer = *static_cast<PCMStreamerOGG const*> ( datasource );
    return streamer.TellInternal ();
}

} // namespace android_vulkan
