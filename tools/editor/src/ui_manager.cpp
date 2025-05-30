#include <precompiled_headers.hpp>
#include <av_assert.hpp>
#include <keyboard_key_event.hpp>
#include <logger.hpp>
#include <mouse_key_event.hpp>
#include <mouse_move_event.hpp>
#include <trace.hpp>
#include <ui_manager.hpp>
#include <ui_props.hpp>


namespace editor {

UIManager::UIManager ( MessageQueue &messageQueue ) noexcept:
    _messageQueue ( messageQueue )
{
    // NOTHING
}

void UIManager::Init () noexcept
{
    AV_TRACE ( "UI: init" )

    _thread = std::thread (
        [ this ]() noexcept {
            AV_THREAD_NAME ( "UI" )
            CreateWidgets ();
            EventLoop ();
        }
    );
}

void UIManager::Destroy () noexcept
{
    AV_TRACE ( "UI: destroy" )

    if ( _thread.joinable () ) [[likely]]
    {
        _thread.join ();
    }
}

void UIManager::RenderUI ( android_vulkan::Renderer &renderer, pbr::UIPass &pass ) noexcept
{
    AV_TRACE ( "UI" )

    pbr::FontStorage &fontStorage = pass.GetFontStorage ();
    bool needRefill = false;
    size_t neededUIVertices = 0U;

    std::shared_lock const lock ( _mutex );

    for ( auto &widget : _widgets )
    {
        Widget::LayoutStatus const status = widget->ApplyLayout ( renderer, fontStorage );
        needRefill |= status._hasChanges;
        neededUIVertices += status._neededUIVertices;
    }

    if ( neededUIVertices == 0U )
    {
        pass.RequestEmptyUI ();
        return;
    }

    VkExtent2D const &viewport = renderer.GetViewportResolution ();

    for ( auto &widget : _widgets )
        needRefill |= widget->UpdateCache ( fontStorage, viewport );

    if ( !needRefill )
        return;

    pbr::UIPass::UIBufferResponse response = pass.RequestUIBuffer ( neededUIVertices );

    if ( !response )
    {
        pass.RequestEmptyUI ();
        return;
    }

    pbr::UIElement::SubmitInfo info
    {
        ._uiPass = &pass,
        ._vertexBuffer = *response
    };

    for ( auto &widget : _widgets )
    {
        widget->Submit ( info );
    }
}

void UIManager::CreateWidgets () noexcept
{
    auto* dialogBox = new UIProps ( _messageQueue );
    dialogBox->SetRect ( Rect ( 44, 444, 133, 333 ) );

    dialogBox->SetMinSize ( pbr::LengthValue ( pbr::LengthValue::eType::PX, 150.0F ),
        pbr::LengthValue ( pbr::LengthValue::eType::PX, 90.0F ) );

    _messageQueue.EnqueueBack (
        {
            ._type = eMessageType::UIAddWidget,
            ._params = dialogBox,
            ._serialNumber = 0U
        }
    );
}

void UIManager::EventLoop () noexcept
{
    MessageQueue &messageQueue = _messageQueue;
    std::optional<Message::SerialNumber> lastRefund {};

    for ( ; ; )
    {
        AV_TRACE ( "Event loop" )
        Message message = messageQueue.DequeueBegin ( lastRefund );

        GX_DISABLE_WARNING ( 4061 )

        switch ( message._type )
        {
            case eMessageType::KeyboardKeyDown:
                OnKeyboardKeyDown ( std::move ( message ) );
            break;

            case eMessageType::KeyboardKeyUp:
                OnKeyboardKeyUp ( std::move ( message ) );
            break;

            case eMessageType::MouseHover:
                OnMouseHover ( std::move ( message ) );
            break;

            case eMessageType::MouseButtonDown:
                OnMouseButtonDown ( std::move ( message ) );
            break;

            case eMessageType::MouseButtonUp:
                OnMouseButtonUp ( std::move ( message ) );
            break;

            case eMessageType::MouseMoved:
                OnMouseMoved ( std::move ( message ) );
            break;

            case eMessageType::Shutdown:
                OnShutdown ( std::move ( message ) );
            return;

            case eMessageType::StartWidgetCaptureMouse:
                OnStartWidgetCaptureMouse ( std::move ( message ) );
            break;

            case eMessageType::StopWidgetCaptureMouse:
                OnStopWidgetCaptureMouse ();
            break;

            case eMessageType::UIAddWidget:
                OnUIAddWidget ( std::move ( message ) );
            break;

            case eMessageType::UIRemoveWidget:
                OnUIRemoveWidget ( std::move ( message ) );
            break;

            default:
                lastRefund = message._serialNumber;
                messageQueue.DequeueEnd ( std::move ( message ), MessageQueue::eRefundLocation::Front );
            break;
        }

        GX_ENABLE_WARNING ( 4061 )
    }
}

static std::unordered_map<eKey, std::string_view> g_fuck =
{
    { eKey::LeftMouseButton, "LeftMouseButton" },
    { eKey::MiddleMouseButton, "MiddleMouseButton" },
    { eKey::RightMouseButton, "RightMouseButton" },

    { eKey::Key0, "Key0" },
    { eKey::Key1, "Key1" },
    { eKey::Key2, "Key2" },
    { eKey::Key3, "Key3" },
    { eKey::Key4, "Key4" },
    { eKey::Key5, "Key5" },
    { eKey::Key6, "Key6" },
    { eKey::Key7, "Key7" },
    { eKey::Key8, "Key8" },
    { eKey::Key9, "Key9" },

    { eKey::KeyF1, "KeyF1" },
    { eKey::KeyF2, "KeyF2" },
    { eKey::KeyF3, "KeyF3" },
    { eKey::KeyF4, "KeyF4" },
    { eKey::KeyF5, "KeyF5" },
    { eKey::KeyF6, "KeyF6" },
    { eKey::KeyF7, "KeyF7" },
    { eKey::KeyF8, "KeyF8" },
    { eKey::KeyF9, "KeyF9" },
    { eKey::KeyF10, "KeyF10" },
    { eKey::KeyF11, "KeyF11" },
    { eKey::KeyF12, "KeyF12" },

    { eKey::KeyA, "KeyA" },
    { eKey::KeyB, "KeyB" },
    { eKey::KeyC, "KeyC" },
    { eKey::KeyD, "KeyD" },
    { eKey::KeyE, "KeyE" },
    { eKey::KeyF, "KeyF" },
    { eKey::KeyG, "KeyG" },
    { eKey::KeyH, "KeyH" },
    { eKey::KeyI, "KeyI" },
    { eKey::KeyJ, "KeyJ" },
    { eKey::KeyK, "KeyK" },
    { eKey::KeyL, "KeyL" },
    { eKey::KeyM, "KeyM" },
    { eKey::KeyN, "KeyN" },
    { eKey::KeyO, "KeyO" },
    { eKey::KeyP, "KeyP" },
    { eKey::KeyQ, "KeyQ" },
    { eKey::KeyR, "KeyR" },
    { eKey::KeyS, "KeyS" },
    { eKey::KeyT, "KeyT" },
    { eKey::KeyU, "KeyU" },
    { eKey::KeyV, "KeyV" },
    { eKey::KeyW, "KeyW" },
    { eKey::KeyX, "KeyX" },
    { eKey::KeyY, "KeyY" },
    { eKey::KeyZ, "KeyZ" },

    { eKey::KeyDown, "KeyDown" },
    { eKey::KeyLeft, "KeyLeft" },
    { eKey::KeyRight, "KeyRight" },
    { eKey::KeyUp, "KeyUp" },

    { eKey::KeyLeftSquareBracket, "KeyLeftSquareBracket" },
    { eKey::KeyRightSquareBracket, "KeyRightSquareBracket" },

    { eKey::KeyNumpad0, "KeyNumpad0" },
    { eKey::KeyNumpad1, "KeyNumpad1" },
    { eKey::KeyNumpad2, "KeyNumpad2" },
    { eKey::KeyNumpad3, "KeyNumpad3" },
    { eKey::KeyNumpad4, "KeyNumpad4" },
    { eKey::KeyNumpad5, "KeyNumpad5" },
    { eKey::KeyNumpad6, "KeyNumpad6" },
    { eKey::KeyNumpad7, "KeyNumpad7" },
    { eKey::KeyNumpad8, "KeyNumpad8" },
    { eKey::KeyNumpad9, "KeyNumpad9" },
    { eKey::KeyNumpadAdd, "KeyNumpadAdd" },
    { eKey::KeyNumpadDiv, "KeyNumpadDiv" },
    { eKey::KeyNumpadDot, "KeyNumpadDot" },
    { eKey::KeyNumpadMinus, "KeyNumpadMinus" },
    { eKey::KeyNumpadMul, "KeyNumpadMul" },

    { eKey::KeyAlt, "KeyAlt" },
    { eKey::KeyApostrophe, "KeyApostrophe" },
    { eKey::KeyBackslash, "KeyBackslash" },
    { eKey::KeyBackspace, "KeyBackspace" },
    { eKey::KeyCapsLock, "KeyCapsLock" },
    { eKey::KeyComma, "KeyComma" },
    { eKey::KeyCtrl, "KeyCtrl" },
    { eKey::KeyDel, "KeyDel" },
    { eKey::KeyEnd, "KeyEnd" },
    { eKey::KeyEnter, "KeyEnter" },
    { eKey::KeyEsc, "KeyEsc" },
    { eKey::KeyHome, "KeyHome" },
    { eKey::KeyIns, "KeyIns" },
    { eKey::KeyMenu, "KeyMenu" },
    { eKey::KeyMinus, "KeyMinus" },
    { eKey::KeyPause, "KeyPause" },
    { eKey::KeyPeriod, "KeyPeriod" },
    { eKey::KeyPgDown, "KeyPgDown" },
    { eKey::KeyPgUp, "KeyPgUp" },
    { eKey::KeyPlus, "KeyPlus" },
    { eKey::KeySemicolon, "KeySemicolon" },
    { eKey::KeyShift, "KeyShift" },
    { eKey::KeySlash, "KeySlash" },
    { eKey::KeySpace, "KeySpace" },
    { eKey::KeyTab, "KeyTab" },
    { eKey::KeyTilde, "KeyTilde" }
};

void UIManager::OnKeyboardKeyDown ( Message &&message ) noexcept
{
    AV_TRACE ( "Keyboard key down" )
    _messageQueue.DequeueEnd ();

    // FUCK - _mouseCapture and keyboard is same thing

    //if ( !_mouseCapture ) [[unlikely]]
    //    return;

    KeyboardKeyEvent const event ( message );
    auto const key = g_fuck.find ( event._key );
    AV_ASSERT ( key != g_fuck.cend () )

    constexpr auto bbb = [] ( bool v ) noexcept -> char const* {
        return v ? "true" : "false";
    };

    constexpr char const format[] = R"__(>>> DOWN: %s
    Left Alt: %s,
    Right Alt: %s,
    Left Ctrl: %s,
    Right Ctrl: %s,
    Left Shift: %s,
    Right Shift: %s,
    Any Alt: %s,
    Any Ctrl: %s,
    Any Shift: %s)__";

