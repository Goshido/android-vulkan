#include <precompiled_headers.hpp>
#include <message_queue.hpp>
#include <move_actor_action.hpp>
#include <trace.hpp>
#include <transform_tool.hpp>
#include <workspace.hpp>


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

//----------------------------------------------------------------------------------------------------------------------

class ScaleActorAction final : public Action
{
    public:
        struct Item final
        {
            Actor*      _actor = nullptr;
            GXVec3      _before {};
            GXVec3      _after {};
        };

        using Items = std::vector<Item>;

    private:
        Items           _items {};

    public:
        ScaleActorAction () = delete;

        ScaleActorAction ( ScaleActorAction const & ) = delete;
        ScaleActorAction &operator = ( ScaleActorAction const & ) = delete;

        ScaleActorAction ( ScaleActorAction && ) = default;
        ScaleActorAction &operator = ( ScaleActorAction && ) = default;

        explicit ScaleActorAction ( Items &&items ) noexcept;

        ~ScaleActorAction () override = default;

    private:
        void Redo () noexcept override;
        void Undo () noexcept override;
};

ScaleActorAction::ScaleActorAction ( Items &&items ) noexcept:
    _items ( std::move ( items ) )
{
    // NOTHING
}

void ScaleActorAction::Redo () noexcept
{
    AV_TRACE ( "Transform tool: redo scale" )

    for ( Item &item : _items )
        item._actor->SetScale ( item._after );

    Workspace::Instance ().OnContentUpdated ();
}

void ScaleActorAction::Undo () noexcept
{
    AV_TRACE ( "Transform tool: undo scale" )

    for ( Item &item : _items )
        item._actor->SetScale ( item._before );

    Workspace::Instance ().OnContentUpdated ();
}

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

        [ this, &axis = _x ] ( std::string const &value ) noexcept {
            NewLocation ( value, axis );
        },

        std::bind ( &TransformTool::CancelY, this ),
        std::bind ( &TransformTool::ChangeY, this, std::placeholders::_1 ),

        [ this, &axis = _y ] ( std::string const &value ) noexcept {
            NewLocation ( value, axis );
        },

        std::bind ( &TransformTool::CancelZ, this ),
        std::bind ( &TransformTool::ChangeZ, this, std::placeholders::_1 ),

        [ this, &axis = _z ] ( std::string const &value ) noexcept {
            NewLocation ( value, axis );
        },

        std::bind ( &TransformTool::CancelScaleX, this ),
        std::bind ( &TransformTool::ChangeScaleX, this, std::placeholders::_1 ),

        [ this, &axis = _scaleX ] ( std::string const &value ) noexcept {
            NewScale ( value, axis );
        },

        std::bind ( &TransformTool::CancelScaleY, this ),
        std::bind ( &TransformTool::ChangeScaleY, this, std::placeholders::_1 ),

        [ this, &axis = _scaleY ] ( std::string const &value ) noexcept {
            NewScale ( value, axis );
        },

        std::bind ( &TransformTool::CancelScaleZ, this ),
        std::bind ( &TransformTool::ChangeScaleZ, this, std::placeholders::_1 ),

        [ this, &axis = _scaleZ ] ( std::string const &value ) noexcept {
            NewScale ( value, axis );
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
    _x = Float { firstLocation._data[ 0U ] };
    _y = Float { firstLocation._data[ 1U ] };
    _z = Float { firstLocation._data[ 2U ] };

    GXVec3 const &firstScale = ( i++ )->_scale;
    _scaleX = Float { firstScale._data[ 0U ] };
    _scaleY = Float { firstScale._data[ 1U ] };
    _scaleZ = Float { firstScale._data[ 2U ] };

    constexpr auto check = [] ( Float &&current, float other ) noexcept -> Float {
        Float cases[] = { std::move ( current ), std::nullopt };
        return cases[ static_cast<uint32_t> ( !current || other != *current ) ];
    };

    for ( auto const end = _updated.cend (); i != end; ++i )
    {
        GXVec3 const &location = i->_location;
        _x = check ( std::move ( _x ), location._data[ 0U ] );
        _y = check ( std::move ( _y ), location._data[ 1U ] );
        _z = check ( std::move ( _z ), location._data[ 2U ] );

        GXVec3 const &scale = i->_scale;
        _scaleX = check ( std::move ( _scaleX ), scale._data[ 0U ] );
        _scaleY = check ( std::move ( _scaleY ), scale._data[ 1U ] );
        _scaleZ = check ( std::move ( _scaleZ ), scale._data[ 2U ] );
    }

    constexpr auto resolve = [] ( Float const &value ) noexcept -> std::string {
        if ( !value )
            return "-";

        char buf[ 128U ];
        int const symbols = std::snprintf ( buf, std::size ( buf ), "%g", *value );
        return std::string ( buf, static_cast<size_t> ( symbols ) );
    };

    ui.SetX ( resolve ( _x ) );
    ui.SetY ( resolve ( _y ) );
    ui.SetZ ( resolve ( _z ) );

    ui.SetScaleX ( resolve ( _scaleX ) );
    ui.SetScaleY ( resolve ( _scaleY ) );
    ui.SetScaleZ ( resolve ( _scaleZ ) );
}

