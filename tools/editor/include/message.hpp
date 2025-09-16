#ifndef EDITOR_MESSAGE_HPP
#define EDITOR_MESSAGE_HPP


#include <GXCommon/GXWarning.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <cstdint>

GX_RESTORE_WARNING_STATE


namespace editor {

enum class eMessageType : uint32_t
{
    CaptureKeyboard,
    CaptureMouse,
    ChangeCursor,
    CloseEditor,
    DoubleClick,
    DPIChanged,
    FontStorageReady,
    FrameComplete,
    HelloTriangleReady,
    KeyboardKeyDown,
    KeyboardKeyUp,
    KillFocus,
    ModuleStopped,
    MouseButtonDown,
    MouseButtonUp,
    MouseHover,
    MouseMoved,
    ReadClipboardRequest,
    ReadClipboardResponse,
    RecreateSwapchain,
    ReleaseKeyboard,
    ReleaseMouse,
    RenderFrame,
    RunEventLoop,
    SetFocus,
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

    // FUCK - remove it
    UIDeleteElementEXT,

    UIElementCreated,
    UIHideElement,

    // FUCK - remove it
    UIHideElementEXT,

    UIPrependChildElement,
    UIRemoveWidget,
    UISetText,
    UIShowElement,

    // FUCK - remove it
    UIShowElementEXT,

    UIUpdateElement,

    // FUCK - remove it
    UIUpdateElementEXT,

    VulkanInitReport,
    WindowVisibilityChanged,
    WriteClipboard,
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
