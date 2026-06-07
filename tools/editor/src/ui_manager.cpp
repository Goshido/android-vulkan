#include <precompiled_headers.hpp>
#include <av_assert.hpp>
#include <font_storage.hpp>
#include <keyboard_key_event.hpp>
#include <logger.hpp>
#include <trace.hpp>
#include <ui_manager.hpp>
#include <ui_props.hpp>


namespace editor {

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
    pbr::FontStorage &fontStorage = FontStorage::Instance ();

    std::shared_lock const lock ( _mutex );

    for ( auto &widget : _widgets )
    {
        Widget::LayoutStatus const status = widget->ApplyLayout ( renderer, fontStorage );
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
    pbr::FontStorage &fontStorage = FontStorage::Instance ();

    for ( auto &widget : _widgets )
        _needRefill |= widget->UpdateCache ( fontStorage, viewport );

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
    messageQueue.EnqueueBack ( Message ( eMessageType::ModuleStarted ) );
    std::optional<Message::SerialNumber> lastRefund {};

    for ( ; ; )
    {
        AV_TRACE ( "Event loop" )
        Message message = messageQueue.DequeueBegin ( lastRefund );

        GX_DISABLE_WARNING ( 4061 )

        switch ( message._type )
        {
            case eMessageType::DoubleClick:
                OnDoubleClick ( messageQueue, std::move ( message ) );
            break;

            case eMessageType::KeyboardKeyDown:
                OnKeyboardKeyDown ( messageQueue, std::move ( message ) );
            break;

            case eMessageType::KeyboardKeyUp:
                OnKeyboardKeyUp ( messageQueue, std::move ( message ) );
            break;

            case eMessageType::KillFocus:
                OnKillFocus ( messageQueue );
            break;

            case eMessageType::MouseHover:
                OnMouseHover ( messageQueue, std::move ( message ) );
            break;

            case eMessageType::MouseButtonDown:
                OnMouseButtonDown ( messageQueue, std::move ( message ) );
            break;

            case eMessageType::MouseButtonUp:
                OnMouseButtonUp ( messageQueue, std::move ( message ) );
            break;

            case eMessageType::MouseMoved:
                OnMouseMoved ( messageQueue, std::move ( message ) );
            break;

            case eMessageType::ReadClipboardResponse:
                OnReadClipboardResponse ( messageQueue, std::move ( message ) );
            break;

            case eMessageType::SetFocus:
                OnSetFocus ( messageQueue, std::move ( message ) );
            break;

            case eMessageType::Shutdown:
                OnShutdown ( messageQueue, std::move ( message ) );
            return;

            case eMessageType::StartWidgetCaptureMouse:
                OnStartWidgetCaptureMouse ( messageQueue, std::move ( message ) );
            break;

            case eMessageType::StopWidgetCaptureMouse:
                OnStopWidgetCaptureMouse ( messageQueue );
            break;

            case eMessageType::Typing:
                OnTyping ( messageQueue, std::move ( message ) );
            break;

            case eMessageType::UIAddWidget:
                OnUIAddWidget ( messageQueue, std::move ( message ) );
            break;

            case eMessageType::UIRemoveWidget:
                OnUIRemoveWidget ( messageQueue, std::move ( message ) );
            break;

            default:
                lastRefund = message._serialNumber;
                messageQueue.DequeueEnd ( std::move ( message ), MessageQueue::eRefundLocation::Front );
            break;
        }

        GX_ENABLE_WARNING ( 4061 )
    }
}

void UIManager::OnDoubleClick ( MessageQueue &messageQueue, Message &&message ) noexcept
{
    AV_TRACE ( "Double click" )
    messageQueue.DequeueEnd ();

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

void UIManager::OnKeyboardKeyDown ( MessageQueue &messageQueue, Message &&message ) noexcept
{
    AV_TRACE ( "Keyboard key down" )
    messageQueue.DequeueEnd ();

    KeyboardKeyEvent const event ( message );

    if ( _typingCapture )
    {
        _typingCapture->OnKeyboardKeyDown ( event._key, event._modifier );
        return;
    }

    std::shared_lock const lock ( _mutex );

    for ( auto &widget : _widgets )
    {
        if ( Widget &w = *widget; w.IsOverlapped ( _lastMouseX, _lastMouseY ) )
        {
            w.OnKeyboardKeyDown ( event._key, event._modifier );
            break;
        }
    }
}

void UIManager::OnKeyboardKeyUp ( MessageQueue &messageQueue, Message &&message ) noexcept
{
    AV_TRACE ( "Keyboard key up" )
    messageQueue.DequeueEnd ();

    KeyboardKeyEvent const event ( message );

    if ( _typingCapture )
    {
        _typingCapture->OnKeyboardKeyUp ( event._key, event._modifier );
        return;
    }

    std::shared_lock const lock ( _mutex );

    for ( auto &widget : _widgets )
    {
        if ( Widget &w = *widget; w.IsOverlapped ( _lastMouseX, _lastMouseY ) )
        {
            w.OnKeyboardKeyUp ( event._key, event._modifier );
            break;
        }
    }
}

void UIManager::OnKillFocus ( MessageQueue &messageQueue ) noexcept
{
    AV_TRACE ( "Kill focus" )
    messageQueue.DequeueEnd ();
    _typingCapture = nullptr;
}

void UIManager::OnSetFocus ( MessageQueue &messageQueue, Message &&message ) noexcept
{
    AV_TRACE ( "Set focus" )
    messageQueue.DequeueEnd ();
    _typingCapture = static_cast<Widget*> ( message._action () );
}

void UIManager::OnMouseHover ( MessageQueue &messageQueue, Message &&message ) noexcept
{
    AV_TRACE ( "Mouse hover" )
    messageQueue.DequeueEnd ();

    auto* widget = static_cast<Widget*> ( message._action () );

    if ( ( _hoverWidget != nullptr ) & ( _hoverWidget != widget ) ) [[likely]]
        _hoverWidget->OnMouseLeave ();

    _hoverWidget = widget;
}

void UIManager::OnMouseButtonDown ( MessageQueue &messageQueue, Message &&message ) noexcept
{
    AV_TRACE ( "Mouse button down" )
    messageQueue.DequeueEnd ();

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

void UIManager::OnMouseButtonUp ( MessageQueue &messageQueue, Message &&message ) noexcept
{
    AV_TRACE ( "Mouse button up" )
    messageQueue.DequeueEnd ();

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

void UIManager::OnMouseMoved ( MessageQueue &messageQueue, Message &&message ) noexcept
{
    AV_TRACE ( "Mouse moved" )
    messageQueue.DequeueEnd ();

    auto const &event = *static_cast<MouseMoveEvent const*> ( message._action () );
    int32_t const x = event._x;
    int32_t const y = event._y;
    _lastMouseX = x;
    _lastMouseY = y;

    if ( _mouseCapture ) [[unlikely]]
    {
        _mouseCapture->OnMouseMove ( event );
        return;
    }

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
        Message ( eMessageType::ChangeCursor,
            [ value = std::bit_cast<void*> ( eCursor::Arrow ) ] () noexcept {
                return value;
            }
        )
    );
}

void UIManager::OnReadClipboardResponse ( MessageQueue &messageQueue, Message &&message ) noexcept
{
    AV_TRACE ( "Read clipboard response" )
    messageQueue.DequeueEnd ();

    if ( _typingCapture ) [[likely]]
    {
        _typingCapture->ApplyClipboard ( *static_cast<std::u32string const*> ( message._action () ) );
    }
}

void UIManager::OnShutdown ( MessageQueue &messageQueue, Message &&refund ) noexcept
{
    AV_TRACE ( "Shutdown" )
    messageQueue.DequeueEnd ( std::move ( refund ), MessageQueue::eRefundLocation::Front );

    {
        std::lock_guard const lock ( _mutex );
        _widgets.clear ();
    }

    messageQueue.EnqueueFront ( Message ( eMessageType::ModuleStopped ) );
}

void UIManager::OnStartWidgetCaptureMouse ( MessageQueue &messageQueue, Message &&message ) noexcept
{
    AV_TRACE ( "Start widget capture input" )
    messageQueue.DequeueEnd ();
    _mouseCapture = static_cast<Widget*> ( message._action () );
}

void UIManager::OnStopWidgetCaptureMouse ( MessageQueue &messageQueue ) noexcept
{
    AV_TRACE ( "Stop widget capture input" )
    messageQueue.DequeueEnd ();
    _mouseCapture = nullptr;
}

void UIManager::OnTyping ( MessageQueue &messageQueue, Message &&message ) noexcept
{
    AV_TRACE ( "Typing" )
    messageQueue.DequeueEnd ();

    if ( _typingCapture ) [[likely]]
    {
        _typingCapture->OnTyping ( static_cast<char32_t> ( std::bit_cast<size_t> ( message._action () ) ) );
    }
}

void UIManager::OnUIAddWidget ( MessageQueue &messageQueue, Message &&message ) noexcept
{
    AV_TRACE ( "Add widget" )
    messageQueue.DequeueEnd ();

    std::lock_guard const lock ( _mutex );
    _widgets.emplace_back ( static_cast<Widget*> ( message._action () ) );
}

void UIManager::OnUIRemoveWidget ( MessageQueue &messageQueue, Message &&message ) noexcept
{
    AV_TRACE ( "Remove widget" )
    messageQueue.DequeueEnd ();
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
