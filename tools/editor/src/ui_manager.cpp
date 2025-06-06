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
                OnStartWidgetCaptureInput ( std::move ( message ) );
            break;

            case eMessageType::StopWidgetCaptureMouse:
                OnStopWidgetCaptureInput ();
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

void UIManager::OnKeyboardKeyDown ( Message &&message ) noexcept
{
    AV_TRACE ( "Keyboard key down" )
    _messageQueue.DequeueEnd ();

    if ( !_inputCapture ) [[unlikely]]
        return;

    KeyboardKeyEvent const event ( message );
    _inputCapture->OnKeyboardKeyDown ( event._key, event._modifier );
}

void UIManager::OnKeyboardKeyUp ( Message &&message ) noexcept
{
    AV_TRACE ( "Keyboard key up" )
    _messageQueue.DequeueEnd ();

    if ( !_inputCapture ) [[unlikely]]
        return;

    KeyboardKeyEvent const event ( message );
    _inputCapture->OnKeyboardKeyUp ( event._key, event._modifier );
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

    if ( _inputCapture ) [[unlikely]]
    {
        _inputCapture->OnMouseButtonDown ( *event );
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

    if ( _inputCapture ) [[unlikely]]
    {
        _inputCapture->OnMouseButtonUp ( *event );
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

    if ( _inputCapture ) [[unlikely]]
    {
        _inputCapture->OnMouseMove ( *event );
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

void UIManager::OnStartWidgetCaptureInput ( Message &&message ) noexcept
{
    AV_TRACE ( "Start widget capture input" )
    _messageQueue.DequeueEnd ();
    _inputCapture = static_cast<Widget*> ( message._params );

    _messageQueue.EnqueueBack (
        {
            ._type = eMessageType::CaptureInput,
            ._params = nullptr,
            ._serialNumber = 0U
        }
    );
}

void UIManager::OnStopWidgetCaptureInput () noexcept
{
    AV_TRACE ( "Stop widget capture input" )
    _messageQueue.DequeueEnd ();
    _inputCapture = nullptr;

    _messageQueue.EnqueueBack (
        {
            ._type = eMessageType::ReleaseInput,
            ._params = nullptr,
            ._serialNumber = 0U
        }
    );
}

void UIManager::OnTyping ( Message &&message ) noexcept
{
    AV_TRACE ( "Typing" )
    _messageQueue.DequeueEnd ();

    if ( !_inputCapture ) [[unlikely]]
        return;

    // FUCK
    auto const fff = std::bit_cast<size_t> ( message._params );
    android_vulkan::LogDebug ( ">>> %llc", static_cast<char32_t> ( fff ) );
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
