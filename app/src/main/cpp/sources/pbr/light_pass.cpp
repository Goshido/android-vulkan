#include <pbr/light_pass.h>
#include <trace.h>
#include <vulkan_utils.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>

GX_RESTORE_WARNING_STATE


namespace pbr {

bool LightPass::Init ( android_vulkan::Renderer &renderer,
    VkCommandPool commandPool,
    VkRenderPass renderPass,
    GBuffer &gBuffer
) noexcept
{
    if ( !_lightupCommonDescriptorSet.Init ( renderer, commandPool, gBuffer ) )
    {
        Destroy ( renderer.GetDevice () );
        return false;
    }

    VkExtent2D const& resolution = gBuffer.GetResolution ();

    if ( !_pointLightPass.Init ( renderer, commandPool, resolution, renderPass ) )
    {
        Destroy ( renderer.GetDevice () );
        return false;
    }

    if ( !_reflectionGlobalPass.Init ( renderer, renderPass, 1U, resolution ) )
    {
        Destroy ( renderer.GetDevice () );
        return false;
    }

    bool const result = _reflectionLocalPass.Init ( renderer, commandPool, renderPass, 1U, resolution );

    if ( result && CreateUnitCube ( renderer, commandPool ) )
        return true;

    Destroy ( renderer.GetDevice () );
    return false;
}

void LightPass::Destroy ( VkDevice device ) noexcept
{
    _unitCube.FreeResources ( device );

    _reflectionLocalPass.Destroy ( device );
    _reflectionGlobalPass.Destroy ( device );
    _pointLightPass.Destroy ( device );
    _lightupCommonDescriptorSet.Destroy ( device );

    if ( _transfer == VK_NULL_HANDLE )
        return;

    vkFreeCommandBuffers ( device, _commandPool, 1U, &_transfer );
    _transfer = VK_NULL_HANDLE;
    _commandPool = VK_NULL_HANDLE;
}

size_t LightPass::GetPointLightCount () const noexcept
{
    return _pointLightPass.GetPointLightCount ();
}

size_t LightPass::GetReflectionLocalCount () const noexcept
{
    return _reflectionLocalPass.GetReflectionLocalCount ();
}

void LightPass::OnFreeTransferResources ( VkDevice device ) noexcept
{
    _unitCube.FreeTransferResources ( device );
    _lightupCommonDescriptorSet.OnFreeTransferResources ( device );

    if ( _transfer == VK_NULL_HANDLE )
        return;

    vkFreeCommandBuffers ( device, _commandPool, 1U, &_transfer );
    _transfer = VK_NULL_HANDLE;
    _commandPool = VK_NULL_HANDLE;
}

bool LightPass::OnPreGeometryPass ( android_vulkan::Renderer &renderer,
    VkExtent2D const &resolution,
    SceneData const &sceneData,
    size_t opaqueMeshCount,
    GXMat4 const &viewerLocal,
    GXMat4 const &view,
    GXMat4 const &viewProjection,
    GXMat4 const &cvvToView
) noexcept
{
    AV_TRACE ( "Light pre-geometry" )

    if ( !_lightupCommonDescriptorSet.Update ( renderer, resolution, viewerLocal, cvvToView ) )
        return false;

    if ( !_pointLightPass.ExecuteShadowPhase ( renderer, sceneData, opaqueMeshCount ) )
        return false;

    return _reflectionLocalPass.UploadGPUData ( renderer, view, viewProjection );
}

bool LightPass::OnPostGeometryPass ( android_vulkan::Renderer &renderer,
    VkCommandBuffer commandBuffer,
    GXMat4 const &viewerLocal,
    GXMat4 const &view,
    GXMat4 const &viewProjection
) noexcept
{
    AV_TRACE ( "Light post-geometry" )

    size_t const pointLights = _pointLightPass.GetPointLightCount ();
    size_t const localReflections = _reflectionLocalPass.GetReflectionLocalCount ();
    size_t const globalReflections = _reflectionGlobalPass.GetReflectionCount ();
    size_t const lightVolumes = pointLights + localReflections;

    if ( lightVolumes + globalReflections == 0U )
        return true;

    _lightupCommonDescriptorSet.Bind ( commandBuffer );

    if ( pointLights )
    {
        bool const result = _pointLightPass.ExecuteLightupPhase ( renderer,
            _unitCube,
            commandBuffer,
            viewerLocal,
            view,
            viewProjection
        );

        if ( !result )
        {
            return false;
        }
    }

    if ( localReflections && !_reflectionLocalPass.Execute ( renderer, _unitCube, commandBuffer ) )
        return false;

    if ( !globalReflections )
        return true;

    bool const result = _reflectionGlobalPass.Execute ( renderer, commandBuffer, viewerLocal );
    return result;
}

void LightPass::Reset () noexcept
{
    _pointLightPass.Reset ();
    _reflectionGlobalPass.Reset ();
    _reflectionLocalPass.Reset ();
}

void LightPass::SubmitPointLight ( LightRef const &light ) noexcept
{
    _pointLightPass.Submit ( light );
}

void LightPass::SubmitReflectionGlobal ( TextureCubeRef &prefilter ) noexcept
{
    _reflectionGlobalPass.Append ( prefilter );
}

void LightPass::SubmitReflectionLocal ( TextureCubeRef &prefilter, GXVec3 const &location, float size ) noexcept
{
    _reflectionLocalPass.Append ( prefilter, location, size );
}

bool LightPass::CreateUnitCube ( android_vulkan::Renderer &renderer, VkCommandPool commandPool ) noexcept
{
    _commandPool = commandPool;

    VkCommandBufferAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = _commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1U
    };

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateCommandBuffers ( renderer.GetDevice (), &allocateInfo, &_transfer ),
        "pbr::LightPass::CreateUnitCube",
        "Can't allocate command buffer"
    );

    if ( !result )
    {
        Destroy ( renderer.GetDevice () );
        return false;
    }

    constexpr GXVec3 const vertices[] =
    {
        GXVec3 ( -0.5F, -0.5F, -0.5F ),
        GXVec3 ( 0.5F, -0.5F, -0.5F ),
        GXVec3 ( -0.5F, 0.5F, -0.5F ),
        GXVec3 ( 0.5F, 0.5F, -0.5F ),
        GXVec3 ( 0.5F, -0.5F, 0.5F ),
        GXVec3 ( -0.5F, -0.5F, 0.5F ),
        GXVec3 ( 0.5F, 0.5F, 0.5F ),
        GXVec3 ( -0.5F, 0.5F, 0.5F )
    };

    constexpr uint32_t const indices[] =
    {
        0U, 2U, 1U,
        1U, 2U, 3U,
        1U, 3U, 4U,
        4U, 3U, 6U,
        4U, 6U, 5U,
        5U, 6U, 7U,
        5U, 7U, 0U,
        0U, 7U, 2U,
        2U, 7U, 3U,
        3U, 7U, 6U,
        5U, 0U, 4U,
        4U, 0U, 1U
    };

    GXAABB bounds;
    bounds.AddVertex ( vertices[ 0U ] );
    bounds.AddVertex ( vertices[ 7U ] );

    return _unitCube.LoadMesh ( reinterpret_cast<uint8_t const*> ( vertices ),
        sizeof ( vertices ),
        indices,
        static_cast<uint32_t> ( std::size ( indices ) ),
        bounds,
        renderer,
        _transfer
    );
}

} // namespace pbr
