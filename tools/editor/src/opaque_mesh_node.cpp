#include <precompiled_headers.hpp>
#include <av_assert.hpp>
#include <mesh_node.hpp>
#include <workspace.hpp>


namespace editor {

MeshNode::MeshNode ( MeshNode &&other ) noexcept:
    _internal ( std::move ( other._internal ) ),
    _workspace ( std::move ( other._workspace ) ),
    _meshInfo ( std::move ( other._meshInfo ) ),
    _hasChanges ( std::move ( other._hasChanges ) )
{
    _lock.store ( other._lock.load () );
}

MeshNode &MeshNode::operator = ( MeshNode &&other ) noexcept
{
    if ( this == &other ) [[unlikely]]
        return *this;

    _internal = std::move ( other._internal );
    _meshInfo = std::move ( other._meshInfo );
    _workspace = std::move ( other._workspace );
    _hasChanges = std::move ( other._hasChanges );
    _lock.store ( other._lock.load () );

    return *this;
}

MeshNode::MeshNode ( Workspace &workspace, MeshInfo const &internal ) noexcept:
    _internal ( &internal ),
    _workspace ( &workspace )
{
    // NOTHING
}

MeshNode::~MeshNode () noexcept
{
    if ( _workspace ) [[likely]]
    {
        _workspace->Unregister ( *this );
    }
}

MeshInfo const &MeshNode::GetInternalMeshInfo () const noexcept
{
    AV_ASSERT ( _internal )
    return *_internal;
}

void MeshNode::Commit ( GXMat4 &local, GXAABB &bounds, ColorData &color, PBRMaterial &material ) noexcept
{
    if ( !_hasChanges ) [[likely]]
        return;

    Lock ();

    local = _meshInfo._local;
    bounds = _meshInfo._bounds;
    color = _meshInfo._color;
    material = _meshInfo._material;
    _hasChanges = false;

    _lock.store ( false );
}

void MeshNode::SetColor ( GXColorUNORM color0,
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

    _meshInfo._color = c;
    _hasChanges = true;

    _lock.store ( false );
}

void MeshNode::SetLocal ( GXMat4 const &local, GXAABB const &localBounds ) noexcept
{
    Lock ();

    _meshInfo._local = local;
    localBounds.Transform ( _meshInfo._bounds, local );
    _hasChanges = true;

    _lock.store ( false );
}

void MeshNode::SetMaterial ( PBRMaterial const &material ) noexcept
{
    Lock ();

    _meshInfo._material = material;
    _hasChanges = true;

    _lock.store ( false );
}

void MeshNode::Lock () noexcept
{
    bool expected = false;

    while ( !_lock.compare_exchange_weak ( expected, true ) ) [[unlikely]]
        expected = false;

    AV_ASSERT ( !expected )
}

} // namespace editor
