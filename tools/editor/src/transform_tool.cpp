#include <precompiled_headers.hpp>
#include <message_queue.hpp>
#include <transform_tool.hpp>


namespace editor {

namespace {

constexpr std::string_view CONFIG_KEY_SECTION = "transform dialog";

constexpr std::string_view CONFIG_KEY_UI = "UI";
constexpr pbr::LengthValue MIN_WIDTH ( pbr::LengthValue::eType::PX, 155.0F );
constexpr pbr::LengthValue HEIGHT ( pbr::LengthValue::eType::PX, 233.0F );
constexpr pbr::LengthValue DEFAULT_X ( pbr::LengthValue::eType::PX, 300.0F );
constexpr pbr::LengthValue DEFAULT_Y ( pbr::LengthValue::eType::PX, 400.0F );
constexpr pbr::LengthValue DEFAULT_WIDTH ( pbr::LengthValue::eType::PX, 247.0F );

constexpr std::string_view CONFIG_KEY_SHOW = "show";
constexpr bool DEFAULT_SHOW = true;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

void TransformTool::Init ( SaveState::Container const &save ) noexcept
{
    _toggle = Hotkey ( eKey::KeyF2,
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
        DEFAULT_Y.GetValue () + HEIGHT.GetValue ()
    );

    GXVec4 zeta {};
    zeta.Multiply ( beta, pbr::CSSUnitToDevicePixel::GetInstance ()._fromPX );
    Rect const defaultUI ( zeta );

    SaveState::Container const &root = save.ReadContainer ( CONFIG_KEY_SECTION );
    SaveState::Container const &ui = root.ReadArray ( CONFIG_KEY_UI );

    // [2026/10/08] Attention do not use inplace array reading when constructing Rect. Constructor parameter evaluation
    // order is not defined in C++.
    // For example MSVC is using reverse order which would be incorrect for current algorithm.
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

void TransformTool::Destroy ( SaveState::Container &save ) noexcept
{
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

void TransformTool::CreateUI () noexcept
{
    _ui = new UITransform (
        [ this ] () noexcept {
            _ui->GetRect ( _rect );
            _ui = nullptr;
        }
    );

    _ui->SetRect ( _rect );
    _ui->SetMinSize ( MIN_WIDTH, HEIGHT );
    _ui->SetMaxSize ( theme::MAX_LENGTH, HEIGHT );

    MessageQueue::Instance ().EnqueueBack (
        Message ( eMessageType::UIPrependWidget,
            [ ui = _ui ] () noexcept {
                return ui;
            }
        )
    );
}

} // namespace editor
