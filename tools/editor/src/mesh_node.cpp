#include <precompiled_headers.hpp>
#include <av_assert.hpp>
#include <mesh_node.hpp>
#include <workspace.hpp>


namespace editor {

MeshNode::MeshNode ( MeshNode &&other ) noexcept
{
    std::ignore = other.TryLock ();

    _workspace = std::exchange ( other._workspace, nullptr );
    _hasChanges = std::move ( other._hasChanges );
    _internal = std::exchange ( other._internal, nullptr );
    _info = std::move ( other._info );

    other.Unlock ();
}

MeshNode &MeshNode::operator = ( MeshNode &&other ) noexcept
{
    if ( this == &other || !other.TryLock () ) [[unlikely]]
        return *this;

    _workspace = std::exchange ( other._workspace, nullptr );
    _hasChanges = std::move ( other._hasChanges );
    _internal = std::exchange ( other._internal, nullptr );
    _info = std::move ( other._info );

    other.Unlock ();
    return *this;
}

MeshNode::MeshNode ( Workspace &workspace, MeshInfo &internal ) noexcept:
    WorkspaceNode ( workspace ),
    _internal ( &internal )
{
    // NOTHING
}

MeshNode::~MeshNode () noexcept
{
    if ( !TryLock () ) [[unlikely]]
        return;

    _workspace->Unregister ( *this );
    _workspace = nullptr;
    Unlock ();
}

void MeshNode::Commit () noexcept
{
    if ( !_hasChanges || !TryLock () ) [[likely]]
        return;

    *_internal = _info;
    _hasChanges = false;

    Unlock ();
}

MeshInfo const &MeshNode::GetInternalInfo () const noexcept
{
    AV_ASSERT ( _internal )
    return *_internal;
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

    if ( !TryLock () ) [[unlikely]]
        return;

   _info._color = c;
    _hasChanges = true;

    Unlock ();
}

void MeshNode::SetLocal ( GXMat4 const &local ) noexcept
{
    if ( !TryLock () ) [[unlikely]]
        return;

    _info._local = local;
    _hasChanges = true;

    Unlock ();
}

void MeshNode::SetBounds ( GXAABB const &boundLocal ) noexcept
{
    if ( !TryLock () ) [[unlikely]]
        return;

    _info._boundLocal = boundLocal;
    _hasChanges = true;

    Unlock ();
}

void MeshNode::SetMaterial ( PBRMaterial const &material ) noexcept
{
    if ( !TryLock () ) [[unlikely]]
        return;

    _info._material = material;
    _hasChanges = true;

    Unlock ();
}

} // namespace editor
