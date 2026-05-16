#include <precompiled_headers.hpp>
#include <av_assert.hpp>
#include <keyboard_key_event.hpp>
#include <logger.hpp>
#include <message_queue.hpp>
#include <trace.hpp>
#include <ui_manager.hpp>
#include <ui_props.hpp>


namespace editor {

UIManager::UIManager ( pbr::FontStorage &fontStorage ) noexcept:
    _fontStorage ( fontStorage )
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
        ._uiBufferStreams = *response
    };

    for ( auto &widget : _widgets )
    {
        widget->Submit ( info );
    }
}

void UIManager::EventLoop () noexcept
{
    MessageQueue &messageQueue = MessageQueue::Instance ();
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
    MessageQueue::Instance ().DequeueEnd ();

    auto const &event = *static_cast<MouseButtonEvent const*> ( message._action () );

    if ( _typingCapture ) [[unlikely]]
    {
        _typingCapture->OnDoubleClick ( event );
        return;
    }

    if ( _mouseCapture ) [[unlikely]]
    {
        _mouseCapture->OnDoubleClick ( event );
        return;
    }

    int32_t const x = event._x;
    int32_t const y = event._y;

    {
        std::shared_lock const lock ( _mutex );

        for ( auto &widget : _widgets )
        {
            if ( Widget &w = *widget; w.IsOverlapped ( x, y ) )
            {
                w.OnDoubleClick ( event );
                break;
            }
        }
    }
}

void UIManager::OnFontStorageReady () noexcept
{
    AV_TRACE ( "FontStorage ready" )
    MessageQueue &messageQueue = MessageQueue::Instance ();
    messageQueue.DequeueEnd ();

    auto* dialogBox = new UIProps ( _fontStorage );
    dialogBox->SetRect ( Rect ( 44, 444, 133, 333 ) );

    dialogBox->SetMinSize ( pbr::LengthValue ( pbr::LengthValue::eType::PX, 150.0F ),
        pbr::LengthValue ( pbr::LengthValue::eType::PX, 90.0F ) );

    messageQueue.EnqueueBack (
        {
            ._type = eMessageType::UIAddWidget,

            ._action = [ value = dialogBox ] () noexcept {
                return value;
            },

            ._serialNumber = 0U
        }
    );
}

void UIManager::OnKeyboardKeyDown ( Message &&message ) noexcept
{
    AV_TRACE ( "Keyboard key down" )
    MessageQueue::Instance ().DequeueEnd ();

    if ( !_typingCapture ) [[unlikely]]
        return;

    KeyboardKeyEvent const event ( message );
    _typingCapture->OnKeyboardKeyDown ( event._key, event._modifier );
}

void UIManager::OnKeyboardKeyUp ( Message &&message ) noexcept
{
    AV_TRACE ( "Keyboard key up" )
    MessageQueue::Instance ().DequeueEnd ();

    if ( !_typingCapture ) [[unlikely]]
        return;

    KeyboardKeyEvent const event ( message );
    _typingCapture->OnKeyboardKeyUp ( event._key, event._modifier );
}

void UIManager::OnKillFocus () noexcept
{
    AV_TRACE ( "Kill focus" )
    MessageQueue::Instance ().DequeueEnd ();
    _typingCapture = nullptr;
}

void UIManager::OnSetFocus ( Message &&message ) noexcept
{
    AV_TRACE ( "Set focus" )
    MessageQueue::Instance ().DequeueEnd ();
    _typingCapture = static_cast<Widget*> ( message._action () );
}

void UIManager::OnMouseHover ( Message &&message ) noexcept
{
    AV_TRACE ( "Mouse hover" )
    MessageQueue::Instance ().DequeueEnd ();

    auto* widget = static_cast<Widget*> ( message._action () );

    if ( ( _hoverWidget != nullptr ) & ( _hoverWidget != widget ) ) [[likely]]
        _hoverWidget->OnMouseLeave ();

    _hoverWidget = widget;
}

void UIManager::OnMouseButtonDown ( Message &&message ) noexcept
{
    AV_TRACE ( "Mouse button down" )
    MessageQueue::Instance ().DequeueEnd ();

    auto const &event = *static_cast<MouseButtonEvent const*> ( message._action () );

    if ( _typingCapture ) [[unlikely]]
    {
        _typingCapture->OnMouseButtonDown ( event );
        return;
    }

    if ( _mouseCapture ) [[unlikely]]
    {
        _mouseCapture->OnMouseButtonDown ( event );
        return;
    }

    int32_t const x = event._x;
    int32_t const y = event._y;

    {
        std::shared_lock const lock ( _mutex );

        for ( auto &widget : _widgets )
        {
            if ( Widget &w = *widget; w.IsOverlapped ( x, y ) )
            {
                w.OnMouseButtonDown ( event );
                break;
            }
        }
    }
}

