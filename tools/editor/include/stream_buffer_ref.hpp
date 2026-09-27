#ifndef EDITOR_STREAM_BUFFER_REF_HPP
#define EDITOR_STREAM_BUFFER_REF_HPP


#include <platform/windows/pbr/stream_buffer.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <memory>

GX_RESTORE_WARNING_STATE


namespace editor {

using StreamBufferRef = std::unique_ptr<pbr::StreamBuffer>;

} // namespace editor


#endif // EDITOR_STREAM_BUFFER_REF_HPP
