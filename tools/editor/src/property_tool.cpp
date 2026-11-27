#include <precompiled_headers.hpp>
#include <property_tool.hpp>
#include <trace.hpp>
#include <workspace.hpp>


namespace editor {

namespace {

constexpr std::string_view CONFIG_KEY_SECTION = "property dialog";

constexpr std::string_view CONFIG_KEY_UI = "UI";
pbr::LengthValue const MIN_WIDTH ( pbr::LengthValue::eType::PX, 255.0F );
pbr::LengthValue const MIN_HEIGHT ( pbr::LengthValue::eType::PX, 70.0F );
pbr::LengthValue const DEFAULT_X ( pbr::LengthValue::eType::PX, 22.0F );
pbr::LengthValue const DEFAULT_Y ( pbr::LengthValue::eType::PX, 218.0F );
pbr::LengthValue const DEFAULT_WIDTH ( pbr::LengthValue::eType::PX, 300.0F );
pbr::LengthValue const DEFAULT_HEIGHT ( pbr::LengthValue::eType::PX, 200.0F );

constexpr std::string_view CONFIG_KEY_SHOW = "show";
constexpr bool DEFAULT_SHOW = true;

//----------------------------------------------------------------------------------------------------------------------

class DeleteComponentAction final : public Action
{
    public:
        struct Item final
        {
            Actor*                  _actor = nullptr;
            Actor::ComponentInfo    _componentInfo {};
            Component const*        _key = nullptr;
        };

        using Items = std::vector<Item>;

    private:
        Items               _items {};

    public:
        DeleteComponentAction () = delete;

        DeleteComponentAction ( DeleteComponentAction const & ) = delete;
        DeleteComponentAction &operator = ( DeleteComponentAction const & ) = delete;

        DeleteComponentAction ( DeleteComponentAction && ) = default;
        DeleteComponentAction &operator = ( DeleteComponentAction && ) = default;

        explicit DeleteComponentAction ( Items &&items ) noexcept;

        ~DeleteComponentAction () override = default;