    android_vulkan::LogDebug ( format,
        key->second.data (),
        bbb ( event._modifier._leftAlt ),
        bbb ( event._modifier._rightAlt ),
        bbb ( event._modifier._leftCtrl ),
        bbb ( event._modifier._rightCtrl ),
        bbb ( event._modifier._leftShift ),
        bbb ( event._modifier._rightShift ),
        bbb ( event._modifier.AnyAltPressed () ),
        bbb ( event._modifier.AnyCtrlPressed () ),
        bbb ( event._modifier.AnyShiftPressed () )
    );
}

void UIManager::OnKeyboardKeyUp ( Message &&message ) noexcept
{
    AV_TRACE ( "Keyboard key up" )
    _messageQueue.DequeueEnd ();

    // FUCK - _mouseCapture and keyboard is same thing

    /*if ( !_mouseCapture ) [[unlikely]]
        return;*/

    KeyboardKeyEvent const event ( message );
}

void UIManager::OnMouseHover ( Message &&message ) noexcept
{
    AV_TRACE ( "Mouse hover" )
    _messageQueue.DequeueEnd ();

    auto* widget = static_cast<Widget*> ( message._params );

    if ( ( _hoverWidget != nullptr ) & ( _hoverWidget != widget ) ) [[likely]]
        _hoverWidget->OnMouseLeave ();

    _hoverWidget = widget;
}

