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
    bool &componentSelect,
    std::string_view componentGlyph
) noexcept
{
    _ui->Append ( componentName, componentCollapse, componentSelect, componentGlyph );
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
            _ui = nullptr;
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
