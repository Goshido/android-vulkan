#include <precompiled_headers.hpp>
#include <stream_buffer_info.hpp>


namespace editor {

StreamBufferInfo::StreamBufferInfo ( StreamBufferRef buffer, StreamBufferAddedNotify &&addedNotify ) noexcept:
    _buffer ( std::move ( buffer ) ),
    _notify ( std::move ( addedNotify ) )
{
    // NOTHING
}

} // namespace editor
