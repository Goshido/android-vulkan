#include <precompiled_headers.hpp>
#include <logger.hpp>
#include <pbr/stream.hpp>


namespace pbr {

Stream::Stream ( Data data, size_t line ) noexcept:
    _data ( data ),
    _line ( line )
{
    // NOTHING
}

void Stream::Init ( Data data, size_t line ) noexcept
{
    _data = data;
    _line = line;
}

bool Stream::ExpectNotEmpty ( char const* streamSource, char const* where ) const noexcept
{
    if ( !_data.empty () )
        return true;

    android_vulkan::LogError ( "%s - %s:%zu: Unexpected end of the document.", where, streamSource, _line );
    return false;
}

} // namespace pbr