void TransformTool::NewLocation ( std::string const &value, Float &axis ) noexcept
{
    AV_TRACE ( "Transform tool: new location" )

    if ( _items.empty () ) [[unlikely]]
        return;

    Float v = ToFloat ( value );

    if ( !v ) [[unlikely]]
    {
        RevertLocation ();
        return;
    }

    axis = std::move ( v );
    CommitLocation ();
}

void TransformTool::CommitLocation () noexcept
{
    AV_TRACE ( "Transform tool: commit location" )
    auto item = _items.begin ();

    MoveActorAction::Items moveItems {};
    moveItems.reserve ( _items.size () );

    // Note - sending 'before' by value is intentional.
    auto const resolve = [ &x = _x, &y = _y, &z = _z ] ( GXVec3 before ) noexcept -> std::optional<GXVec3> {
        bool const change = CheckAxis ( before._data[ 0U ], x ) |
            CheckAxis ( before._data[ 1U ], y ) |
            CheckAxis ( before._data[ 2U ], z );

        std::optional<GXVec3> cases[] = { std::nullopt, std::optional<GXVec3> { std::move ( before ) } };
        return cases[ static_cast<uint32_t> ( change ) ];
    };

    for ( Actor* actor : Workspace::Instance ().GetSelection ().GetActors () )
    {
        GXVec3 &before = ( item++ )->_location;
        auto const after = resolve ( before );

        if ( !after ) [[unlikely]]
            continue;

        GXVec3 const &a = *after;

        moveItems.push_back (
            {
                ._actor = actor,
                ._before = before,
                ._after = a
            }
        );

        before = a;
    }

    if ( moveItems.empty () ) [[unlikely]]
        return;

    History &history = History::Instance ();
    history.Begin ();
    history.Append ( std::make_unique<MoveActorAction> ( std::move ( moveItems ) ) );
    history.End ();
}

void TransformTool::RevertLocation () const noexcept
{
    AV_TRACE ( "Transform tool: revert location" )
    auto item = _items.cbegin ();
    Workspace &workspace = Workspace::Instance ();

    for ( Actor* actor : workspace.GetSelection ().GetActors () )
        actor->SetLocation ( ( item++ )->_location );

    workspace.OnContentUpdated ();
}

void TransformTool::NewScale ( std::string const &value, Float &axis ) noexcept
{
    AV_TRACE ( "Transform tool: new scale" )

    if ( _items.empty () ) [[unlikely]]
        return;

    Float v = ToFloat ( value );

    if ( !v ) [[unlikely]]
    {
        RevertScale ();
        return;
    }

    axis = std::move ( v );
    CommitScale ();
}

void TransformTool::CommitScale () noexcept
{
    AV_TRACE ( "Transform tool: commit scale" )
    auto item = _items.begin ();

    ScaleActorAction::Items scaleItems {};
    scaleItems.reserve ( _items.size () );

    // Note - sending 'before' by value is intentional.
    auto const resolve = [ &x = _scaleX, &y = _scaleY, &z = _scaleZ ] (
        GXVec3 before
    ) noexcept -> std::optional<GXVec3> {
        bool const change = CheckAxis ( before._data[ 0U ], x ) |
            CheckAxis ( before._data[ 1U ], y ) |
            CheckAxis ( before._data[ 2U ], z );

        std::optional<GXVec3> cases[] = { std::nullopt, std::optional<GXVec3> { std::move ( before ) } };
        return cases[ static_cast<uint32_t> ( change ) ];
    };

    for ( Actor* actor : Workspace::Instance ().GetSelection ().GetActors () )
    {
        GXVec3 &before = ( item++ )->_scale;
        auto const after = resolve ( before );

        if ( !after ) [[unlikely]]
            continue;

        GXVec3 const &a = *after;

        scaleItems.push_back (
            {
                ._actor = actor,
                ._before = before,
                ._after = a
            }
        );

        before = a;
    }

    if ( scaleItems.empty () ) [[unlikely]]
        return;

    History &history = History::Instance ();
    history.Begin ();
    history.Append ( std::make_unique<ScaleActorAction> ( std::move ( scaleItems ) ) );
    history.End ();
}

void TransformTool::RevertScale () const noexcept
{
    AV_TRACE ( "Transform tool: revert scale" )
    auto item = _items.cbegin ();
    Workspace &workspace = Workspace::Instance ();

    for ( Actor* actor : workspace.GetSelection ().GetActors () )
        actor->SetScale ( ( item++ )->_scale );

    workspace.OnContentUpdated ();
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

bool TransformTool::CheckAxis ( float &before, Float const &after ) noexcept
{
    if ( !after )
        return false;

    float const a = *after;
    return a != std::exchange ( before, a );
}

TransformTool::Float TransformTool::ToFloat ( std::string const &string ) noexcept
{
    if ( string.empty () ) [[unlikely]]
        return std::nullopt;

    Float cases[] = { std::nullopt, Float { 42.0F } };
    char const *begin = string.data ();
    char const *end = begin + string.size ();
    auto const [ptr, status] = std::from_chars ( begin, end, *cases[ 1U ], std::chars_format::general );
    constexpr std::errc ok {};
    return cases[ static_cast<uint32_t> ( ( status == ok ) & ( ptr == end ) ) ];
}

} // namespace editor
