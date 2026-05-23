#ifndef EDITOR_MESSAGE_HPP
#define EDITOR_MESSAGE_HPP


#include <GXCommon/GXWarning.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <cstdint>
#include <functional>

GX_RESTORE_WARNING_STATE


namespace editor {

enum class eMessageType : uint32_t
{
    CaptureKeyboard,
    CaptureMouse,
    ChangeCursor,
    CloseEditor,
    DestroyMesh,
    DestroyTexture2D,
    DoubleClick,
    DPIChanged,
    FontStorageReady,
    FrameComplete,
    HelloTriangleReady,
    InvokeIO,
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
    UIElementCreated,
    UIHideElement,
    UIPrependChildElement,
    UIRemoveWidget,
    UISetText,
    UIShowElement,
    UIUpdateElement,
    UploadMesh,
    UploadTexture2D,
    VulkanInitReport,
    WindowVisibilityChanged,
    WriteClipboard,
    Unknown
};

struct Message final
{
    using SerialNumber = uint32_t;
    using Action = std::function<void* ()>;

    eMessageType    _type;
    Action          _action;
    SerialNumber    _serialNumber = 0U;
};

} // namespace editor


#endif // EDITOR_MESSAGE_HPP
