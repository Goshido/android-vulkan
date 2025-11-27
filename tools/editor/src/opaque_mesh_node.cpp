#include <precompiled_headers.hpp>
#include <opaque_mesh_node.hpp>
#include <workspace.hpp>


namespace editor {

OpaqueMeshNode::OpaqueMeshNode ( OpaqueMeshNode &&other ) noexcept:
    _bounds ( std::move ( other._bounds ) ),
    _local ( std::move ( other._local ) ),
    _workspace ( std::move ( other._workspace ) ),
    _hasChanges ( std::move ( other._hasChanges ) )
{
    _lock.store ( other._lock.load () );
}

OpaqueMeshNode &OpaqueMeshNode::operator = ( OpaqueMeshNode &&other ) noexcept
{
    if ( this == &other ) [[unlikely]]
        return *this;

    _bounds = std::move ( other._bounds );
    _local = std::move ( other._local );
    _workspace = std::move ( other._workspace );
    _hasChanges = std::move ( other._hasChanges );
    _lock.store ( other._lock.load () );

    return *this;
}

OpaqueMeshNode::OpaqueMeshNode ( Workspace &workspace ) noexcept:
    _workspace ( &workspace )
{
    // NOTHING
}

OpaqueMeshNode::~OpaqueMeshNode () noexcept
{
    if ( _workspace ) [[likely]]
    {
        _workspace->Unregister ( *this );
    }
}

void OpaqueMeshNode::Commit ( GXMat4 &local, GXAABB &bounds, ColorData &color, PBRMaterial &material ) noexcept
{
    if ( !_hasChanges ) [[likely]]
        return;

    Lock ();

    local = _local;
    bounds = _bounds;
    color = _color;
    material = _material;
    _hasChanges = false;

    _lock.store ( false );
}

void OpaqueMeshNode::SetColor ( GXColorUNORM color0,
    GXColorUNORM color1,
    GXColorUNORM color2,
    GXColorUNORM emission,
    float emissionIntensity
) noexcept
{
    // Emission intensity should take range from 0 to 6000. Emission intensity is packed as 24bit fixed point value.
    constexpr double maxIntensity = 6.0e+3;
    constexpr double convertFactor = static_cast<double> ( 0x00FFFFFFU ) / maxIntensity;
    double const beta = convertFactor * std::clamp ( static_cast<double> ( emissionIntensity ), 0.0, maxIntensity );

    ColorData const c
    {
        ._emiRcol0rgb = static_cast<uint32_t> ( emission._data[ 0U ] ) | ( std::bit_cast<uint32_t> ( color0 ) << 8U ),
        ._emiGcol1rgb = static_cast<uint32_t> ( emission._data[ 1U ] ) | ( std::bit_cast<uint32_t> ( color1 ) << 8U ),
        ._emiBcol2rgb = static_cast<uint32_t> ( emission._data[ 2U ] ) | ( std::bit_cast<uint32_t> ( color2 ) << 8U ),
        ._col0aEmiIntens = static_cast<uint32_t> ( color0._data[ 3U ] ) | ( static_cast<uint32_t> ( beta ) << 8U )
    };

    Lock ();

    _color = c;
    _hasChanges = true;

    _lock.store ( false );
}

void OpaqueMeshNode::SetLocal ( GXMat4 const &local, GXAABB const &localBounds ) noexcept
{
    Lock ();

    _local = local;
    localBounds.Transform ( _bounds, local );
    _hasChanges = true;

    _lock.store ( false );
}

void OpaqueMeshNode::SetMaterial ( PBRMaterial const &/*material*/ ) noexcept
{
    // FUCK
}

void OpaqueMeshNode::Lock () noexcept
{
    bool expected = false;

    while ( !_lock.compare_exchange_weak ( expected, true ) ) [[unlikely]]
        expected = false;

    AV_ASSERT ( !expected )
}

} // namespace editor