void UIManager::OnMouseButtonDown ( Message &&message ) noexcept
{
    AV_TRACE ( "Mouse button down" )
    _messageQueue.DequeueEnd ();

    auto const* event = static_cast<MouseKeyEvent const*> ( message._params );

    if ( _mouseCapture ) [[unlikely]]
    {
        _mouseCapture->OnMouseButtonDown ( *event );
        delete event;
        return;
    }

    int32_t const x = event->_x;
    int32_t const y = event->_y;

    {
        std::shared_lock const lock ( _mutex );

        for ( auto& widget : _widgets )
        {
            Widget &w = *widget;

            if ( w.IsOverlapped ( x, y ) )
            {
                w.OnMouseButtonDown ( *event );
                break;
            }
        }
    }

    delete event;
}

void UIManager::OnMouseButtonUp ( Message &&message ) noexcept
{
    AV_TRACE ( "Mouse button up" )
    _messageQueue.DequeueEnd ();

    auto const* event = static_cast<MouseKeyEvent const*> ( message._params );

    if ( _mouseCapture ) [[unlikely]]
    {
        _mouseCapture->OnMouseButtonUp ( *event );
        delete event;
        return;
    }

    int32_t const x = event->_x;
    int32_t const y = event->_y;

    {
        std::shared_lock const lock ( _mutex );

        for ( auto &widget : _widgets )
        {
            Widget &w = *widget;

            if ( w.IsOverlapped ( x, y ) )
            {
                w.OnMouseButtonUp ( *event );
                break;
            }
        }
    }

    delete event;
}