void UIManager::OnMouseButtonUp ( Message &&message ) noexcept
{
    AV_TRACE ( "Mouse button up" )
    MessageQueue::Instance ().DequeueEnd ();

    auto const &event = *static_cast<MouseButtonEvent const*> ( message._action () );

    if ( _typingCapture ) [[unlikely]]
    {
        _typingCapture->OnMouseButtonUp ( event );
        return;
    }

    if ( _mouseCapture ) [[unlikely]]
    {
        _mouseCapture->OnMouseButtonUp ( event );
        return;
    }

    int32_t const x = event._x;
    int32_t const y = event._y;

    {
        std::shared_lock const lock ( _mutex );

        for ( auto &widget : _widgets )
        {
            Widget &w = *widget;

            if ( w.IsOverlapped ( x, y ) )
            {
                w.OnMouseButtonUp ( event );
                break;
            }
        }
    }
}

void UIManager::OnMouseMoved ( Message &&message ) noexcept
{
    AV_TRACE ( "Mouse moved" )
    MessageQueue &messageQueue = MessageQueue::Instance ();
    messageQueue.DequeueEnd ();

    auto const &event = *static_cast<MouseMoveEvent const*> ( message._action () );

    if ( _mouseCapture ) [[unlikely]]
    {
        _mouseCapture->OnMouseMove ( event );
        return;
    }

    int32_t const x = event._x;
    int32_t const y = event._y;

    {
        std::shared_lock const lock ( _mutex );

        for ( auto &widget : _widgets )
        {
            Widget &w = *widget;

            if ( w.IsOverlapped ( x, y ) )
            {
                w.OnMouseMove ( event );
                return;
            }
        }
    }

    size_t const eventID = event._eventID;

    if ( eventID - std::exchange ( _eventID, eventID ) <= 1U ) [[likely]]
        return;

    messageQueue.EnqueueBack (
        {
            ._type = eMessageType::ChangeCursor,

            ._action = [ value = std::bit_cast<void*> ( eCursor::Arrow ) ] () noexcept {
                return value;
            },

            ._serialNumber = 0U
        }
    );
}

void UIManager::OnReadClipboardResponse ( Message &&message ) noexcept
{
    AV_TRACE ( "Read clipboard response" )
    MessageQueue::Instance ().DequeueEnd ();

    if ( _typingCapture ) [[likely]]
    {
        _typingCapture->ApplyClipboard ( *static_cast<std::u32string const*> ( message._action () ) );
    }
}

void UIManager::OnShutdown ( Message &&refund ) noexcept
{
    AV_TRACE ( "Shutdown" )
    MessageQueue &messageQueue = MessageQueue::Instance ();
    messageQueue.DequeueEnd ( std::move ( refund ), MessageQueue::eRefundLocation::Front );

    {
        std::lock_guard const lock ( _mutex );
        _widgets.clear ();
    }

    messageQueue.EnqueueFront (
        {
            ._type = eMessageType::ModuleStopped,
            ._action = nullptr,
            ._serialNumber = 0U
        }
    );
}

void UIManager::OnStartWidgetCaptureMouse ( Message &&message ) noexcept
{
    AV_TRACE ( "Start widget capture input" )
    MessageQueue::Instance ().DequeueEnd ();
    _mouseCapture = static_cast<Widget*> ( message._action () );
}

void UIManager::OnStopWidgetCaptureMouse () noexcept
{
    AV_TRACE ( "Stop widget capture input" )
    MessageQueue::Instance ().DequeueEnd ();
    _mouseCapture = nullptr;
}

void UIManager::OnTyping ( Message &&message ) noexcept
{
    AV_TRACE ( "Typing" )
    MessageQueue::Instance ().DequeueEnd ();

    if ( _typingCapture ) [[likely]]
    {
        _typingCapture->OnTyping ( static_cast<char32_t> ( std::bit_cast<size_t> ( message._action () ) ) );
    }
}

void UIManager::OnUIAddWidget ( Message &&message ) noexcept
{
    AV_TRACE ( "Add widget" )
    MessageQueue::Instance ().DequeueEnd ();

    std::lock_guard const lock ( _mutex );
    _widgets.emplace_back ( static_cast<Widget*> ( message._action () ) );
}

void UIManager::OnUIRemoveWidget ( Message &&message ) noexcept
{
    AV_TRACE ( "Remove widget" )
    MessageQueue::Instance ().DequeueEnd ();
    auto const* widget = static_cast<Widget const*> ( message._action () );

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
