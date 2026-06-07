#include <precompiled_headers.hpp>
#include <av_assert.hpp>
#include <mesh_node.hpp>
#include <texture2D_storage.hpp>
#include <workspace.hpp>


namespace editor {

MeshNode::MeshNode ( MeshNode &&other ) noexcept
{
    std::ignore = other.TryLock ();

    _workspace = std::exchange ( other._workspace, nullptr );
    _hasChanges = std::move ( other._hasChanges );
    _meshInfo = std::exchange ( other._meshInfo, nullptr );
    _renderInfo = std::move ( other._renderInfo );

    other.Unlock ();
}

MeshNode &MeshNode::operator = ( MeshNode &&other ) noexcept
{
    if ( this == &other || !other.TryLock () ) [[unlikely]]
        return *this;

    _workspace = std::exchange ( other._workspace, nullptr );
    _hasChanges = std::move ( other._hasChanges );
    _meshInfo = std::exchange ( other._meshInfo, nullptr );
    _renderInfo = std::move ( other._renderInfo );

    other.Unlock ();
    return *this;
}

MeshNode::MeshNode ( Workspace &workspace, MeshInfo &meshInfo ) noexcept:
    WorkspaceNode ( workspace ),
    _meshInfo ( &meshInfo )
{
    // NOTHING
}

MeshNode::~MeshNode () noexcept
{
    PBRMaterial &material = _renderInfo._material;
    Texture2DStorage &storage = Texture2DStorage::Instance ();

    if ( material._albedo )
        storage.Unload ( std::move ( material._albedo ) );

    if ( material._emission )
        storage.Unload ( std::move ( material._emission ) );

    if ( material._mask )
        storage.Unload ( std::move ( material._mask ) );

    if ( material._normal )
        storage.Unload ( std::move ( material._normal ) );

    if ( material._param )
        storage.Unload ( std::move ( material._param ) );

    if ( !_workspace || !TryLock () ) [[unlikely]]
        return;

    _workspace->Unregister ( *this );
    _workspace = nullptr;

    Unlock ();
}

void MeshNode::Commit () noexcept
{
    MeshInfo const &meshInfo = *_meshInfo;

    if ( !_hasChanges || !TryLock () ) [[likely]]
        return;

    GXMat4 local {};
    local.FromFast ( meshInfo._rotation, meshInfo._location );
    auto &x = *reinterpret_cast<GXVec3*> ( local._data );
    GXVec3 const &s = meshInfo._scale;

    auto &y = *reinterpret_cast<GXVec3*> ( local._data + 4U );
    x.Multiply ( x, s._data[ 0U ] );

    auto &z = *reinterpret_cast<GXVec3*> ( local._data + 8U );
    y.Multiply ( y, s._data[ 1U ] );
    z.Multiply ( z, s._data[ 2U ] );

    _renderInfo._material = meshInfo._material;
    _renderInfo._local = local;
    meshInfo._boundLocal.Transform ( _renderInfo._boundWorld, local );
    _renderInfo._color = meshInfo._colors;

    _hasChanges = false;

    Unlock ();
}

MeshInfo &MeshNode::GetMeshInfo () const noexcept
{
    AV_ASSERT ( _meshInfo )
    return *_meshInfo;
}

MeshNode::RenderInfo const &MeshNode::GetRenderInfo () const noexcept
{
    return _renderInfo;
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

    ColorData const c
    {
        ._emiR = emission._data[ 0U ],
        ._0rgb = std::bit_cast<uint32_t> ( color0._data ) >> 8U,
        ._emiB = static_cast<uint32_t> ( emission._data[ 1U ] ),
        ._1rgb = std::bit_cast<uint32_t> ( color1 ) >> 8U,
        ._emiG = static_cast<uint32_t> ( emission._data[ 2U ] ),
        ._2rgb = std::bit_cast<uint32_t> ( color2 ) >> 8U,
        ._0A = static_cast<uint32_t> ( color0._data[ 3U ] ),

        ._emiIntensity = static_cast<uint32_t> (
            convertFactor * std::clamp ( static_cast<double> ( emissionIntensity ), 0.0, maxIntensity )
        )
    };

    if ( !TryLock () ) [[unlikely]]
        return;

    _meshInfo->_colors = c;
    _hasChanges = true;

    Unlock ();
}

void MeshNode::SetRotation ( GXQuat const &rotation ) noexcept
{
    if ( !TryLock () ) [[unlikely]]
        return;

    _meshInfo->_rotation = rotation;
    _hasChanges = true;

    Unlock ();
}

void MeshNode::SetRotation ( GXMat3 const &rotation ) noexcept
{
    GXQuat const r ( rotation );

    if ( !TryLock () ) [[unlikely]]
        return;

    _meshInfo->_rotation = r;
    _hasChanges = true;

    Unlock ();
}

void MeshNode::SetRotation ( GXMat4 const &rotation ) noexcept
{
    GXQuat const r ( rotation );

    if ( !TryLock () ) [[unlikely]]
        return;

    _meshInfo->_rotation = r;
    _hasChanges = true;

    Unlock ();
}

void MeshNode::SetLocation ( GXVec3 const &location ) noexcept
{
    if ( !TryLock () ) [[unlikely]]
        return;

    _meshInfo->_location = location;
    _hasChanges = true;

    Unlock ();
}

void MeshNode::SetScale ( GXVec3 const &scale ) noexcept
{
    if ( !TryLock () ) [[unlikely]]
        return;

    _meshInfo->_scale = scale;
    _hasChanges = true;

    Unlock ();
}

void MeshNode::SetLocal ( GXMat4 const &local ) noexcept
{
    GXQuat const r ( local );

    GXVec3 s {};
    local.ClearScale ( s );

    if ( !TryLock () ) [[unlikely]]
        return;

    _meshInfo->_rotation = r;
    _meshInfo->_location = *reinterpret_cast<GXVec3 const*> ( local._data + 12U );
    _meshInfo->_scale = s;
    _hasChanges = true;

    Unlock ();
}

void MeshNode::SetLocal ( GXQuat const &rotation, GXVec3 const &location ) noexcept
{
    if ( !TryLock () ) [[unlikely]]
        return;

    _meshInfo->_rotation = rotation;
    _meshInfo->_location = location;
    _hasChanges = true;

    Unlock ();
}

void MeshNode::SetLocal ( GXQuat const &rotation, GXVec3 const &location, GXVec3 const &scale ) noexcept
{
    if ( !TryLock () ) [[unlikely]]
        return;

    _meshInfo->_rotation = rotation;
    _meshInfo->_location = location;
    _meshInfo->_scale = scale;
    _hasChanges = true;

    Unlock ();
}

void MeshNode::SetBounds ( GXAABB const &boundLocal ) noexcept
{
    _meshInfo->_boundLocal = boundLocal;
    _hasChanges = true;
}

void MeshNode::SetMaterial ( PBRMaterial const &material ) noexcept
{
    if ( !TryLock () ) [[unlikely]]
        return;

    _meshInfo->_material = material;
    _hasChanges = true;

    Unlock ();
}

} // namespace editor