void UIManager::OnMouseMoved ( Message &&message ) noexcept
{
    AV_TRACE ( "Mouse moved" )
    _messageQueue.DequeueEnd ();

    auto const* event = static_cast<MouseMoveEvent const*> ( message._params );

    if ( _mouseCapture ) [[unlikely]]
    {
        _mouseCapture->OnMouseMove ( *event );
        delete event;
        return;
    }

    int32_t const x = event->_x;
    int32_t const y = event->_y;

    {
        std::shared_lock const lock ( _mutex );

        for ( auto &widget : _widgets )
        {
            Widget &w = *widget;

            if ( w.IsOverlapped ( x, y ) )
            {
                w.OnMouseMove ( *event );
                delete event;
                return;
            }
        }
    }

    size_t const eventID = event->_eventID;

    if ( eventID - std::exchange( _eventID, eventID ) > 1U ) [[unlikely]]
    {
        _messageQueue.EnqueueBack (
            {
                ._type = eMessageType::ChangeCursor,
                ._params = reinterpret_cast<void*> ( eCursor::Arrow ),
                ._serialNumber = 0U
            }
        );
    }

    delete event;
}

void UIManager::OnShutdown ( Message &&refund ) noexcept
{
    AV_TRACE ( "Shutdown" )
    _messageQueue.DequeueEnd ( std::move ( refund ), MessageQueue::eRefundLocation::Front );

    {
        std::lock_guard const lock ( _mutex );
        _widgets.clear ();
    }

    _messageQueue.EnqueueFront (
        {
            ._type = eMessageType::ModuleStopped,
            ._params = nullptr,
            ._serialNumber = 0U
        }
    );
}

void UIManager::OnStartWidgetCaptureMouse ( Message &&message ) noexcept
{
    AV_TRACE ( "Start widget capture mouse" )
    _messageQueue.DequeueEnd ();
    _mouseCapture = static_cast<Widget*> ( message._params );

    _messageQueue.EnqueueBack (
        {
            ._type = eMessageType::CaptureMouse,
            ._params = nullptr,
            ._serialNumber = 0U
        }
    );
}

void UIManager::OnStopWidgetCaptureMouse () noexcept
{
    AV_TRACE ( "Stop widget capture mouse" )
    _messageQueue.DequeueEnd ();
    _mouseCapture = nullptr;

    _messageQueue.EnqueueBack (
        {
            ._type = eMessageType::ReleaseMouse,
            ._params = nullptr,
            ._serialNumber = 0U
        }
    );
}

void UIManager::OnUIAddWidget ( Message &&message ) noexcept
{
    AV_TRACE ( "Add widget" )
    _messageQueue.DequeueEnd ();

    std::lock_guard const lock ( _mutex );
    _widgets.emplace_back ( static_cast<Widget*> ( message._params ) );
}

void UIManager::OnUIRemoveWidget ( Message &&message ) noexcept
{
    AV_TRACE ( "Remove widget" )
    _messageQueue.DequeueEnd ();
    auto const* widget = static_cast<Widget const*> ( message._params );

    std::lock_guard const lock ( _mutex );
    auto const end = _widgets.cend ();

    auto const findResult = std::find_if ( _widgets.cbegin (),
        end,

        [ widget ] ( std::unique_ptr<Widget> const &w ) noexcept -> bool {
            return w.get () == widget;
        }
    );

    if ( findResult != end ) [[likely]]
    {
        _widgets.erase ( findResult );
        return;
    }

    android_vulkan::LogWarning ( "UIManager::OnUIRemoveWidget - Can't find widget!" );
    AV_ASSERT ( false );
}

} // namespace editor
