#include <precompiled_headers.hpp>
#include <mesh_node.hpp>
#include <workspace.hpp>


namespace editor {

MeshNode::MeshNode ( MeshNode &&other ) noexcept
{
    bool const myLock = TryLock ();
    Disconnect ();

    bool const otherLock = other.TryLock ();

    _workspace = std::exchange ( other._workspace, nullptr );
    _hasChanges = std::exchange ( other._hasChanges, false );

    if ( _meshInfo = std::exchange ( other._meshInfo, nullptr ); _meshInfo )
        _meshInfo->_node = this;

    _colors = std::exchange ( other._colors, {} );
    _rotation = std::exchange ( other._rotation, GXQuat::IDENTITY );
    _location = std::exchange ( other._location, GXVec3::ZERO );
    _scale = std::exchange ( other._scale, GXVec3::ONE );
    _boundLocal = std::exchange ( other._boundLocal, {} );
    _id = std::exchange ( other._id, std::bit_cast<uint64_t> ( nullptr ) );

    if ( otherLock )
        other.Unlock ();

    if ( myLock )
    {
        Unlock ();
    }
}

MeshNode &MeshNode::operator = ( MeshNode &&other ) noexcept
{
    if ( this == &other ) [[unlikely]]
        return *this;

    bool const locked = TryLock ();
    Disconnect ();

    bool const otherLock = other.TryLock ();
    _workspace = std::exchange ( other._workspace, nullptr );
    _hasChanges = std::exchange ( other._hasChanges, false );

    if ( _meshInfo = std::exchange ( other._meshInfo, nullptr ); _meshInfo )
        _meshInfo->_node = this;

    _colors = std::exchange ( other._colors, {} );
    _rotation = std::exchange ( other._rotation, GXQuat::IDENTITY );
    _location = std::exchange ( other._location, GXVec3::ZERO );
    _scale = std::exchange ( other._scale, GXVec3::ONE );
    _boundLocal = std::exchange ( other._boundLocal, {} );
    _id = std::exchange ( other._id, std::bit_cast<uint64_t> ( nullptr ) );

    if ( otherLock )
        other.Unlock ();

    if ( locked )
        Unlock ();

    return *this;
}

MeshNode::MeshNode ( Workspace &workspace, MeshInfo &meshInfo ) noexcept:
    WorkspaceNode ( workspace ),
    _meshInfo ( &meshInfo )
{
    meshInfo._node = this;
}

MeshNode::~MeshNode () noexcept
{
    std::ignore = TryLock ();

    if ( !_workspace )
        return;

    Disconnect ();
}

void MeshNode::Commit ( uint32_t defaultAlbedo,
    uint32_t defaultEmission,
    uint32_t defaultMask,
    uint32_t defaultParam,
    uint32_t defaultNormal
) noexcept
{
    MeshInfo &meshInfo = *_meshInfo;

    if ( !_hasChanges || !TryLock () ) [[likely]]
        return;

    GXMat4 local {};
    local.FromFast ( _rotation, _location );
    auto &x = *reinterpret_cast<GXVec3*> ( local._data[ 0U ] );
    GXVec3 const &s = _scale;

    auto &y = *reinterpret_cast<GXVec3*> ( local._data[ 1U ] );
    x.Multiply ( x, s._data[ 0U ] );

    auto &z = *reinterpret_cast<GXVec3*> ( local._data[ 2U ] );
    y.Multiply ( y, s._data[ 1U ] );
    z.Multiply ( z, s._data[ 2U ] );

    meshInfo._transform =
    {
        ._model
        {
            ._x = x,
            ._y = y,
            ._z = z,
            ._w = *reinterpret_cast<GXVec3*> ( local._data[ 3U ] ),
        },

        ._normal = _rotation.ToTBN64 ()
    };

    _boundLocal.Transform ( meshInfo._boundWorld, local );

    meshInfo._shading =
    {
        ._albedo = !_material._albedo ? defaultAlbedo : *_material._albedo->_sampledIndex,
        ._emission = !_material._emission ? defaultEmission : *_material._emission->_sampledIndex,
        ._mask = !_material._mask ? defaultMask : *_material._mask->_sampledIndex,
        ._param = !_material._param ? defaultParam : *_material._param->_sampledIndex,
        ._normal = !_material._normal ? defaultNormal : *_material._normal->_sampledIndex,
        ._colors = _colors
    };

    meshInfo._id = _id;
    _hasChanges = false;
    Unlock ();
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

    _colors = c;
    _hasChanges = true;

    Unlock ();
}

void MeshNode::SetRotation ( GXQuat const &rotation ) noexcept
{
    if ( !TryLock () ) [[unlikely]]
        return;

    _rotation = rotation;
    _hasChanges = true;

    Unlock ();
}

void MeshNode::SetRotation ( GXMat3 const &rotation ) noexcept
{
    GXQuat const r ( rotation );

    if ( !TryLock () ) [[unlikely]]
        return;

    _rotation = r;
    _hasChanges = true;

    Unlock ();
}

void MeshNode::SetRotation ( GXMat4 const &rotation ) noexcept
{
    GXQuat const r ( rotation );

    if ( !TryLock () ) [[unlikely]]
        return;

    _rotation = r;
    _hasChanges = true;

    Unlock ();
}

void MeshNode::SetLocation ( GXVec3 const &location ) noexcept
{
    if ( !TryLock () ) [[unlikely]]
        return;

    _location = location;
    _hasChanges = true;

    Unlock ();
}

void MeshNode::SetScale ( GXVec3 const &scale ) noexcept
{
    if ( !TryLock () ) [[unlikely]]
        return;

    _scale = scale;
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

    _rotation = r;
    _location = *reinterpret_cast<GXVec3 const*> ( local._data[ 3U ] );
    _scale = s;
    _hasChanges = true;

    Unlock ();
}

void MeshNode::SetLocal ( GXQuat const &rotation, GXVec3 const &location ) noexcept
{
    if ( !TryLock () ) [[unlikely]]
        return;

    _rotation = rotation;
    _location = location;
    _hasChanges = true;

    Unlock ();
}

void MeshNode::SetLocal ( GXQuat const &rotation, GXVec3 const &location, GXVec3 const &scale ) noexcept
{
    if ( !TryLock () ) [[unlikely]]
        return;

    _rotation = rotation;
    _location = location;
    _scale = scale;
    _hasChanges = true;

    Unlock ();
}

void MeshNode::SetBounds ( GXAABB const &boundLocal ) noexcept
{
    _boundLocal = boundLocal;
    _hasChanges = true;
}

void MeshNode::SetMaterial ( PBRMaterial const &material ) noexcept
{
    if ( !TryLock () ) [[unlikely]]
        return;

    _material = material;
    _hasChanges = true;

    Unlock ();
}

void MeshNode::SetID ( void const* id ) noexcept
{
    if ( !TryLock () ) [[unlikely]]
        return;

    _id = std::bit_cast<uint64_t> ( id );
    _hasChanges = true;

    Unlock ();
}

void MeshNode::Disconnect () noexcept
{
    if ( !_workspace )
        return;

    switch ( _meshInfo->_material )
    {
        case eMaterial::Opaque:
            _workspace->UnregisterOpaque ( *this );
        break;

        case eMaterial::Outline:
            _workspace->UnregisterOutline ( *this );
        break;

        case eMaterial::Stipple:
            _workspace->UnregisterStipple ( *this );
        break;
    }
}

} // namespace editor