    private:
        void Redo () noexcept override;
        void Undo () noexcept override;
};

DeleteComponentAction::DeleteComponentAction ( Items &&items ) noexcept:
    _items ( std::move ( items ) )
{
    // NOTHING
}

void DeleteComponentAction::Redo () noexcept
{
    AV_TRACE ( "Redo delete component" )

    for ( Item &item : _items )
        item._componentInfo = item._actor->Detach ( *item._key );

    Workspace::Instance ().OnContentCommitted ();
}

void DeleteComponentAction::Undo () noexcept
{
    AV_TRACE ( "Undo delete component" )
    auto const end = _items.rend ();

    for ( auto item = _items.rbegin (); item != end; ++item )
        ( *item )._actor->Attach ( std::move ( item->_componentInfo ) );

    Workspace::Instance ().OnContentCommitted ();
}

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

PropertyTool* PropertyTool::_instance = nullptr;

PropertyTool::PropertyTool () noexcept
{
    _instance = this;
}

void PropertyTool::Init ( SaveState::Container const &save ) noexcept
{
    AV_TRACE ( "Init property tool" )

    _toggle = Hotkey ( eKey::KeyF1,
        false,
        false,
        false,

        [ this ] () noexcept {
            if ( !_ui )
            {
                CreateUI ( Workspace::Instance ().GetSelection ().GetActors () );
                return;
            }

            _ui->GetRect ( _rect );

            MessageQueue::Instance ().EnqueueBack (
                Message ( eMessageType::UIRemoveWidget,
                    [ ui = std::exchange ( _ui, nullptr ) ] () noexcept {
                        return ui;
                    }
                )
            );
        }
    );

    GXVec4 const beta ( DEFAULT_X.GetValue (),
        DEFAULT_X.GetValue () + DEFAULT_WIDTH.GetValue (),
        DEFAULT_Y.GetValue (),
        DEFAULT_Y.GetValue () + DEFAULT_HEIGHT.GetValue ()
    );

    GXVec4 zeta {};
    zeta.Multiply ( beta, pbr::CSSUnitToDevicePixel::GetInstance ()._fromPX );
    Rect const defaultUI ( zeta );

    SaveState::Container const &root = save.ReadContainer ( CONFIG_KEY_SECTION );
    SaveState::Container const &ui = root.ReadArray ( CONFIG_KEY_UI );

    int32_t const left = ui.Read ( defaultUI._left );
    int32_t const right = ui.Read ( defaultUI._right );
    int32_t const top = ui.Read ( defaultUI._top );
    int32_t const bottom = ui.Read ( defaultUI._bottom );
    _rect = Rect ( left, right, top, bottom );

    if ( root.Read ( CONFIG_KEY_SHOW, DEFAULT_SHOW ) )
    {
        CreateUI ( Workspace::Instance ().GetSelection ().GetActors () );
    }
}

void PropertyTool::Destroy ( SaveState::Container &save ) noexcept
{
    AV_TRACE ( "Destroy property tool" )
    SaveState::Container &root = save.WriteContainer ( CONFIG_KEY_SECTION );

    if ( !_ui )
    {
        root.Write ( CONFIG_KEY_SHOW, false );
    }
    else
    {
        root.Write ( CONFIG_KEY_SHOW, true );
        _ui->GetRect ( _rect );
        _ui->Disconnect ();
    }

    SaveState::Container &ui = root.WriteArray ( CONFIG_KEY_UI );
    ui.Write ( _rect._left );
    ui.Write ( _rect._right );
    ui.Write ( _rect._top );
    ui.Write ( _rect._bottom );
}

void PropertyTool::Rebuild ( Selection::Actors const &actors ) noexcept
{
    AV_TRACE ( "Rebuild properties" )

    if ( !_ui )
        return;

    MessageQueue::Instance ().EnqueueBack (
        Message ( eMessageType::InvokeUI,
            [ this, actors ] () noexcept -> void* {
                AV_TRACE ( "Rebuild properties" )
                _ui->GetRect ( _rect );

                MessageQueue::Instance ().EnqueueBack (
                    Message ( eMessageType::UIRemoveWidget,
                        [ ui = std::exchange ( _ui, nullptr ) ] () noexcept {
                            return ui;
                        }
                    )
                );

                CreateUI ( actors );
                return nullptr;
            }
        )
    );
}

bool PropertyTool::DeleteComponents () noexcept
{
    AV_TRACE ( "Delete components" )

    if ( !_ui )
        return false;

    Selection::Actors &actors = Workspace::Instance ().GetSelection ().GetActors ();
    DeleteComponentAction::Items items {};

    for ( Actor* actor : actors )
    {
        for ( ComponentRef &component : actor->GetComponents () )
        {
            if ( Component *c = component.get (); c->IsEditing () )
            {
                items.push_back (
                    {
                        ._actor = actor,
                        ._key = c
                    }
                );
            }
        }
    }

    if ( items.empty () ) [[likely]]
        return false;

    History &history = History::Instance ();
    history.Begin ();
    history.Append ( std::make_unique<DeleteComponentAction> ( std::move ( items ) ) );
    history.End ();

    return true;
}

void PropertyTool::OnContentUpdated ( Selection::Actors const &/*actors*/ ) noexcept
{
    // FUCK
}

void PropertyTool::Append ( std::string_view name, StringProperty &property ) noexcept
{
    _ui->Append ( name, property );
}

void PropertyTool::Append ( StringProperty &componentName,
    bool &componentCollapse,
    bool &componentEdit,
    std::string_view componentGlyph
) noexcept
{
    _ui->Append ( componentName, componentCollapse, componentEdit, componentGlyph );
}

void PropertyTool::AppendSeparator () noexcept
{
    _ui->AppendSeparator ();
}

PropertyTool &PropertyTool::Instance () noexcept
{
    return *_instance;
}

void PropertyTool::CreateUI ( Selection::Actors const &actors ) noexcept
{
    AV_TRACE ( "Create properties" )

    _ui = new UIPropertyDialog (
        [ this ] () noexcept {
            _ui->GetRect ( _rect );

            MessageQueue::Instance ().EnqueueBack (
                Message ( eMessageType::UIRemoveWidget,
                    [ ui = std::exchange ( _ui, nullptr ) ] () noexcept {
                        return ui;
                    }
                )
            );
        }
    );

    _ui->SetRect ( _rect );
    _ui->SetMinSize ( MIN_WIDTH, MIN_HEIGHT );

    if ( !actors.empty () )
        ( *actors.cbegin () )->ShowProperties ( *this );

    MessageQueue &messageQueue = MessageQueue::Instance ();

    messageQueue.EnqueueBack (
        Message ( eMessageType::UIPrependWidget,
            [ ui = _ui ] () noexcept {
                return ui;
            }
        )
    );
}

} // namespace editor
