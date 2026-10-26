#include <precompiled_headers.hpp>
#include <property_tool.hpp>
#include <trace.hpp>


namespace editor {

namespace {

constexpr std::string_view CONFIG_KEY_SECTION = "property dialog";

constexpr std::string_view CONFIG_KEY_UI = "UI";
constexpr pbr::LengthValue MIN_WIDTH ( pbr::LengthValue::eType::PX, 255.0F );
constexpr pbr::LengthValue MIN_HEIGHT ( pbr::LengthValue::eType::PX, 70.0F );
constexpr pbr::LengthValue DEFAULT_X ( pbr::LengthValue::eType::PX, 22.0F );
constexpr pbr::LengthValue DEFAULT_Y ( pbr::LengthValue::eType::PX, 218.0F );
constexpr pbr::LengthValue DEFAULT_WIDTH ( pbr::LengthValue::eType::PX, 300.0F );
constexpr pbr::LengthValue DEFAULT_HEIGHT ( pbr::LengthValue::eType::PX, 200.0F );

constexpr std::string_view CONFIG_KEY_SHOW = "show";
constexpr bool DEFAULT_SHOW = true;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

void PropertyTool::Init ( SaveState::Container const &save ) noexcept
{
    AV_TRACE ( "Property tool: init" )

    _toggle = Hotkey ( eKey::KeyF1,
        false,
        false,
        false,

        [ this ] () noexcept {
            if ( !_ui )
            {
                CreateUI ();
                return;
            }

            _ui->Close ();
        }
    );

    constexpr GXVec4 beta ( DEFAULT_X.GetValue (),
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
        CreateUI ();
    }
}

void PropertyTool::Destroy ( SaveState::Container &save ) noexcept
{
    AV_TRACE ( "Property tool: destroy" )
    SaveState::Container &root = save.WriteContainer ( CONFIG_KEY_SECTION );

    if ( !_ui )
    {
        root.Write ( CONFIG_KEY_SHOW, false );
    }
    else
    {
        root.Write ( CONFIG_KEY_SHOW, true );
        _ui->GetRect ( _rect );
    }

    SaveState::Container &ui = root.WriteArray ( CONFIG_KEY_UI );
    ui.Write ( _rect._left );
    ui.Write ( _rect._right );
    ui.Write ( _rect._top );
    ui.Write ( _rect._bottom );
}

void PropertyTool::OnContentCommitted ( Selection::Actors const &/*actors*/ ) noexcept
{
    // FUCK
}

void PropertyTool::OnContentUpdated ( Selection::Actors const &/*actors*/ ) noexcept
{
    // FUCK
}

void PropertyTool::OnSelectionChanged ( Selection::Actors const &/*actors*/ ) noexcept
{
    // FUCK
}

void PropertyTool::CreateUI () noexcept
{
    AV_TRACE ( "Property tool: create UI" )

    _ui = new UIPropertyDialog (
        [ this ] () noexcept {
            _ui->GetRect ( _rect );
            _ui = nullptr;
        }
    );

    _ui->SetRect ( _rect );
    _ui->SetMinSize ( MIN_WIDTH, MIN_HEIGHT );

    MessageQueue::Instance ().EnqueueBack (
        Message ( eMessageType::UIPrependWidget,
            [ ui = _ui ] () noexcept {
                return ui;
            }
        )
    );
}

} // namespace editor
