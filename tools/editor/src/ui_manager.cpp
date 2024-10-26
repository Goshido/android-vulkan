#include <precompiled_headers.hpp>
#include <logger.hpp>
#include <mouse_move_event.hpp>
#include <trace.hpp>
#include <ui_dialog_box.hpp>
#include <ui_manager.hpp>


namespace editor {

void UIManager::Init ( MessageQueue &messageQueue ) noexcept
{
    AV_TRACE ( "UI: init" )
    _messageQueue = &messageQueue;

    _thread = std::thread (
        [ this ]() noexcept
        {
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
        _thread.join ();

    _messageQueue = nullptr;
}

void UIManager::RenderUI ( android_vulkan::Renderer &renderer, pbr::UIPass &pass ) noexcept
{
    AV_TRACE ( "UI" )

    pbr::FontStorage &fontStorage = pass.GetFontStorage ();
    bool needRefill = false;
    size_t neededUIVertices = 0U;

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
    auto* dialogBox = new UIDialogBox ();
    dialogBox->SetRect ( Rect ( 100, 500, 100, 300 ) );
    _widgets.push_back ( std::unique_ptr<Widget> ( dialogBox ) );
}

void UIManager::EventLoop () noexcept
{
    MessageQueue &messageQueue = *_messageQueue;
    std::optional<Message::SerialNumber> lastRefund {};

    for ( ; ; )
    {
        AV_TRACE ( "Event loop" )
        Message message = messageQueue.DequeueBegin ( lastRefund );

        GX_DISABLE_WARNING ( 4061 )

        switch ( message._type )
        {
            case eMessageType::MouseMoved:
                OnMouseMoved ( std::move ( message ) );
            break;

            case eMessageType::Shutdown:
                OnShutdown ( std::move ( message ) );
            return;

            default:
                lastRefund = message._serialNumber;
                messageQueue.DequeueEnd ( std::move ( message ) );
            break;
        }

        GX_ENABLE_WARNING ( 4061 )
    }
}

void UIManager::OnMouseMoved ( Message &&message ) noexcept
{
    AV_TRACE ( "Mouse moved" )
    _messageQueue->DequeueEnd ();

    auto const* event = static_cast<MouseMoveEvent const*> ( message._params );
    int32_t const x = event->_x;
    int32_t const y = event->_y;

    for ( auto& widget : _widgets )
    {
        Widget &w = *widget;

        if ( w.IsOverlapped ( x, y ) )
        {
            w.OnMouseMove ( *event );
            break;
        }
    }

    delete event;
}

void UIManager::OnShutdown ( Message &&refund ) noexcept
{
    AV_TRACE ( "Shutdown" )
    _messageQueue->DequeueEnd ( std::move ( refund ) );

    _messageQueue->EnqueueFront (
        Message
        {
            ._type = eMessageType::ModuleStopped,
            ._params = nullptr,
            ._serialNumber = 0U
        }
    );
}

} // namespace editor
