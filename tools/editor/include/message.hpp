#ifndef EDITOR_MESSAGE_HPP
#define EDITOR_MESSAGE_HPP


#include <GXCommon/GXWarning.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <cstdint>

GX_RESTORE_WARNING_STATE


namespace editor {

enum class eMessageType : uint32_t
{
    CaptureMouse,
    ChangeCursor,
    CloseEditor,
    DPIChanged,
    FrameComplete,
    HelloTriangleReady,
    ModuleStopped,
    MouseHover,
    MouseKeyDown,
    MouseKeyUp,
    MouseMoved,
    RecreateSwapchain,
    ReleaseMouse,
    RenderFrame,
    RunEventLoop,
    Shutdown,
    StartWidgetCaptureMouse,
    StopWidgetCaptureMouse,
    SwapchainCreated,
    UIAppendChildElement,
    UIDeleteElement,
    UIElementCreated,
    UIPrependChildElement,
    UISetText,
    VulkanInitReport,
    WindowVisibilityChanged,
    Unknown
};

struct Message final
{
    using SerialNumber = uint32_t;

    eMessageType    _type;
    void*           _params;
    SerialNumber    _serialNumber = 0U;
};

} // namespace editor


#endif // EDITOR_MESSAGE_HPP
