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
    DPIChanged,
    FrameComplete,
    HelloTriangleReady,
    ModuleStopped,
    MouseMoved,
    RecreateSwapchain,
    RenderFrame,
    RunEventLoop,
    Shutdown,
    SwapchainCreated,
    VulkanInitReport,
    WindowVisibilityChanged,
    Unknown
};

class Message final
{
    public:
        using SerialNumber = uint32_t;

    public:
        eMessageType    _type;
        void*           _params;
        SerialNumber    _serialNumber = 0U;
};

} // namespace editor


#endif // EDITOR_MESSAGE_HPP
