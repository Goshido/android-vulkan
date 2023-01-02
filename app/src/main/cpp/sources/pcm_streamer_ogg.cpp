#include <logger.h>
#include <pcm_streamer_ogg.h>
#include <sound_mixer.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

namespace {

constexpr uint8_t OGG_BYTES_PER_CHANNEL = 2U;
static_assert ( OGG_BYTES_PER_CHANNEL == sizeof ( PCMStreamer::PCMType ) );

constexpr size_t PCM_SAMPLES_HINT = 1U * 1024U * 1024U / sizeof ( PCMStreamer::PCMType );

//----------------------------------------------------------------------------------------------------------------------

class VorbisCloser final
{
    private:
        OggVorbis_File&             _file;
        [[maybe_unused]] bool&      _isFileOpen;

    public:
        VorbisCloser () = delete;

        VorbisCloser ( VorbisCloser const & ) = delete;
        VorbisCloser& operator = ( VorbisCloser const & ) = delete;

        VorbisCloser ( VorbisCloser && ) = delete;
        VorbisCloser& operator = ( VorbisCloser && ) = delete;

        explicit VorbisCloser ( OggVorbis_File &file, bool &isFileOpen ) noexcept;

        ~VorbisCloser () noexcept;
};

VorbisCloser::VorbisCloser ( OggVorbis_File &file, bool &isFileOpen ) noexcept:
    _file ( file ),
    _isFileOpen ( isFileOpen )
{
    // NOTHING
}

VorbisCloser::~VorbisCloser () noexcept
{
    PCMStreamerOGG::CheckVorbisResult ( ov_clear ( &_file ),
        "VorbisCloser::~VorbisCloser",
        "Can't close file"
    );

    _isFileOpen = false;
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
        VorbisCloser const vorbisCloser ( _fileOGG, _isFileOpen );
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
            { OV_FALSE, "OV_FALSE" },
            { OV_EOF, "OV_EOF" },
            { OV_HOLE, "OV_HOLE" },
            { OV_EREAD, "OV_EREAD" },
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

void PCMStreamerOGG::GetNextBuffer ( std::span<PCMType> buffer, Gain left, Gain right ) noexcept
{
    PCMData const& data = _pcmBuffers[ _activeBufferIndex ];
    PCMType const* pcm = data._samples.data ();

    // Spinlock to decompress data.
    while ( _readyBuffers.load () == 0U );

    _sampleCount = data._sampleCount;

    if ( buffer.size () > _sampleCount )
    {
        // C++ calling method by pointer syntax.
        ( this->*_loopHandler ) ( buffer,
            pcm,

            Consume
            {
                ._bufferSampleCount = 0U,
                ._lastPCMBuffer = data._lastBuffer,
                ._pcmSampleCount = 0U
            },

            left,
            right
        );

        return;
    }

    // C++ calling method by pointer syntax.
    Consume const consume = ( this->*_readHandler ) ( buffer, left, right, pcm + _offset, data._lastBuffer );

    if ( _offset + consume._pcmSampleCount >= _sampleCount )
    {
        // Expectation: this is thread shared code. This should work without race condition while decompressor has
        // enough time to fill next PCM buffer. Available time is the period when active PCM buffer is consuming by
        // sound tract.
        _activeBufferIndex = ( _activeBufferIndex + 1U ) % BUFFER_COUNT;
        _readyBuffers.fetch_sub ( 1U );
    }

    // C++ calling method by pointer syntax.
    ( this->*_loopHandler ) ( buffer, pcm, consume, left, right );
}

void PCMStreamerOGG::OnDecompress () noexcept
{
    std::unique_lock<std::mutex> const lock ( _mutex );

    // Expectation: this is thread shared code. This should work without race condition while decompressor has enough
    // time to fill next PCM buffer. Available time is the period when active PCM buffer is consuming by sound tract.
    uint8_t readyBuffer = _readyBuffers.load ();

    if ( readyBuffer == BUFFER_COUNT )
        return;

    constexpr int BIG_ENDIAN = 0;
    constexpr int WORD_SIZE_16BIT = 2;
    constexpr int SIGNED_SAMPLES = 1;
    constexpr long END_OF_FILE = 0;

    auto const fill = [ this ] ( PCMData &data, long bufferBytes ) noexcept -> bool {
        int bitstream;
        long read = 0;
        char* buffer = reinterpret_cast<char*> ( data._samples.data () );
        data._lastBuffer = false;

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
            {
                data._lastBuffer = true;
                break;
            }

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

    auto const bufferBytes = static_cast<long> ( sizeof ( PCMType ) * _pcmBuffers->_samples.size () );

    if ( readyBuffer == 0U )
    {
        if ( !fill ( _pcmBuffers[ 0U ], bufferBytes ) )
            return;

        readyBuffer = 1U;
    }

    if ( !fill ( _pcmBuffers[ ( _activeBufferIndex + 1U ) % BUFFER_COUNT ], bufferBytes ) )
        return;

    _readyBuffers.store ( readyBuffer + 1U );
}

bool PCMStreamerOGG::Reset () noexcept
{
    std::unique_lock<std::mutex> const lock ( _mutex );

    if ( !PCMStreamer::Reset () )
        return false;

    if ( !_isFileOpen )
        return true;

    int const result = ov_pcm_seek ( &_fileOGG, 0 );

    if ( !CheckVorbisResult ( result, "PCMStreamerOGG::Reset", "Can't perform seek operation" ) )
    {
        assert ( false );
        return false;
    }

    _activeBufferIndex = 0U;
    _readyBuffers.store ( 0U );

    return true;
}

std::optional<PCMStreamer::Info> PCMStreamerOGG::ResolveInfo ( bool looped, size_t samplesPerBurst ) noexcept
{
    if ( _isFileOpen )
        VorbisCloser const vorbisCloser ( _fileOGG, _isFileOpen );

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
        VorbisCloser const vorbisCloser ( _fileOGG, _isFileOpen );
        return std::nullopt;
    }

    vorbis_info const* info = ov_info ( &_fileOGG, 0 );

    if ( !AllocateDecompressBuffers ( static_cast<size_t> ( info->channels ), samplesPerBurst ) )
    {
        VorbisCloser const vorbisCloser ( _fileOGG, _isFileOpen );
        return std::nullopt;
    }

    _looped = looped;

    return Info
    {
        ._bytesPerChannelSample = OGG_BYTES_PER_CHANNEL,
        ._channelCount = static_cast<uint8_t> ( info->channels ),
        ._sampleRate = static_cast<uint32_t> ( info->rate )
    };
}

bool PCMStreamerOGG::AllocateDecompressBuffers ( size_t channels, size_t samplesPerBurst ) noexcept
{
    static_assert ( SoundMixer::GetChannelCount () == 2U );

    if ( channels < 1U || channels > 2U )
    {
        LogError ( "PCMStreamerOGG::AllocateDecompressBuffers - Unsupported channel amount %zu.", channels );
        return false;
    }

    // 1 channel - samplesPerBurst should be divided by 2 to make optimal decompressed sample count.
    // 2 channels - samplesPerBurst equal to decompressed sample count.
    size_t const optimal = samplesPerBurst / ( 3U - channels );
    size_t const sampleCount = PCM_SAMPLES_HINT / optimal * optimal;

    for ( auto& buffer : _pcmBuffers )
        buffer._samples.resize ( sampleCount );

    _activeBufferIndex = 0U;
    _readyBuffers = 0U;
    return true;
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
    // Note return value should be similar to std::fread.
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
