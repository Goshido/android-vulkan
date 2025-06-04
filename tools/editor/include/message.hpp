#ifndef EDITOR_MESSAGE_HPP
#define EDITOR_MESSAGE_HPP


#include <GXCommon/GXWarning.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <cstdint>

GX_RESTORE_WARNING_STATE


namespace editor {

enum class eMessageType : uint32_t
{
    CaptureInput,
    ChangeCursor,
    CloseEditor,
    DPIChanged,
    FrameComplete,
    HelloTriangleReady,
    KeyboardKeyDown,
    KeyboardKeyUp,
    ModuleStopped,
    MouseHover,
    MouseButtonDown,
    MouseButtonUp,
    MouseMoved,
    RecreateSwapchain,
    ReleaseInput,
    RenderFrame,
    RunEventLoop,
    Shutdown,
    StartTimer,
    StartWidgetCaptureMouse,
    StopTimer,
    StopWidgetCaptureMouse,
    SwapchainCreated,
    Typing,
    UIAddWidget,
    UIAppendChildElement,
    UIDeleteElement,
    UIElementCreated,
    UIPrependChildElement,
    UIRemoveWidget,
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
