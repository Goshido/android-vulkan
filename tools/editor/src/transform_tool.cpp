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
pbr::LengthValue const MIN_WIDTH ( pbr::LengthValue::eType::PX, 155.0F );
pbr::LengthValue const HEIGHT ( pbr::LengthValue::eType::PX, 204.0F );
pbr::LengthValue const DEFAULT_X ( pbr::LengthValue::eType::PX, 350.0F );
pbr::LengthValue const DEFAULT_Y ( pbr::LengthValue::eType::PX, 18.0F );
pbr::LengthValue const DEFAULT_WIDTH ( pbr::LengthValue::eType::PX, 247.0F );

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
    AV_TRACE ( "Transform tool: init" )

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

    GXVec4 const beta ( DEFAULT_X.GetValue (),
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

void TransformTool::OnContentCommitted ( Selection::Actors const &actors ) noexcept
{
    AV_TRACE ( "Transform tool: content committed" )
    OnContentUpdated ( actors );
    _items = std::move ( _updated );
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

        std::bind_front ( &TransformTool::RevertLocation, this ),
        std::bind_front ( &TransformTool::RevertScale, this ),

        [ this, &control = _x ] ( std::string const &value ) noexcept {
            ChangeLocation ( value, control );
        },

        [ this, &control = _x ] ( std::string const &value ) noexcept {
            NewLocation ( value, control );
        },

        [ this, &control = _y ] ( std::string const &value ) noexcept {
            ChangeLocation ( value, control );
        },

        [ this, &control = _y ] ( std::string const &value ) noexcept {
            NewLocation ( value, control );
        },

        [ this, &control = _z ] ( std::string const &value ) noexcept {
            ChangeLocation ( value, control );
        },

        [ this, &control = _z ] ( std::string const &value ) noexcept {
            NewLocation ( value, control );
        },

        [ this, &control = _scaleX ] ( std::string const &value ) noexcept {
            ChangeScale ( value, control );
        },

        [ this, &control = _scaleX ] ( std::string const &value ) noexcept {
            NewScale ( value, control );
        },

        [ this, &control = _scaleY ] ( std::string const &value ) noexcept {
            ChangeScale ( value, control );
        },

        [ this, &control = _scaleY ] ( std::string const &value ) noexcept {
            NewScale ( value, control );
        },

        [ this, &control = _scaleZ ] ( std::string const &value ) noexcept {
            ChangeScale ( value, control );
        },

        [ this, &control = _scaleZ ] ( std::string const &value ) noexcept {
            NewScale ( value, control );
        }
    );

    _ui->SetRect ( _rect );
    _ui->SetMinSize ( MIN_WIDTH, HEIGHT );
    _ui->SetMaxSize ( theme::MAX_LENGTH, HEIGHT );

    _updated = _items;
    UpdateUI ();

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
    _x._value = Float { firstLocation._data[ 0U ] };
    _y._value = Float { firstLocation._data[ 1U ] };
    _z._value = Float { firstLocation._data[ 2U ] };

    GXVec3 const &firstScale = ( i++ )->_scale;
    _scaleX._value = Float { firstScale._data[ 0U ] };
    _scaleY._value = Float { firstScale._data[ 1U ] };
    _scaleZ._value = Float { firstScale._data[ 2U ] };

    constexpr auto check = [] ( Float &&current, float other ) noexcept -> Float {
        Float cases[] = { std::move ( current ), std::nullopt };
        return cases[ static_cast<uint32_t> ( !current || other != *current ) ];
    };

    for ( auto const end = _updated.cend (); i != end; ++i )
    {
        GXVec3 const &location = i->_location;
        _x._value = check ( std::move ( _x._value ), location._data[ 0U ] );
        _y._value = check ( std::move ( _y._value ), location._data[ 1U ] );
        _z._value = check ( std::move ( _z._value ), location._data[ 2U ] );

        GXVec3 const &scale = i->_scale;
        _scaleX._value = check ( std::move ( _scaleX._value ), scale._data[ 0U ] );
        _scaleY._value = check ( std::move ( _scaleY._value ), scale._data[ 1U ] );
        _scaleZ._value = check ( std::move ( _scaleZ._value ), scale._data[ 2U ] );
    }

    constexpr auto resolve = [] ( Float const &value ) noexcept -> std::string {
        if ( !value )
            return "-";

        char buf[ 128U ];
        int const symbols = std::snprintf ( buf, std::size ( buf ), "%g", *value );
        return std::string ( buf, static_cast<size_t> ( symbols ) );
    };

    if ( !std::exchange ( _x._ignoreUpdate, false ) )
        ui.SetX ( resolve ( _x._value ) );

    if ( !std::exchange ( _y._ignoreUpdate, false ) )
        ui.SetY ( resolve ( _y._value ) );

    if ( !std::exchange ( _z._ignoreUpdate, false ) )
        ui.SetZ ( resolve ( _z._value ) );

    if ( !std::exchange ( _scaleX._ignoreUpdate, false ) )
        ui.SetScaleX ( resolve ( _scaleX._value ) );

    if ( !std::exchange ( _scaleY._ignoreUpdate, false ) )
        ui.SetScaleY ( resolve ( _scaleY._value ) );

    if ( !std::exchange ( _scaleZ._ignoreUpdate, false ) )
    {
        ui.SetScaleZ ( resolve ( _scaleZ._value ) );
    }
}

