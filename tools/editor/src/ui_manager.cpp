#include <precompiled_headers.hpp>
#include <av_assert.hpp>
#include <keyboard_key_event.hpp>
#include <logger.hpp>
#include <trace.hpp>
#include <ui_manager.hpp>
#include <ui_props.hpp>


namespace editor {

UIManager::UIManager ( MessageQueue &messageQueue, pbr::FontStorage &fontStorage ) noexcept:
    _fontStorage ( fontStorage ),
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

void UIManager::ComputeLayout ( android_vulkan::Renderer &renderer, pbr::UIPass &pass ) noexcept
{
    AV_TRACE ( "Compute UI layout" )

    // FUCK - remove namespace
    _needRefill = false;
    _neededUIVertices = 0U;

    std::shared_lock const lock ( _mutex );

    for ( auto &widget : _widgets )
    {
        Widget::LayoutStatus const status = widget->ApplyLayout ( renderer, _fontStorage );
        _needRefill |= status._hasChanges;
        _neededUIVertices += status._neededUIVertices;
    }

    if ( _neededUIVertices == 0U )
    {
        pass.RequestEmptyUI ();
    }
}

void UIManager::Submit ( android_vulkan::Renderer &renderer, pbr::UIPass &pass ) noexcept
{
    if ( !_neededUIVertices )
        return;

    AV_TRACE ( "Submit UI" )

    VkExtent2D const &viewport = renderer.GetViewportResolution ();

    for ( auto &widget : _widgets )
        _needRefill |= widget->UpdateCache ( _fontStorage, viewport );

    if ( !_needRefill )
        return;

    pbr::UIPass::UIBufferResponse response = pass.RequestUIBuffer ( _neededUIVertices );

    if ( !response )
    {
        pass.RequestEmptyUI ();
        return;
    }

    pbr::UIElement::SubmitInfo info
    {
        ._uiPass = &pass,
        ._uiVertexBuffer = *response
    };

    for ( auto &widget : _widgets )
    {
        widget->Submit ( info );
    }
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
            case eMessageType::DoubleClick:
                OnDoubleClick ( std::move ( message ) );
            break;

            case eMessageType::FontStorageReady:
                OnFontStorageReady ();
            break;

            case eMessageType::KeyboardKeyDown:
                OnKeyboardKeyDown ( std::move ( message ) );
            break;

            case eMessageType::KeyboardKeyUp:
                OnKeyboardKeyUp ( std::move ( message ) );
            break;

            case eMessageType::KillFocus:
                OnKillFocus ();
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

            case eMessageType::ReadClipboardResponse:
                OnReadClipboardResponse ( std::move ( message ) );
            break;

            case eMessageType::SetFocus:
                OnSetFocus ( std::move ( message ) );
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

            case eMessageType::Typing:
                OnTyping ( std::move ( message ) );
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

void UIManager::OnDoubleClick ( Message &&message ) noexcept
{
    AV_TRACE ( "Double click" )
    _messageQueue.DequeueEnd ();

    auto const* event = static_cast<MouseButtonEvent const*> ( message._params );

    if ( _typingCapture ) [[unlikely]]
    {
        _typingCapture->OnDoubleClick ( *event );
        delete event;
        return;
    }

    if ( _mouseCapture ) [[unlikely]]
    {
        _mouseCapture->OnDoubleClick ( *event );
        delete event;
        return;
    }

    int32_t const x = event->_x;
    int32_t const y = event->_y;

    {
        std::shared_lock const lock ( _mutex );

        for ( auto &widget : _widgets )
        {
            if ( Widget &w = *widget; w.IsOverlapped ( x, y ) )
            {
                w.OnDoubleClick ( *event );
                break;
            }
        }
    }

    delete event;
}

void UIManager::OnFontStorageReady () noexcept
{
    AV_TRACE ( "FontStorage ready" )
    _messageQueue.DequeueEnd ();

    auto* dialogBox = new UIProps ( _messageQueue, _fontStorage );
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

void UIManager::OnKeyboardKeyDown ( Message &&message ) noexcept
{
    AV_TRACE ( "Keyboard key down" )
    _messageQueue.DequeueEnd ();

    if ( !_typingCapture ) [[unlikely]]
        return;

    KeyboardKeyEvent const event ( message );
    _typingCapture->OnKeyboardKeyDown ( event._key, event._modifier );
}

void UIManager::OnKeyboardKeyUp ( Message &&message ) noexcept
{
    AV_TRACE ( "Keyboard key up" )
    _messageQueue.DequeueEnd ();

    if ( !_typingCapture ) [[unlikely]]
        return;

    KeyboardKeyEvent const event ( message );
    _typingCapture->OnKeyboardKeyUp ( event._key, event._modifier );
}

void UIManager::OnKillFocus () noexcept
{
    AV_TRACE ( "Kill focus" )
    _messageQueue.DequeueEnd ();
    _typingCapture = nullptr;
}

void UIManager::OnSetFocus ( Message &&message ) noexcept
{
    AV_TRACE ( "Set focus" )
    _messageQueue.DequeueEnd ();
    _typingCapture = static_cast<Widget*> ( message._params );
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

    auto const* event = static_cast<MouseButtonEvent const*> ( message._params );

    if ( _typingCapture ) [[unlikely]]
    {
        _typingCapture->OnMouseButtonDown ( *event );
        delete event;
        return;
    }

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

        for ( auto &widget : _widgets )
        {
            if ( Widget &w = *widget; w.IsOverlapped ( x, y ) )
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

    auto const* event = static_cast<MouseButtonEvent const*> ( message._params );

    if ( _typingCapture ) [[unlikely]]
    {
        _typingCapture->OnMouseButtonUp ( *event );
        delete event;
        return;
    }

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
    delete event;

    if ( eventID - std::exchange ( _eventID, eventID ) <= 1U ) [[likely]]
        return;

    _messageQueue.EnqueueBack (
        {
            ._type = eMessageType::ChangeCursor,
            ._params = std::bit_cast<void*> ( eCursor::Arrow ),
            ._serialNumber = 0U
        }
    );
}

void UIManager::OnReadClipboardResponse ( Message &&message ) noexcept
{
    AV_TRACE ( "Read clipboard response" )
    _messageQueue.DequeueEnd ();
    auto const* text = static_cast<std::u32string const*> ( message._params );

    if ( _typingCapture ) [[likely]]
        _typingCapture->ApplyClipboard ( *text );

    delete text;
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
    AV_TRACE ( "Start widget capture input" )
    _messageQueue.DequeueEnd ();
    _mouseCapture = static_cast<Widget*> ( message._params );
}

void UIManager::OnStopWidgetCaptureMouse () noexcept
{
    AV_TRACE ( "Stop widget capture input" )
    _messageQueue.DequeueEnd ();
    _mouseCapture = nullptr;
}

void UIManager::OnTyping ( Message &&message ) noexcept
{
    AV_TRACE ( "Typing" )
    _messageQueue.DequeueEnd ();

    if ( _typingCapture ) [[likely]]
    {
        _typingCapture->OnTyping ( static_cast<char32_t> ( std::bit_cast<size_t> ( message._params ) ) );
    }
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
