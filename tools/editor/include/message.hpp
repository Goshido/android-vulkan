#ifndef EDITOR_MESSAGE_HPP
#define EDITOR_MESSAGE_HPP


#include <GXCommon/GXWarning.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <cstdint>

GX_RESTORE_WARNING_STATE


namespace editor {

enum class eMessageType : uint32_t
{
    CloseEditor,
    RunEventLoop,
    RenderFrame,
    Shutdown,
    WindowVisibilityChanged,
    VulkanInitReport,
    Unknown
};

struct Message final
{
    eMessageType    _type;
    void*           _params;
};

} // namespace editor


#endif // EDITOR_MESSAGE_HPP