void TransformTool::ChangeLocation ( std::string const &value, Control &control ) noexcept
{
    AV_TRACE ( "Transform tool: change location" )

    if ( _items.empty () ) [[unlikely]]
        return;

    Float v = ToFloat ( value );

    if ( !v ) [[unlikely]]
        return;

    control =
    {
        ._value = std::move ( v ),
        ._ignoreUpdate = true
    };

    Workspace &workspace = Workspace::Instance ();

    for ( Actor* actor : workspace.GetSelection ().GetActors () )
    {
        if ( auto const after = ResolveLocation ( actor->GetLocation () ); after ) [[likely]]
        {
            actor->SetLocation ( *after );
        }
    }

    workspace.OnContentUpdated ();
}

void TransformTool::CommitLocation () noexcept
{
    AV_TRACE ( "Transform tool: commit location" )
    auto item = _items.begin ();

    MoveActorAction::Items moveItems {};
    moveItems.reserve ( _items.size () );

    for ( Actor* actor : Workspace::Instance ().GetSelection ().GetActors () )
    {
        GXVec3 &before = ( item++ )->_location;
        auto const after = ResolveLocation ( before );

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

void TransformTool::NewLocation ( std::string const &value, Control &control ) noexcept
{
    AV_TRACE ( "Transform tool: new location" )

    if ( _items.empty () ) [[unlikely]]
        return;

    if ( Float v = ToFloat ( value ); v ) [[likely]]
    {
        control._value = std::move ( v );
        CommitLocation ();
        return;
    }

    RevertLocation ();
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

void TransformTool::ChangeScale ( std::string const &value, Control &control ) const noexcept
{
    AV_TRACE ( "Transform tool: change scale" )

    if ( _items.empty () ) [[unlikely]]
        return;

    Float v = ToFloat ( value );

    if ( !v ) [[unlikely]]
        return;

    control =
    {
        ._value = std::move ( v ),
        ._ignoreUpdate = true
    };

    for ( Actor* actor : Workspace::Instance ().GetSelection ().GetActors () )
    {
        if ( auto const after = ResolveScale ( actor->GetScale () ); after ) [[likely]]
        {
            actor->SetScale ( *after );
        }
    }
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
        bool const change = CheckAxis ( before._data[ 0U ], x._value ) |
            CheckAxis ( before._data[ 1U ], y._value ) |
            CheckAxis ( before._data[ 2U ], z._value );

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

void TransformTool::NewScale ( std::string const &value, Control &control ) noexcept
{
    AV_TRACE ( "Transform tool: new scale" )

    if ( _items.empty () ) [[unlikely]]
        return;

    if ( Float v = ToFloat ( value ); v ) [[likely]]
    {
        control._value = std::move ( v );
        CommitScale ();
        return;
    }

    RevertScale ();
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

std::optional<GXVec3> TransformTool::ResolveLocation ( GXVec3 before ) const noexcept
{
    bool const change = CheckAxis ( before._data[ 0U ], _x._value ) |
        CheckAxis ( before._data[ 1U ], _y._value ) |
        CheckAxis ( before._data[ 2U ], _z._value );

    std::optional<GXVec3> cases[] = { std::nullopt, std::optional<GXVec3> { std::move ( before ) } };
    return cases[ static_cast<uint32_t> ( change ) ];
}

std::optional<GXVec3> TransformTool::ResolveScale ( GXVec3 before ) const noexcept
{
    bool const change = CheckAxis ( before._data[ 0U ], _scaleX._value ) |
        CheckAxis ( before._data[ 1U ], _scaleY._value ) |
        CheckAxis ( before._data[ 2U ], _scaleZ._value );

    std::optional<GXVec3> cases[] = { std::nullopt, std::optional<GXVec3> { std::move ( before ) } };
    return cases[ static_cast<uint32_t> ( change ) ];
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
