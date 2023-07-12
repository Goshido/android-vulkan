#include <pbr/light_pass.h>
#include <trace.h>
#include <vulkan_utils.h>


namespace pbr {

bool LightPass::Init ( android_vulkan::Renderer &renderer,
    VkRenderPass renderPass,
    GBuffer &gBuffer
) noexcept
{
    VkCommandPoolCreateInfo const createInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .queueFamilyIndex = renderer.GetQueueFamilyIndex ()
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateCommandPool ( renderer.GetDevice (), &createInfo, nullptr, &_commandPool ),
        "pbr::LightPass::Init",
        "Can't create lead command pool"
    );

    if ( !result )
        return false;

    AV_REGISTER_COMMAND_POOL ( "pbr::LightPass::_commandPool" )

    VkExtent2D const &resolution = gBuffer.GetResolution ();

    return _pointLightPass.Init ( renderer, resolution, renderPass ) &&
        _reflectionGlobalPass.Init ( renderer, renderPass, 1U, resolution ) &&
        _reflectionLocalPass.Init ( renderer, renderPass, 1U, resolution ) &&
        _lightupCommonDescriptorSet.Init ( renderer, _commandPool, gBuffer ) &&
        CreateUnitCube ( renderer ) &&

        _volumeBufferPool.Init ( renderer,
            LightVolumeDescriptorSetLayout {},
            sizeof ( PointLightLightupProgram::VolumeData ),
            0U,
            "pbr::LightPass::_lightVolumeBufferPool"
        );
}

void LightPass::Destroy ( android_vulkan::Renderer &renderer ) noexcept
{
    VkDevice device = renderer.GetDevice ();

    _volumeBufferPool.Destroy ( renderer, "pbr::LightPass::_lightVolumeBufferPool" );
    _unitCube.FreeResources ( renderer );

    _reflectionLocalPass.Destroy ( renderer );
    _reflectionGlobalPass.Destroy ( device );
    _pointLightPass.Destroy ( renderer );
    _lightupCommonDescriptorSet.Destroy ( renderer );

    if ( _commandPool == VK_NULL_HANDLE )
        return;

    vkDestroyCommandPool ( device, _commandPool, nullptr );
    _commandPool = VK_NULL_HANDLE;
    AV_UNREGISTER_COMMAND_POOL ( "pbr::LightPass::_commandPool" )
}

size_t LightPass::GetPointLightCount () const noexcept
{
    return _pointLightPass.GetPointLightCount ();
}

size_t LightPass::GetReflectionLocalCount () const noexcept
{
    return _reflectionLocalPass.GetReflectionLocalCount ();
}

void LightPass::OnFreeTransferResources ( android_vulkan::Renderer &renderer ) noexcept
{
    _unitCube.FreeTransferResources ( renderer );
    _lightupCommonDescriptorSet.OnFreeTransferResources ( renderer );

    if ( _commandPool == VK_NULL_HANDLE )
        return;

    vkDestroyCommandPool ( renderer.GetDevice (), _commandPool, nullptr );
    _commandPool = VK_NULL_HANDLE;
    AV_UNREGISTER_COMMAND_POOL ( "pbr::LightPass::_commandPool" )
}

bool LightPass::OnPreGeometryPass ( android_vulkan::Renderer &renderer,
    VkCommandBuffer commandBuffer,
    size_t swapchainImageIndex,
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

    VkDevice device = renderer.GetDevice ();

    _lightupCommonDescriptorSet.Update ( device,
        commandBuffer,
        swapchainImageIndex,
        resolution,
        viewerLocal,
        cvvToView
    );

    if ( !_pointLightPass.ExecuteShadowPhase ( renderer, commandBuffer, sceneData, opaqueMeshCount ) )
        return false;

    _pointLightPass.UploadGPUData ( device,
        commandBuffer,
        _volumeBufferPool,
        viewerLocal,
        view,
        viewProjection
    );

    _reflectionLocalPass.UploadGPUData ( device, commandBuffer, _volumeBufferPool, view, viewProjection );
    _volumeBufferPool.IssueSync ( device, commandBuffer );

    return true;
}

void LightPass::OnPostGeometryPass ( VkDevice device,
    VkCommandBuffer commandBuffer,
    size_t swapchainImageIndex
) noexcept
{
    AV_TRACE ( "Light post-geometry" )

    size_t const pointLights = _pointLightPass.GetPointLightCount ();
    size_t const localReflections = _reflectionLocalPass.GetReflectionLocalCount ();
    size_t const globalReflections = _reflectionGlobalPass.GetReflectionCount ();
    size_t const lightVolumes = pointLights + localReflections;

    if ( lightVolumes + globalReflections == 0U )
        return;

    _lightupCommonDescriptorSet.Bind ( commandBuffer, swapchainImageIndex );

    if ( pointLights )
        _pointLightPass.ExecuteLightupPhase ( commandBuffer, _unitCube, _volumeBufferPool );

    if ( localReflections )
        _reflectionLocalPass.Execute ( commandBuffer, _unitCube, _volumeBufferPool );

    if ( !globalReflections )
    {
        _volumeBufferPool.Commit ();
        return;
    }

    _reflectionGlobalPass.Execute ( device, commandBuffer );
    _volumeBufferPool.Commit ();
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

bool LightPass::CreateUnitCube ( android_vulkan::Renderer &renderer ) noexcept
{
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

    VkCommandBufferAllocateInfo const bufferAllocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = _commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1U
    };

    VkCommandBuffer commandBuffer;

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateCommandBuffers ( renderer.GetDevice (), &bufferAllocateInfo, &commandBuffer ),
        "pbr::CreateUnitCube::CreateUnitCube",
        "Can't allocate command buffer"
    );

    if ( !result )
        return false;

    return _unitCube.LoadMesh ( reinterpret_cast<uint8_t const*> ( vertices ),
        sizeof ( vertices ),
        indices,
        static_cast<uint32_t> ( std::size ( indices ) ),
        bounds,
        renderer,
        commandBuffer,
        VK_NULL_HANDLE
    );
}

} // namespace pbr
