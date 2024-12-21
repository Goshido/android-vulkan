#include <precompiled_headers.hpp>
#include <pbr/light_pass.hpp>
#include <trace.hpp>
#include <vulkan_utils.hpp>


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

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( renderer.GetDevice (), _commandPool, VK_OBJECT_TYPE_COMMAND_POOL, "Light pass" )

    VkExtent2D const &resolution = gBuffer.GetResolution ();

    return _pointLightPass.Init ( renderer, resolution, renderPass ) &&
        _dummyLightProgram.Init ( renderer, renderPass, resolution ) &&
        _reflectionGlobalPass.Init ( renderer, renderPass, resolution ) &&
        _reflectionLocalPass.Init ( renderer, renderPass, resolution ) &&
        _lightupCommonDescriptorSet.Init ( renderer, _commandPool, gBuffer ) &&
        CreateUnitCube ( renderer ) &&

        _volumeDataPool.Init ( renderer,
            LightVolumeDescriptorSetLayout {},
            eUniformSize::Nanoscopic_64KB,
            sizeof ( VolumeData ),
            0U,
            "Light volume"
        );
}

void LightPass::Destroy ( android_vulkan::Renderer &renderer ) noexcept
{
    VkDevice device = renderer.GetDevice ();

    _dummyLightProgram.Destroy ( device );
    _volumeDataPool.Destroy ( renderer );
    _unitCube.FreeResources ( renderer );

    _reflectionLocalPass.Destroy ( renderer );
    _reflectionGlobalPass.Destroy ( device );
    _pointLightPass.Destroy ( renderer );
    _lightupCommonDescriptorSet.Destroy ( renderer );

    if ( _commandPool == VK_NULL_HANDLE )
        return;

    vkDestroyCommandPool ( device, _commandPool, nullptr );
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

void LightPass::OnFreeTransferResources ( android_vulkan::Renderer &renderer ) noexcept
{
    _unitCube.FreeTransferResources ( renderer );
    _lightupCommonDescriptorSet.OnFreeTransferResources ( renderer );

    if ( _commandPool == VK_NULL_HANDLE )
        return;

    vkDestroyCommandPool ( renderer.GetDevice (), _commandPool, nullptr );
    _commandPool = VK_NULL_HANDLE;
}

bool LightPass::OnPreGeometryPass ( android_vulkan::Renderer &renderer,
    VkCommandBuffer commandBuffer,
    size_t commandBufferIndex,
    VkExtent2D const &resolution,
    SceneData const &sceneData,
    size_t opaqueMeshCount,
    GXMat4 const &viewerLocal,
    GXMat4 const &view,
    GXMat4 const &viewProjection,
    GXMat4 const &cvvToView
) noexcept
{
    // FUCK separate upload uniform operations from command buffer recording operations.
    AV_TRACE ( "Light pre-geometry" )
    AV_VULKAN_GROUP ( commandBuffer, "Light pre-geometry" )

    VkDevice device = renderer.GetDevice ();

    return _lightupCommonDescriptorSet.Update ( device,
            commandBufferIndex,
            resolution,
            viewerLocal,
            cvvToView
        ) &&

        _pointLightPass.ExecuteShadowPhase ( renderer, commandBuffer, sceneData, opaqueMeshCount ) &&

        _pointLightPass.UploadGPUData ( device,
            viewerLocal,
            view,
            viewProjection
        ) &&

        _reflectionLocalPass.UploadGPUData ( device, view, viewProjection ) &&
        _volumeDataPool.IssueSync ( device );
}

void LightPass::OnPostGeometryPass ( VkDevice device,
    VkCommandBuffer commandBuffer,
    size_t commandBufferIndex
) noexcept
{
    // FUCK separate upload uniform operations from command buffer recording operations.
    AV_TRACE ( "Light post-geometry" )
    AV_VULKAN_GROUP ( commandBuffer, "Light post-geometry" )

    size_t const pointLightCount = _pointLightPass.GetPointLightCount ();
    size_t const reflectionLocalCount = _reflectionLocalPass.GetReflectionLocalCount ();
    size_t const reflectionGlobalCount = _reflectionGlobalPass.GetReflectionCount ();
    size_t const lightVolumeCount = pointLightCount + reflectionLocalCount;

    _lightupCommonDescriptorSet.Bind ( commandBuffer, commandBufferIndex );

    if ( lightVolumeCount + reflectionGlobalCount == 0U )
    {
        // See https://github.com/Goshido/android-vulkan/issues/84
        _dummyLightProgram.Bind ( commandBuffer );
        vkCmdDraw ( commandBuffer, 3U, 1U, 0U, 0U );
        return;
    }

    if ( pointLightCount )
        _pointLightPass.ExecuteLightupPhase ( commandBuffer, _unitCube );

    if ( reflectionLocalCount )
        _reflectionLocalPass.Execute ( commandBuffer, _unitCube );

    if ( reflectionGlobalCount )
        _reflectionGlobalPass.Execute ( device, commandBuffer );

    _volumeDataPool.Commit ();
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
    constexpr uint16_t const indices[]
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

    constexpr GXVec3 const positions[]
    {
        { -0.5F, -0.5F, -0.5F },
        { 0.5F, -0.5F, -0.5F },
        { -0.5F, 0.5F, -0.5F },
        { 0.5F, 0.5F, -0.5F },
        { 0.5F, -0.5F, 0.5F },
        { -0.5F, -0.5F, 0.5F },
        { 0.5F, 0.5F, 0.5F },
        { -0.5F, 0.5F, 0.5F }
    };

    GXAABB bounds {};
    bounds.AddVertex ( positions[ 0U ] );
    bounds.AddVertex ( positions[ 7U ] );

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
        "pbr::LightPass::CreateUnitCube",
        "Can't allocate command buffer"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( renderer.GetDevice (),
        commandBuffer,
        VK_OBJECT_TYPE_COMMAND_BUFFER,
        "Light pass Unit cube"
    )

    return _unitCube.LoadMesh ( renderer,
        commandBuffer,
        false,
        VK_NULL_HANDLE,
        { indices, std::size ( indices ) },
        { positions, std::size ( positions ) },
        bounds
    );
}

} // namespace pbr
