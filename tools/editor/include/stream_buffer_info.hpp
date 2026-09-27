#ifndef EDITOR_STREAM_BUFFER_INFO_HPP
#define EDITOR_STREAM_BUFFER_INFO_HPP


#include "stream_buffer_ref.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <functional>

GX_RESTORE_WARNING_STATE


namespace editor {

using StreamBufferAddedNotify = std::move_only_function<void ( StreamBufferRef )>;

class StreamBufferInfo final
{
    public:
        StreamBufferRef             _buffer {};
        StreamBufferAddedNotify     _notify = nullptr;

    public:
        StreamBufferInfo () = delete;

        StreamBufferInfo ( StreamBufferInfo const & ) = delete;
        StreamBufferInfo &operator = ( StreamBufferInfo const & ) = delete;

        StreamBufferInfo ( StreamBufferInfo && ) = default;
        StreamBufferInfo &operator = ( StreamBufferInfo && ) = default;

        explicit StreamBufferInfo ( StreamBufferRef buffer, StreamBufferAddedNotify &&addedNotify ) noexcept;

        ~StreamBufferInfo () = default;
};

} // namespace editor


#endif // EDITOR_STREAM_BUFFER_INFO_HPP
