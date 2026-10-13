#include <precompiled_headers.hpp>
#include <message_queue.hpp>
#include <trace.hpp>
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
    AV_TRACE ( "Transform tool: destroy" )
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

void TransformTool::OnContentUpdated ( Selection::Actors const &actors ) noexcept
{
    AV_TRACE ( "Transform tool: content updated" )
    _updated.clear ();
    _updated.reserve ( actors.size () );

    for ( auto &actor : actors )
    {
        _updated.push_back (
            {
                ._location = actor->GetLocation (),
                ._scale = actor->GetScale ()
            }
        );
    }

    if ( _ui )
    {
        UpdateUI ();
    }
}

void TransformTool::OnSelectionChanged ( Selection::Actors const &actors ) noexcept
{
    AV_TRACE ( "Transform tool: selection changed" )
    OnContentUpdated ( actors );
    _items = std::move ( _updated );
}

void TransformTool::CreateUI () noexcept
{
    AV_TRACE ( "Transform tool: create UI" )
    _ui = new UITransform (
        [ this ] () noexcept {
            _ui->GetRect ( _rect );
            _ui = nullptr;
        },

        std::bind ( &TransformTool::CancelX, this ),
        std::bind ( &TransformTool::ChangeX, this, std::placeholders::_1 ),
        std::bind ( &TransformTool::NewX, this, std::placeholders::_1 ),
        std::bind ( &TransformTool::NextX, this, std::placeholders::_1 ),

        std::bind ( &TransformTool::CancelY, this ),
        std::bind ( &TransformTool::ChangeY, this, std::placeholders::_1 ),
        std::bind ( &TransformTool::NewY, this, std::placeholders::_1 ),
        std::bind ( &TransformTool::NextY, this, std::placeholders::_1 ),

        std::bind ( &TransformTool::CancelZ, this ),
        std::bind ( &TransformTool::ChangeZ, this, std::placeholders::_1 ),
        std::bind ( &TransformTool::NewZ, this, std::placeholders::_1 ),
        std::bind ( &TransformTool::NextZ, this, std::placeholders::_1 ),

        std::bind ( &TransformTool::CancelScaleX, this ),
        std::bind ( &TransformTool::ChangeScaleX, this, std::placeholders::_1 ),
        std::bind ( &TransformTool::NewScaleX, this, std::placeholders::_1 ),
        std::bind ( &TransformTool::NextScaleX, this, std::placeholders::_1 ),

        std::bind ( &TransformTool::CancelScaleY, this ),
        std::bind ( &TransformTool::ChangeScaleY, this, std::placeholders::_1 ),
        std::bind ( &TransformTool::NewScaleY, this, std::placeholders::_1 ),
        std::bind ( &TransformTool::NextScaleY, this, std::placeholders::_1 ),

        std::bind ( &TransformTool::CancelScaleZ, this ),
        std::bind ( &TransformTool::ChangeScaleZ, this, std::placeholders::_1 ),
        std::bind ( &TransformTool::NewScaleZ, this, std::placeholders::_1 ),
        std::bind ( &TransformTool::NextScaleZ, this, std::placeholders::_1 )
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

void TransformTool::UpdateUI () noexcept
{
    AV_TRACE ( "Transform tool: update UI" )
    UITransform &ui = *_ui;

    if ( _updated.empty () )
    {
        ui.SetX ( "" );
        ui.SetY ( "" );
        ui.SetZ ( "" );
        ui.SetScaleX ( "" );
        ui.SetScaleY ( "" );
        ui.SetScaleZ ( "" );
        return;
    }

    auto i = _updated.cbegin ();

    GXVec3 const &firstLocation = i->_location;
    std::optional<float> x = std::optional<float> { firstLocation._data[ 0U ] };
    std::optional<float> y = std::optional<float> { firstLocation._data[ 1U ] };
    std::optional<float> z = std::optional<float> { firstLocation._data[ 2U ] };

    GXVec3 const &firstScale = ( i++ )->_scale;
    std::optional<float> scaleX = std::optional<float> { firstScale._data[ 0U ] };
    std::optional<float> scaleY = std::optional<float> { firstScale._data[ 1U ] };
    std::optional<float> scaleZ = std::optional<float> { firstScale._data[ 2U ] };

    constexpr auto check = [] ( std::optional<float> &&current, float other ) noexcept -> std::optional<float> {
        std::optional<float> cases[] = { std::move ( current ), std::nullopt };
        return cases[ static_cast<uint32_t> ( !current || other != *current ) ];
    };

    for ( auto const end = _updated.cend (); i != end; ++i )
    {
        GXVec3 const &location = i->_location;
        x = check ( std::move ( x ), location._data[ 0U ] );
        y = check ( std::move ( y ), location._data[ 1U ] );
        z = check ( std::move ( z ), location._data[ 2U ] );

        GXVec3 const &scale = i->_scale;
        scaleX = check ( std::move ( scaleX ), scale._data[ 0U ] );
        scaleY = check ( std::move ( scaleY ), scale._data[ 1U ] );
        scaleZ = check ( std::move ( scaleZ ), scale._data[ 2U ] );
    }

    constexpr auto resolve = [] ( std::optional<float> const &value ) noexcept -> std::string {
        if ( !value )
            return "-";

        char buf[ 128U ];
        int const symbols = std::snprintf ( buf, std::size ( buf ), "%g", *value );
        return std::string ( buf, static_cast<size_t> ( symbols ) );
    };

    ui.SetX ( resolve ( x ) );
    ui.SetY ( resolve ( y ) );
    ui.SetZ ( resolve ( z ) );

    ui.SetScaleX ( resolve ( scaleX ) );
    ui.SetScaleY ( resolve ( scaleY ) );
    ui.SetScaleZ ( resolve ( scaleZ ) );
}


void TransformTool::CancelX () noexcept
{
    AV_TRACE ( "Transform tool: cancel X" )
    android_vulkan::LogDebug ( "CancelX" );
}

void TransformTool::ChangeX ( std::string const &/*value*/ ) noexcept
{
    AV_TRACE ( "Transform tool: change X" )
    android_vulkan::LogDebug ( "ChangeX" );
}

void TransformTool::NewX ( std::string const &value ) noexcept
{
    AV_TRACE ( "Transform tool: new X" )
    auto const x = ToFloat ( value );

    if ( !x ) [[unlikely]]
        return;

    // FUCK

    android_vulkan::LogDebug ( "NewX" );
}

void TransformTool::NextX ( std::string const &/*value*/ ) noexcept
{
    AV_TRACE ( "Transform tool: next X" )
    android_vulkan::LogDebug ( "NextX" );
}

void TransformTool::CancelY () noexcept
{
    AV_TRACE ( "Transform tool: cancel Y" )
    android_vulkan::LogDebug ( "CancelY" );
}

void TransformTool::ChangeY ( std::string const &/*value*/ ) noexcept
{
    AV_TRACE ( "Transform tool: change Y" )
    android_vulkan::LogDebug ( "ChangeY" );
}

void TransformTool::NewY ( std::string const &/*value*/ ) noexcept
{
    AV_TRACE ( "Transform tool: new Y" )
    android_vulkan::LogDebug ( "NewY" );
}

void TransformTool::NextY ( std::string const &/*value*/ ) noexcept
{
    AV_TRACE ( "Transform tool: next Y" )
    android_vulkan::LogDebug ( "NextY" );
}

void TransformTool::CancelZ () noexcept
{
    AV_TRACE ( "Transform tool: cancel Z" )
    android_vulkan::LogDebug ( "CancelZ" );
}

void TransformTool::ChangeZ ( std::string const &/*value*/ ) noexcept
{
    AV_TRACE ( "Transform tool: change Z" )
    android_vulkan::LogDebug ( "ChangeZ" );
}

void TransformTool::NewZ ( std::string const &/*value*/ ) noexcept
{
    AV_TRACE ( "Transform tool: new Z" )
    android_vulkan::LogDebug ( "NewZ" );
}

void TransformTool::NextZ ( std::string const &/*value*/ ) noexcept
{
    AV_TRACE ( "Transform tool: next Z" )
    android_vulkan::LogDebug ( "NextZ" );
}

void TransformTool::CancelScaleX () noexcept
{
    AV_TRACE ( "Transform tool: cancel scale X" )
    android_vulkan::LogDebug ( "CancelScaleX" );
}

void TransformTool::ChangeScaleX ( std::string const &/*value*/ ) noexcept
{
    AV_TRACE ( "Transform tool: change scale X" )
    android_vulkan::LogDebug ( "ChangeScaleX" );
}

void TransformTool::NewScaleX ( std::string const &/*value*/ ) noexcept
{
    AV_TRACE ( "Transform tool: new scale X" )
    android_vulkan::LogDebug ( "NewScaleX" );
}

void TransformTool::NextScaleX ( std::string const &/*value*/ ) noexcept
{
    AV_TRACE ( "Transform tool: next scale X" )
    android_vulkan::LogDebug ( "NextScaleX" );
}

void TransformTool::CancelScaleY () noexcept
{
    AV_TRACE ( "Transform tool: cancel scale Y" )
    android_vulkan::LogDebug ( "CancelScaleY" );
}

void TransformTool::ChangeScaleY ( std::string const &/*value*/ ) noexcept
{
    AV_TRACE ( "Transform tool: change scale Y" )
    android_vulkan::LogDebug ( "ChangeScaleY" );
}

void TransformTool::NewScaleY ( std::string const &/*value*/ ) noexcept
{
    AV_TRACE ( "Transform tool: new scale Y" )
    android_vulkan::LogDebug ( "NewScaleY" );
}

void TransformTool::NextScaleY ( std::string const &/*value*/ ) noexcept
{
    AV_TRACE ( "Transform tool: next scale Y" )
    android_vulkan::LogDebug ( "NextScaleY" );
}

void TransformTool::CancelScaleZ () noexcept
{
    AV_TRACE ( "Transform tool: cancel scale Z" )
    android_vulkan::LogDebug ( "CancelScaleZ" );
}

void TransformTool::ChangeScaleZ ( std::string const &/*value*/ ) noexcept
{
    AV_TRACE ( "Transform tool: change scale Z" )
    android_vulkan::LogDebug ( "ChangeScaleZ" );
}

void TransformTool::NewScaleZ ( std::string const &/*value*/ ) noexcept
{
    AV_TRACE ( "Transform tool: new scale Z" )
    android_vulkan::LogDebug ( "NewScaleZ" );
}

void TransformTool::NextScaleZ ( std::string const &/*value*/ ) noexcept
{
    AV_TRACE ( "Transform tool: next scale Z" )
    android_vulkan::LogDebug ( "NextScaleZ" );
}

std::optional<float> TransformTool::ToFloat ( std::string const &string ) noexcept
{
    if ( string.empty () ) [[unlikely]]
        return std::nullopt;

    std::optional<float> cases[] = { std::nullopt, std::optional<float> { 42.0F } };
    char const *begin = string.data ();
    char const *end = begin + string.size ();
    auto const [ptr, status] = std::from_chars ( begin, end, *cases[ 1U ], std::chars_format::general );
    constexpr std::errc ok {};
    return cases[ static_cast<uint32_t> ( ( status == ok ) & ( ptr == end ) ) ];
}

} // namespace editor
