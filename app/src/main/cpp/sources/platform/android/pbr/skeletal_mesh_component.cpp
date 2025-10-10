#include <precompiled_headers.hpp>
#include <av_assert.hpp>
#include <logger.hpp>
#include <pbr/scriptable_gxmat4.hpp>
#include <pbr/scriptable_gxvec3.hpp>
#include <pbr/scriptable_gxvec4.hpp>
#include <platform/android/pbr/material_manager.hpp>
#include <platform/android/pbr/mesh_manager.hpp>
#include <platform/android/pbr/scriptable_material.hpp>
#include <platform/android/pbr/skeletal_mesh_component.hpp>
#include <platform/android/pbr/skin.inc>

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lauxlib.h>

} // extern "C"

GX_RESTORE_WARNING_STATE


namespace pbr {

namespace {

constexpr GXColorUNORM DEFAULT_COLOR ( 255U, 255U, 255U, 255U );
constexpr GXColorUNORM DEFAULT_EMISSION ( 255U, 255U, 255U, 255U );

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

size_t SkeletalMeshComponent::_lastCommandBufferIndex = 0U;
SkinProgram SkeletalMeshComponent::_program {};
android_vulkan::Renderer* SkeletalMeshComponent::_renderer = nullptr;
SkeletalMeshComponent::SkeletalMeshes SkeletalMeshComponent::_skeletalMeshes {};
SkinPool SkeletalMeshComponent::_skinPool {};
std::list<SkeletalMeshComponent::Usage> SkeletalMeshComponent::_toDelete[ FIF_COUNT ] {};

// NOLINTNEXTLINE - no initialization for some fields
SkeletalMeshComponent::SkeletalMeshComponent ( bool &success,
    char const* mesh,
    char const* skin,
    char const* skeleton,
    std::string &&name
) noexcept:
    RenderableComponent ( ClassID::SkeletalMesh, std::move ( name ) ),
    _color0 ( DEFAULT_COLOR ),
    _color1 ( DEFAULT_COLOR ),
    _color2 ( DEFAULT_COLOR ),
    _emission ( DEFAULT_EMISSION )
{
    success = false;
    _localMatrix.Identity ();
    size_t consumed;

    // Default material already loaded
    _material = MaterialManager::GetInstance ().LoadMaterial ( *_renderer,
        consumed,
        MaterialManager::DEFAULT_MATERIAL,
        nullptr,
        nullptr
    );

    if ( !_material ) [[unlikely]]
        return;

    if ( _referenceMesh = MeshManager::GetInstance ().LoadMesh ( *_renderer, mesh ); !_referenceMesh ) [[unlikely]]
        return;

    MeshRef skinMesh = std::make_shared<android_vulkan::MeshGeometry> ();

    if ( !skinMesh->LoadMesh ( *_renderer, mesh ) ) [[unlikely]]
        return;

    skinMesh->MakeUnique ();

    _usage._skinMesh = std::move ( skinMesh );

    bool const result = _usage._skinData.LoadSkin ( skin,
        skeleton,
        *_renderer
    );

    if ( !result ) [[unlikely]]
        return;

    VkExtent3D const &maxDispatch = _renderer->GetMaxComputeDispatchSize ();
    uint32_t const vertices = _referenceMesh->GetVertexCount ();
    uint32_t const totalGroups = ( vertices + THREADS_PER_GROUP ) / THREADS_PER_GROUP;

    uint32_t rest = totalGroups / maxDispatch.width;
    rest /= maxDispatch.height;
    rest /= maxDispatch.depth;

    if ( rest > 0U ) [[unlikely]]
    {
        android_vulkan::LogWarning ( "pbr::SkeletalMeshComponent::SkeletalMeshComponent - Mesh '%s' can't fit to "
             "skin dispatch call. Too many vertices %u for hardware.",
            _referenceMesh->GetName ().c_str (),
            vertices
        );

        return;
    }

    if ( totalGroups < maxDispatch.width ) [[likely]]
    {
        _dispatch = VkExtent3D
        {
            .width = totalGroups,
            .height = 1U,
            .depth = 1U
        };

        success = true;
        return;
    }

    android_vulkan::LogWarning ( "pbr::SkeletalMeshComponent::SkeletalMeshComponent - More that on group row! "
        "Figure out how to implement this!"
    );

    AV_ASSERT ( false )
}

void SkeletalMeshComponent::RegisterFromScript ( Actor &actor ) noexcept
{
    _actor = &actor;
}

void SkeletalMeshComponent::Unregister () noexcept
{
    if ( _usage._skinMesh )
    {
        _toDelete[ _lastCommandBufferIndex ].emplace_back ( std::move ( _usage ) );
    }
}

bool SkeletalMeshComponent::ApplySkin ( VkCommandBuffer commandBuffer, size_t commandBufferIndex ) noexcept
{
    AV_VULKAN_GROUP ( commandBuffer, "Skinning" )

    if ( !_renderer ) [[unlikely]]
        return true;

    _lastCommandBufferIndex = commandBufferIndex;

    FreeUnusedResources ( commandBufferIndex );
    VkDevice device = _renderer->GetDevice ();
    bool hasUpdates = false;

    for ( auto &item : _skeletalMeshes )
    {
        // NOLINTNEXTLINE - downcast.
        auto &mesh = *const_cast<SkeletalMeshComponent*> ( static_cast<SkeletalMeshComponent const*> ( item.first ) );

        if ( !mesh._animationGraph )
            continue;

        hasUpdates = true;
        Usage &usage = mesh._usage;

        _skinPool.Push ( mesh._animationGraph->GetPoseInfo (),
            usage._skinData.GetSkinInfo (),
            mesh._referenceMesh->GetMeshBufferInfo (),
            usage._skinMesh->GetMeshBufferInfo ()._buffer
        );
    }

    if ( !hasUpdates )
        return true;

    _skinPool.UpdateDescriptorSets ( device );
    _program.Bind ( commandBuffer );
    SkinProgram::PushConstants pushConstants {};

    for ( auto &item : _skeletalMeshes )
    {
        // NOLINTNEXTLINE - downcast.
        auto &mesh = *const_cast<SkeletalMeshComponent*> ( static_cast<SkeletalMeshComponent const*> ( item.first ) );

        if ( !mesh._animationGraph )
            continue;

        pushConstants._vertexCount = mesh._referenceMesh->GetVertexBufferVertexCount ();
        _program.SetDescriptorSet ( commandBuffer, _skinPool.Acquire () );
        _program.SetPushConstants ( commandBuffer, &pushConstants );

        VkExtent3D const &d = mesh._dispatch;
        vkCmdDispatch ( commandBuffer, d.width, d.height, d.depth );
    }

    _skinPool.SubmitPipelineBarriers ( commandBuffer );
    return true;
}

void SkeletalMeshComponent::FreeUnusedResources ( size_t commandBufferIndex ) noexcept
{
    auto &toDelete = _toDelete[ commandBufferIndex ];

    for ( Usage &usage : toDelete )
    {
        usage._skinMesh->FreeResources ( *_renderer );
        usage._skinData.FreeResources ( *_renderer );
    }

    toDelete.clear ();
}

bool SkeletalMeshComponent::Init ( lua_State &vm, android_vulkan::Renderer &renderer ) noexcept
{
    constexpr luaL_Reg const extensions[] =
    {
        {
            .name = "av_SkeletalMeshComponentCreate",
            .func = &SkeletalMeshComponent::OnCreate
        },
        {
            .name = "av_SkeletalMeshComponentDestroy",
            .func = &SkeletalMeshComponent::OnDestroy
        },
        {
            .name = "av_SkeletalMeshComponentCollectGarbage",
            .func = &SkeletalMeshComponent::OnGarbageCollected
        },
        {
            .name = "av_SkeletalMeshComponentGetLocal",
            .func = &SkeletalMeshComponent::OnGetLocal
        },
        {
            .name = "av_SkeletalMeshComponentSetAnimationGraph",
            .func = &SkeletalMeshComponent::OnSetAnimationGraph
        },
        {
            .name = "av_SkeletalMeshComponentSetColor0",
            .func = &SkeletalMeshComponent::OnSetColor0
        },
        {
            .name = "av_SkeletalMeshComponentSetColor1",
            .func = &SkeletalMeshComponent::OnSetColor1
        },
        {
            .name = "av_SkeletalMeshComponentSetColor2",
            .func = &SkeletalMeshComponent::OnSetColor2
        },
        {
            .name = "av_SkeletalMeshComponentSetEmission",
            .func = &SkeletalMeshComponent::OnSetEmission
        },
        {
            .name = "av_SkeletalMeshComponentSetLocal",
            .func = &SkeletalMeshComponent::OnSetLocal
        },
        {
            .name = "av_SkeletalMeshComponentSetMaterial",
            .func = &SkeletalMeshComponent::OnSetMaterial
        }
    };

    for ( auto const &extension : extensions )
        lua_register ( &vm, extension.name, extension.func );

    _renderer = &renderer;
    return _program.Init ( renderer, nullptr ) && _skinPool.Init ( renderer.GetDevice () );
}

void SkeletalMeshComponent::Destroy () noexcept
{
    if ( !_skeletalMeshes.empty () )
    {
        android_vulkan::LogWarning ( "pbr::SkeletalMeshComponent::Destroy - Memory leak." );
        AV_ASSERT ( false )
    }

    FreeUnusedResources ( 0U );
    FreeUnusedResources ( 1U );

    VkDevice device = _renderer->GetDevice ();

    _skinPool.Destroy ( device );
    _program.Destroy ( device );

    _lastCommandBufferIndex = 0U;
    _renderer = nullptr;
}

ComponentRef &SkeletalMeshComponent::GetReference () noexcept
{
    auto findResult = _skeletalMeshes.find ( this );
    AV_ASSERT ( findResult != _skeletalMeshes.end () )
    return findResult->second;
}

void SkeletalMeshComponent::FreeTransferResources ( android_vulkan::Renderer &renderer ) noexcept
{
    if ( !_material )
        return;

    // NOLINTNEXTLINE - downcast.
    auto &m = static_cast<GeometryPassMaterial &> ( *_material );

    if ( m.GetAlbedo () )
        m.GetAlbedo ()->FreeTransferResources ( renderer );

    if ( m.GetEmission () )
        m.GetEmission ()->FreeTransferResources ( renderer );

    if ( m.GetNormal () )
        m.GetNormal ()->FreeTransferResources ( renderer );

    if ( !m.GetParam () )
        return;

    m.GetParam ()->FreeTransferResources ( renderer );
}

void SkeletalMeshComponent::Submit ( RenderSession &renderSession ) noexcept
{
    UpdateColorData ();
    renderSession.SubmitMesh ( _usage._skinMesh, _material, _localMatrix, _worldBounds, *_colorData );
}

void SkeletalMeshComponent::OnTransform ( GXMat4 const &transformWorld ) noexcept
{
    SetTransform ( transformWorld );
}

bool SkeletalMeshComponent::SetAnimationGraph ( AnimationGraph &animationGraph ) noexcept
{
    if ( _usage._skinData.GetMinPoseRange () <= animationGraph.GetPoseRange () ) [[likely]]
    {
        _animationGraph = &animationGraph;
        return true;
    }

    android_vulkan::LogWarning ( "pbr::SkeletalMeshComponent::SetAnimationGraph - Can't set animation graph. "
        "Skeletal mesh '%s' is incompatible with '%s' skeleton.",
        _referenceMesh->GetName ().c_str (),
        animationGraph.GetSkeletonName ().c_str ()
    );

    return false;
}

void SkeletalMeshComponent::SetColor0 ( GXColorUNORM color ) noexcept
{
    _color0 = color;
    _colorData = std::nullopt;
}

void SkeletalMeshComponent::SetColor1 ( GXColorUNORM color ) noexcept
{
    _color1 = color;
    _colorData = std::nullopt;
}

void SkeletalMeshComponent::SetColor2 ( GXColorUNORM color ) noexcept
{
    _color2 = color;
    _colorData = std::nullopt;
}

void SkeletalMeshComponent::SetEmission ( GXColorUNORM color, float intensity ) noexcept
{
    _emission = color;
    _emissionIntensity = intensity;
    _colorData = std::nullopt;
}

void SkeletalMeshComponent::SetTransform ( GXMat4 const &transform ) noexcept
{
    _localMatrix = transform;
    _usage._skinMesh->GetBounds ().Transform ( _worldBounds, transform );
}

void SkeletalMeshComponent::UpdateColorData () noexcept
{
    if ( !_colorData ) [[unlikely]]
    {
        _colorData = GeometryPassProgram::ColorData ( _color0, _color1, _color2, _emission, _emissionIntensity );
    }
}

int SkeletalMeshComponent::OnCreate ( lua_State* state )
{
    if ( !lua_checkstack ( state, 1 ) )
    {
        android_vulkan::LogWarning ( "pbr::SkeletalMeshComponent::OnCreate - Stack is too small." );
        return 0;
    }

    char const* name = lua_tostring ( state, 1 );

    if ( !name ) [[unlikely]]
    {
        lua_pushnil ( state );
        return 1;
    }

    char const* mesh = lua_tostring ( state, 2 );

    if ( !mesh ) [[unlikely]]
    {
        lua_pushnil ( state );
        return 1;
    }

    char const* skin = lua_tostring ( state, 3 );

    if ( !skin ) [[unlikely]]
    {
        lua_pushnil ( state );
        return 1;
    }

    char const* skeleton = lua_tostring ( state, 4 );

    if ( !skeleton ) [[unlikely]]
    {
        lua_pushnil ( state );
        return 1;
    }

    bool success;

    ComponentRef component = std::make_shared<SkeletalMeshComponent> ( success,
        mesh,
        skin,
        skeleton,
        name
    );

    if ( !success ) [[unlikely]]
    {
        lua_pushnil ( state );
        return 1;
    }

    Component* handle = component.get ();
    _skeletalMeshes.emplace ( handle, std::move ( component ) );

    lua_pushlightuserdata ( state, handle );
    return 1;
}

int SkeletalMeshComponent::OnDestroy ( lua_State* state )
{
    auto &self = *static_cast<SkeletalMeshComponent*> ( lua_touserdata ( state, 1 ) );
    self._actor->DestroyComponent ( self );
    return 0;
}

int SkeletalMeshComponent::OnGarbageCollected ( lua_State* state )
{
    auto findResult = _skeletalMeshes.find ( static_cast<Component*> ( lua_touserdata ( state, 1 ) ) );

    // NOLINTNEXTLINE - downcast.
    auto &self = static_cast<SkeletalMeshComponent &> ( *findResult->second );
    self.Unregister ();

    _skeletalMeshes.erase ( findResult );
    return 0;
}

int SkeletalMeshComponent::OnSetAnimationGraph ( lua_State* state )
{
    if ( !lua_checkstack ( state, 1 ) ) [[unlikely]]
    {
        android_vulkan::LogWarning ( "pbr::SkeletalMeshComponent::OnSetAnimationGraph - Stack is too small." );
        return 0;
    }

    auto &self = *static_cast<SkeletalMeshComponent*> ( lua_touserdata ( state, 1 ) );
    lua_pushboolean ( state, self.SetAnimationGraph ( *static_cast<AnimationGraph*> ( lua_touserdata ( state, 2 ) ) ) );
    return 1;
}

int SkeletalMeshComponent::OnSetColor0 ( lua_State* state )
{
    auto &self = *static_cast<SkeletalMeshComponent*> ( lua_touserdata ( state, 1 ) );
    auto const &color = *reinterpret_cast<GXColorRGB const *> ( &ScriptableGXVec4::Extract ( state, 2 ) );
    self.SetColor0 ( color.ToColorUNORM () );
    return 0;
}

int SkeletalMeshComponent::OnSetColor1 ( lua_State* state )
{
    auto &self = *static_cast<SkeletalMeshComponent*> ( lua_touserdata ( state, 1 ) );
    GXVec3 const &color = ScriptableGXVec3::Extract ( state, 2 );
    self.SetColor1 ( GXColorRGB ( color._data[ 0U ], color._data[ 1U ], color._data[ 2U ], 1.0F ).ToColorUNORM () );
    return 0;
}

int SkeletalMeshComponent::OnSetColor2 ( lua_State* state )
{
    auto &self = *static_cast<SkeletalMeshComponent*> ( lua_touserdata ( state, 1 ) );
    GXVec3 const &color = ScriptableGXVec3::Extract ( state, 2 );
    self.SetColor2 ( GXColorRGB ( color._data[ 0U ], color._data[ 1U ], color._data[ 2U ], 1.0F ).ToColorUNORM () );
    return 0;
}

int SkeletalMeshComponent::OnSetEmission ( lua_State* state )
{
    auto &self = *static_cast<SkeletalMeshComponent*> ( lua_touserdata ( state, 1 ) );
    GXVec3 emission = ScriptableGXVec3::Extract ( state, 2 );
    float const maxComp = std::max ( { emission._data[ 0U ], emission._data[ 1U ], emission._data[ 2U ] } );

    if ( maxComp == 0.0F )
    {
        self.SetEmission ( GXColorUNORM ( 0U, 0U, 0U, 0U ), 0.0F );
        return 0;
    }

    emission.Multiply ( emission, 1.0F / maxComp );

    self.SetEmission (
        GXColorRGB ( emission._data[ 0U ], emission._data[ 1U ], emission._data[ 2U ], 1.0F ).ToColorUNORM (),
        maxComp
    );

    return 0;
}

int SkeletalMeshComponent::OnGetLocal ( lua_State* state )
{
    auto const &self = *static_cast<SkeletalMeshComponent const*> ( lua_touserdata ( state, 1 ) );
    ScriptableGXMat4::Extract ( state, 2 ) = self._localMatrix;
    return 0;
}

int SkeletalMeshComponent::OnSetLocal ( lua_State* state )
{
    auto &self = *static_cast<SkeletalMeshComponent*> ( lua_touserdata ( state, 1 ) );
    self.SetTransform ( ScriptableGXMat4::Extract ( state, 2 ) );
    return 0;
}

int SkeletalMeshComponent::OnSetMaterial ( lua_State* state )
{
    auto &self = *static_cast<SkeletalMeshComponent*> ( lua_touserdata ( state, 1 ) );

    self._material = ScriptableMaterial::GetReference (
        *static_cast<Material const *> ( lua_touserdata ( state, 2 ) )
    );

    return 0;
}

} // namespace pbr
