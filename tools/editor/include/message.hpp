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
    DestroyProgram,
    DestroyStreamBuffer,
    DestroyTexture2D,
    DoubleClick,
    DPIChanged,
    FrameComplete,
    InvokeIO,
    InvokeRenderSession,
    InvokeUI,
    KeyboardKeyDown,
    KeyboardKeyUp,
    KillFocus,
    ModuleStarted,
    ModuleStopped,
    MouseButtonDown,
    MouseButtonUp,
    MouseHover,
    MouseMoved,
    NewProgram,
    NewStreamBuffer,
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
    StopIO,
    StopTimer,
    StopWidgetCaptureMouse,
    SwapchainCreated,
    Typing,
    UIAppendChildElement,
    UIAppendWidget,
    UIDeleteElement,
    UIElementCreated,
    UIHideElement,
    UIPrependChildElement,
    UIPrependWidget,
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
    public:
        using SerialNumber = uint32_t;
        using Action = std::move_only_function<void* ()>;

    public:
        eMessageType    _type;
        Action          _action;
        SerialNumber    _serialNumber = 0U;

    public:
        Message () = default;

        Message ( Message const & ) = delete;
        Message &operator = ( Message const & ) = delete;

        Message ( Message && ) = default;
        Message &operator = ( Message && ) = default;

        explicit Message ( eMessageType type ) noexcept;
        explicit Message ( eMessageType type, Action&& action ) noexcept;
        explicit Message ( eMessageType type, Action&& action, SerialNumber serialNumber ) noexcept;

        ~Message () = default;
};

} // namespace editor


#endif // EDITOR_MESSAGE_HPP
