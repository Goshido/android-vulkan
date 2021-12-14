#include <pbr/geometry_pass.h>
#include <half_types.h>


namespace pbr {

constexpr static size_t const TOTAL_COMMAND_BUFFERS = GeometryPass::DEFAULT_TEXTURE_COUNT + 2U;

GeometryPass::GeometryPass () noexcept:
    _albedoDefault {},
    _emissionDefault {},
    _maskDefault {},
    _normalDefault {},
    _paramDefault {},
    _isFreeTransferResources ( false ),
    _commandPool ( VK_NULL_HANDLE ),
    _descriptorPool ( VK_NULL_HANDLE ),
    _fence ( VK_NULL_HANDLE ),
    _program {},
    _renderCommandBuffer ( VK_NULL_HANDLE ),
    _renderPassInfo {},
    _sceneData {},
    _submitInfoTransfer {},
    _textureCommandBuffers {},
    _transferCommandBuffer ( VK_NULL_HANDLE ),
    _uniformPool ( eUniformPoolSize::Huge_64M )
{
    // NOTHING
}

SceneData& GeometryPass::GetSceneData ()
{
    return _sceneData;
}

bool GeometryPass::Init ( android_vulkan::Renderer &renderer,
    VkCommandPool commandPool,
    VkExtent2D const &resolution,
    VkRenderPass renderPass,
    VkFramebuffer framebuffer
)
{
    VkDevice device = renderer.GetDevice ();

    constexpr VkFenceCreateInfo const fenceInfo
    {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateFence ( device, &fenceInfo, nullptr, &_fence ),
        "GeometryPass::Init",
        "Can't create fence"
    );

    if ( !result )
    {
        Destroy ( device );
        return false;
    }

    AV_REGISTER_FENCE ( "GeometryPass::_fence" )

    if ( !_program.Init ( renderer, renderPass, 0U, resolution ) )
    {
        Destroy ( device );
        return false;
    }

    if ( !_uniformPool.Init ( renderer, sizeof ( OpaqueProgram::InstanceData ) ) )
    {
        Destroy ( device );
        return false;
    }

    _commandPool = commandPool;
    VkCommandBuffer commandBuffers[ TOTAL_COMMAND_BUFFERS ];

    VkCommandBufferAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = _commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = static_cast<uint32_t> ( std::size ( commandBuffers ) )
    };

    result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateCommandBuffers ( device, &allocateInfo, commandBuffers ),
        "RenderSession::Init",
        "Can't allocate command buffers"
    );

    if ( !result )
    {
        Destroy ( device );
        return false;
    }

    for ( size_t i = 0U; i < DEFAULT_TEXTURE_COUNT; ++i )
        _textureCommandBuffers[ i ] = commandBuffers[ i ];

    _renderCommandBuffer = commandBuffers[ DEFAULT_TEXTURE_COUNT ];
    _transferCommandBuffer = commandBuffers[ DEFAULT_TEXTURE_COUNT + 1U ];

    if ( !InitDefaultTextures ( renderer ) )
    {
        Destroy ( device );
        return false;
    }

    InitCommonStructures ( renderPass, framebuffer, resolution );
    return true;
}

void GeometryPass::Destroy ( VkDevice device )
{
    DestroyDescriptorPool ( device );
    DestroyDefaultTextures ( device );

    if ( _commandPool != VK_NULL_HANDLE )
    {
        if ( !_isFreeTransferResources )
        {
            vkFreeCommandBuffers ( device,
                _commandPool,
                static_cast<uint32_t> ( DEFAULT_TEXTURE_COUNT ),
                _textureCommandBuffers
            );

            for ( VkCommandBuffer& commandBuffer : _textureCommandBuffers )
            {
                commandBuffer = VK_NULL_HANDLE;
            }
        }

        VkCommandBuffer const commandBuffers[] =
        {
            _transferCommandBuffer,
            _renderCommandBuffer
        };

        vkFreeCommandBuffers ( device,
            _commandPool,
            static_cast<uint32_t> ( std::size ( commandBuffers ) ),
            commandBuffers
        );

        _transferCommandBuffer = VK_NULL_HANDLE;
        _renderCommandBuffer = VK_NULL_HANDLE;
        _commandPool = VK_NULL_HANDLE;
    }

    _uniformPool.Destroy ( device );
    _program.Destroy ( device );

    if ( _fence == VK_NULL_HANDLE )
        return;

    vkDestroyFence ( device, _fence, nullptr );
    _fence = VK_NULL_HANDLE;
    AV_UNREGISTER_FENCE ( "GeometryPass::_fence" )
}

VkCommandBuffer GeometryPass::Execute ( android_vulkan::Renderer &renderer,
    GXProjectionClipPlanes const &frustum,
    GXMat4 const &view,
    GXMat4 const &viewProjection,
    SamplerManager &samplerManager,
    RenderSessionStats &renderSessionStats
)
{
    CleanupTransferResources ( renderer );

    if ( !BeginRenderPass ( renderer ) )
        return VK_NULL_HANDLE;

    std::vector<VkDescriptorSet> descriptorSetStorage;

    if ( !UpdateGPUData ( renderer, frustum, view, viewProjection, descriptorSetStorage, samplerManager ) )
        return VK_NULL_HANDLE;

    VkDescriptorSet const* textureSets = descriptorSetStorage.data ();
    AppendDrawcalls ( textureSets, textureSets + _sceneData.size (), renderSessionStats );
    vkCmdEndRenderPass ( _renderCommandBuffer );

    return _renderCommandBuffer;
}

VkFence GeometryPass::GetFence () const
{
    return _fence;
}

void GeometryPass::Reset ()
{
    _sceneData.clear ();
}

void GeometryPass::Submit ( MeshRef &mesh,
    MaterialRef const &material,
    GXMat4 const &local,
    GXAABB const &worldBounds,
    android_vulkan::Color32 const &color0,
    android_vulkan::Color32 const &color1,
    android_vulkan::Color32 const &color2,
    android_vulkan::Color32 const &color3
)
{
    // Note it's safe to cast like that here. "NOLINT" is a clang-tidy control comment.
    auto& opaqueMaterial = *static_cast<OpaqueMaterial*> ( material.get () ); // NOLINT
    auto findResult = _sceneData.find ( opaqueMaterial );

    if ( findResult != _sceneData.cend () )
    {
        findResult->second.Append ( mesh, local, worldBounds, color0, color1, color2, color3 );
        return;
    }

    _sceneData.insert (
        std::make_pair ( opaqueMaterial, OpaqueCall ( mesh, local, worldBounds, color0, color1, color2, color3 ) )
    );
}

VkSubpassDescription GeometryPass::GetSubpassDescription ()
{
    constexpr static VkAttachmentReference const colorReferences[] =
    {
        // #0: albedo
        {
            .attachment = 0U,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        },

        // #1: emission
        {
            .attachment = 1U,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        },

        // #2: normal
        {
            .attachment = 2U,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        },

        // #3: params
        {
            .attachment = 3U,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        }
    };

    // depth|stencil
    constexpr static VkAttachmentReference const depthStencilReference
    {
        .attachment = 4U,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

    return
    {
        .flags = 0U,
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .inputAttachmentCount = 0U,
        .pInputAttachments = nullptr,
        .colorAttachmentCount = static_cast<uint32_t> ( std::size ( colorReferences ) ),
        .pColorAttachments = colorReferences,
        .pResolveAttachments = nullptr,
        .pDepthStencilAttachment = &depthStencilReference,
        .preserveAttachmentCount = 0U,
        .pPreserveAttachments = nullptr
    };
}

size_t GeometryPass::AggregateUniformCount () const noexcept
{
    size_t count = 0U;
    constexpr size_t const roundUpFactor = PBR_OPAQUE_MAX_INSTANCE_COUNT - 1U;

    for ( auto const& [material, call] : _sceneData )
    {
        count += call.GetUniqueList().size ();

        for ( auto const& [mesh, group] : call.GetBatchList () )
        {
            size_t const batchSize = group._opaqueData.size ();
            count += ( batchSize + roundUpFactor ) / PBR_OPAQUE_MAX_INSTANCE_COUNT;
        }
    }

    return count;
}

void GeometryPass::AppendDrawcalls ( VkDescriptorSet const* textureSets,
    VkDescriptorSet const* instanceSets,
    RenderSessionStats &renderSessionStats
)
{
    size_t textureSetIndex = 0U;
    size_t uniformUsed = 0U;
    bool isProgramBind = false;

    constexpr VkDeviceSize const offset = 0U;

    for ( auto const& call : _sceneData )
    {
        OpaqueCall const &opaqueCall = call.second;

        VkDescriptorSet textureSet = textureSets[ textureSetIndex ];
        ++textureSetIndex;

        if ( !isProgramBind )
        {
            _program.Bind ( _renderCommandBuffer );
            isProgramBind = true;
        }

        bool isUniformBind = false;

        auto instanceDrawer = [ & ] ( MeshRef const &mesh, uint32_t batches ) {
            if ( isUniformBind )
            {
                _program.SetDescriptorSet ( _renderCommandBuffer, instanceSets + uniformUsed, 1U, 1U );
            }
            else
            {
                VkDescriptorSet sets[] = { textureSet, instanceSets[ uniformUsed ] };

                _program.SetDescriptorSet ( _renderCommandBuffer,
                    sets,
                    0U,
                    static_cast<uint32_t> ( std::size ( sets ) )
                );

                isUniformBind = true;
            }

            vkCmdBindVertexBuffers ( _renderCommandBuffer, 0U, 1U, &mesh->GetVertexBuffer (), &offset );
            vkCmdBindIndexBuffer ( _renderCommandBuffer, mesh->GetIndexBuffer (), 0U, VK_INDEX_TYPE_UINT32 );

            vkCmdDrawIndexed ( _renderCommandBuffer,
                mesh->GetVertexCount (),
                batches,
                0U,
                0U,
                0U
            );

            renderSessionStats.RenderOpaque ( mesh->GetVertexCount (), batches );
            ++uniformUsed;
        };

        for ( auto const& [mesh, opaqueData] : opaqueCall.GetUniqueList () )
        {
            if ( !opaqueData._isVisible )
                continue;

            instanceDrawer ( mesh, 1U );
        }

        for ( auto const& item : opaqueCall.GetBatchList () )
        {
            MeshGroup const& group = item.second;
            MeshRef const& mesh = group._mesh;
            size_t instanceCount = 0U;

            for ( auto const& opaqueData : group._opaqueData )
            {
                if ( !opaqueData._isVisible )
                    continue;

                ++instanceCount;
            }

            size_t instanceIndex = 0U;
            size_t batches = 0U;

            while ( instanceIndex < instanceCount )
            {
                batches = std::min ( instanceCount - instanceIndex,
                    static_cast<size_t> ( PBR_OPAQUE_MAX_INSTANCE_COUNT )
                );

                instanceIndex += batches;

                if ( batches < PBR_OPAQUE_MAX_INSTANCE_COUNT )
                    continue;

                instanceDrawer ( mesh, static_cast<uint32_t> ( batches ) );
                batches = 0U;
            }

            if ( !batches )
                continue;

            instanceDrawer ( mesh, static_cast<uint32_t> ( batches ) );
        }
    }
}

bool GeometryPass::BeginRenderPass ( android_vulkan::Renderer &renderer )
{
    VkDevice device = renderer.GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkWaitForFences ( device, 1U, &_fence, VK_TRUE, UINT64_MAX ),
        "GeometryPass::BeginRenderPass",
        "Can't wait for fence"
    );

    if ( !result )
        return false;

    result = android_vulkan::Renderer::CheckVkResult ( vkResetFences ( device, 1U, &_fence ),
        "GeometryPass::BeginRenderPass",
        "Can't reset fence"
    );

    if ( !result )
        return false;

    constexpr VkCommandBufferBeginInfo const beginInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr
    };

    result = android_vulkan::Renderer::CheckVkResult ( vkBeginCommandBuffer ( _renderCommandBuffer, &beginInfo ),
        "GeometryPass::BeginRenderPass",
        "Can't begin rendering command buffer"
    );

    if ( !result )
        return false;

    vkCmdBeginRenderPass ( _renderCommandBuffer, &_renderPassInfo, VK_SUBPASS_CONTENTS_INLINE );
    return true;
}

void GeometryPass::CleanupTransferResources ( android_vulkan::Renderer &renderer )
{
    if ( !_isFreeTransferResources )
        return;

    VkDevice device = renderer.GetDevice ();

    if ( _albedoDefault )
        _albedoDefault->FreeTransferResources ( device );

    if ( _emissionDefault )
        _emissionDefault->FreeTransferResources ( device );

    if ( _maskDefault )
        _maskDefault->FreeTransferResources ( device );

    if ( _normalDefault )
        _normalDefault->FreeTransferResources ( device );

    if ( _paramDefault )
        _paramDefault->FreeTransferResources ( device );

    _isFreeTransferResources = false;

    vkFreeCommandBuffers ( device,
        _commandPool,
        static_cast<uint32_t> ( DEFAULT_TEXTURE_COUNT ),
        _textureCommandBuffers
    );

    for ( auto& item : _textureCommandBuffers )
    {
        item = VK_NULL_HANDLE;
    }
}

void GeometryPass::InitCommonStructures ( VkRenderPass renderPass,
    VkFramebuffer framebuffer,
    VkExtent2D const &resolution
)
{
    constexpr static VkClearValue const clearValues[] =
    {
        // albedo
        {
            .color
            {
                .float32 = { 0.0F, 0.0F, 0.0F, 0.0F }
            }
        },

        // emission
        {
            .color
            {
                .float32 = { 0.0F, 0.0F, 0.0F, 0.0F }
            }
        },

        // normal
        {
            .color
            {
                .float32 = { 0.5F, 0.5F, 0.5F, 0.0F }
            }
        },

        // param
        {
            .color
            {
                .float32 = { 0.5F, 0.5F, 0.5F, 0.0F }
            }
        },

        // depth|stencil
        {
            .depthStencil
            {
                .depth = 1.0F,
                .stencil = 0U
            }
        }
    };

    _renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    _renderPassInfo.pNext = nullptr;
    _renderPassInfo.renderPass = renderPass;
    _renderPassInfo.framebuffer = framebuffer;
    _renderPassInfo.clearValueCount = static_cast<uint32_t> ( std::size ( clearValues ) );
    _renderPassInfo.pClearValues = clearValues;
    _renderPassInfo.renderArea.offset.x = 0;
    _renderPassInfo.renderArea.offset.y = 0;
    _renderPassInfo.renderArea.extent = resolution;

    _submitInfoTransfer.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    _submitInfoTransfer.pNext = nullptr;
    _submitInfoTransfer.waitSemaphoreCount = 0U;
    _submitInfoTransfer.pWaitSemaphores = nullptr;
    _submitInfoTransfer.pWaitDstStageMask = nullptr;
    _submitInfoTransfer.commandBufferCount = 1U;
    _submitInfoTransfer.pCommandBuffers = &_transferCommandBuffer;
    _submitInfoTransfer.signalSemaphoreCount = 0U;
    _submitInfoTransfer.pSignalSemaphores = nullptr;
}

bool GeometryPass::InitDefaultTextures ( android_vulkan::Renderer &renderer )
{
    auto textureLoader = [ &renderer ] ( Texture2DRef &texture,
        const uint8_t* data,
        size_t size,
        VkFormat format,
        VkCommandBuffer commandBuffer
    ) -> bool {
        texture = std::make_shared<android_vulkan::Texture2D> ();
        constexpr VkExtent2D const resolution { .width = 1U, .height = 1U };

        bool const result = texture->UploadData ( renderer,
            data,
            size,
            resolution,
            format,
            false,
            commandBuffer
        );

        if ( result )
            return true;

        texture = nullptr;
        return false;
    };

    constexpr uint8_t const albedo[ 4U ] = { 255U, 255U, 255U, 255U };

    bool result = textureLoader ( _albedoDefault,
        albedo,
        sizeof ( albedo ),
        VK_FORMAT_R8G8B8A8_SRGB,
        _textureCommandBuffers[ 0U ]
    );

    if ( !result )
        return false;

    _isFreeTransferResources = true;

    android_vulkan::Half4 const emission ( 0.0F, 0.0F, 0.0F, 0.0F );

    result = textureLoader ( _emissionDefault,
        reinterpret_cast<const uint8_t*> ( emission._data ),
        sizeof ( emission ),
        VK_FORMAT_R16G16B16A16_SFLOAT,
        _textureCommandBuffers[ 1U ]
    );

    if ( !result )
        return false;

    constexpr uint8_t const mask[ 4U ] = { 255U, 0U, 0U, 0U };

    result = textureLoader ( _maskDefault,
        mask,
        sizeof ( mask ),
        VK_FORMAT_R8G8B8A8_UNORM,
        _textureCommandBuffers[ 2U ]
    );

    if ( !result )
        return false;

    constexpr const uint8_t normal[ 4U ] = { 128U, 0U, 0U, 128U };

    result = textureLoader ( _normalDefault,
        normal,
        sizeof ( normal ),
        VK_FORMAT_R8G8B8A8_UNORM,
        _textureCommandBuffers[ 3U ]
    );

    if ( !result )
        return false;

    constexpr const uint8_t param[ 4U ] = { 128U, 128U, 128U, 128U };

    return textureLoader ( _paramDefault,
        param,
        sizeof ( param ),
        VK_FORMAT_R8G8B8A8_UNORM,
        _textureCommandBuffers[ 4U ]
    );
}

void GeometryPass::DestroyDefaultTextures ( VkDevice device )
{
    auto freeTexture = [ device ] ( Texture2DRef &texture ) {
        if ( !texture )
            return;

        texture->FreeResources ( device );
        texture = nullptr;
    };

    freeTexture ( _paramDefault );
    freeTexture ( _normalDefault );
    freeTexture ( _maskDefault );
    freeTexture ( _emissionDefault );
    freeTexture ( _albedoDefault );
}

void GeometryPass::DestroyDescriptorPool ( VkDevice device )
{
    if ( _descriptorPool == VK_NULL_HANDLE )
        return;

    vkDestroyDescriptorPool ( device, _descriptorPool, nullptr );
    _descriptorPool = VK_NULL_HANDLE;
    AV_UNREGISTER_DESCRIPTOR_POOL ( "GeometryPass::_descriptorPool" )
}

bool GeometryPass::UpdateGPUData ( android_vulkan::Renderer &renderer,
    GXProjectionClipPlanes const &frustum,
    GXMat4 const &view,
    GXMat4 const &viewProjection,
    std::vector<VkDescriptorSet> &descriptorSetStorage,
    SamplerManager &samplerManager
)
{
    // TODO no any elements have been submitted.

    size_t const opaqueCount = _sceneData.size ();
    size_t const textureCount = opaqueCount * OpaqueTextureDescriptorSetLayout::TEXTURE_SLOTS;

    std::vector<VkDescriptorImageInfo> imageStorage {};
    imageStorage.reserve ( textureCount );

    std::vector<VkWriteDescriptorSet> writeStorage0 {};
    writeStorage0.reserve ( textureCount * 2U );

    Program::DescriptorSetInfo const& descriptorSetInfo = _program.GetResourceInfo ();
    Program::SetItem const& descriptorSet0 = descriptorSetInfo[ 0U ];
    size_t uniqueFeatures = descriptorSet0.size ();

    std::vector<VkDescriptorBufferInfo> uniformStorage {};
    std::vector<VkWriteDescriptorSet> writeStorage1 {};

    constexpr VkCommandBufferBeginInfo const beginInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr
    };

    bool result = android_vulkan::Renderer::CheckVkResult ( vkBeginCommandBuffer ( _transferCommandBuffer, &beginInfo ),
        "GeometryPass::UpdateGPUData",
        "Can't begin command buffer"
    );

    if ( !result )
        return false;

    // Note reserve size is an estimation from the top.
    size_t const uniformCount = AggregateUniformCount ();
    uniformStorage.reserve ( uniformCount );
    writeStorage1.reserve ( uniformCount );
    ++uniqueFeatures;

    std::vector<VkDescriptorPoolSize> poolSizeStorage {};
    poolSizeStorage.reserve ( uniqueFeatures );

    for ( auto const& item : descriptorSet0 )
    {
        poolSizeStorage.emplace_back (
            VkDescriptorPoolSize
            {
                .type = item.type,
                .descriptorCount = static_cast<uint32_t> ( item.descriptorCount * opaqueCount + 1U )
            }
        );
    }

    poolSizeStorage.emplace_back (
        VkDescriptorPoolSize
        {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = static_cast<uint32_t> ( uniformCount )
        }
    );

    VkDevice device = renderer.GetDevice ();
    DestroyDescriptorPool ( device );

    VkDescriptorPoolCreateInfo const poolInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .maxSets = static_cast<uint32_t> ( opaqueCount + uniformCount ),
        .poolSizeCount = static_cast<uint32_t> ( poolSizeStorage.size () ),
        .pPoolSizes = poolSizeStorage.data ()
    };

    result = android_vulkan::Renderer::CheckVkResult (
        vkCreateDescriptorPool ( device, &poolInfo, nullptr, &_descriptorPool ),
        "GeometryPass::UpdateGPUData",
        "Can't create descriptor pool"
    );

    if ( !result )
        return false;

    AV_REGISTER_DESCRIPTOR_POOL ( "GeometryPass::_descriptorPool" )

    OpaqueTextureDescriptorSetLayout const opaqueTextureLayout {};
    VkDescriptorSetLayout opaqueTextureLayoutNative = opaqueTextureLayout.GetLayout ();

    std::vector<VkDescriptorSetLayout> layouts;
    layouts.reserve ( static_cast<size_t> ( poolInfo.maxSets ) );

    for ( size_t i = 0U; i < opaqueCount; ++i )
        layouts.push_back ( opaqueTextureLayoutNative );

    OpaqueInstanceDescriptorSetLayout const instanceLayout;
    VkDescriptorSetLayout instanceLayoutNative = instanceLayout.GetLayout ();

    for ( auto i = static_cast<uint32_t> ( opaqueCount ); i < poolInfo.maxSets; ++i )
        layouts.push_back ( instanceLayoutNative );

    VkDescriptorSetAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = _descriptorPool,
        .descriptorSetCount = poolInfo.maxSets,
        .pSetLayouts = layouts.data ()
    };

    descriptorSetStorage.resize ( static_cast<size_t> ( poolInfo.maxSets ) );
    VkDescriptorSet* descriptorSets = descriptorSetStorage.data ();

    result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateDescriptorSets ( device, &allocateInfo, descriptorSets ),
        "GeometryPass::UpdateGPUData",
        "Can't allocate descriptor sets"
    );

    if ( !result )
        return false;

    VkWriteDescriptorSet writeInfo0 {};
    writeInfo0.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeInfo0.pNext = nullptr;
    writeInfo0.descriptorCount = 1U;
    writeInfo0.dstArrayElement = 0U;
    writeInfo0.pBufferInfo = nullptr;
    writeInfo0.pTexelBufferView = nullptr;

    auto textureBinder = [ & ] ( Texture2DRef &texture,
        Texture2DRef &defaultTexture,
        uint32_t imageBindSlot,
        uint32_t samplerBindSlot
    ) noexcept {
        Texture2DRef& t = texture ? texture : defaultTexture;
        SamplerRef sampler = samplerManager.GetSampler ( renderer, t->GetMipLevelCount () );

        writeInfo0.pImageInfo = &imageStorage.emplace_back (
            VkDescriptorImageInfo
            {
                .sampler = sampler->GetSampler (),
                .imageView = t->GetImageView (),
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            }
        );

        writeInfo0.dstBinding = imageBindSlot;
        writeInfo0.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        writeStorage0.push_back ( writeInfo0 );

        writeInfo0.dstBinding = samplerBindSlot;
        writeInfo0.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
        writeStorage0.push_back ( writeInfo0 );
    };

    VkWriteDescriptorSet writeInfo1 {};
    writeInfo1.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeInfo1.pNext = nullptr;
    writeInfo1.dstBinding = 0U;
    writeInfo1.dstArrayElement = 0U;
    writeInfo1.descriptorCount = 1U;
    writeInfo1.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writeInfo1.pImageInfo = nullptr;
    writeInfo1.pTexelBufferView = nullptr;

    constexpr VkPipelineStageFlags const syncFlags = AV_VK_FLAG ( VK_PIPELINE_STAGE_VERTEX_SHADER_BIT ) |
        AV_VK_FLAG ( VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT );

    auto uniformBinder = [ & ] ( VkBuffer uniformBuffer, VkDescriptorSet descriptorSet ) noexcept {
        writeInfo1.pBufferInfo = &uniformStorage.emplace_back (
            VkDescriptorBufferInfo
            {
                .buffer = uniformBuffer,
                .offset = 0U,
                .range = static_cast<VkDeviceSize> ( sizeof ( OpaqueProgram::InstanceData ) )
            }
        );

        writeInfo1.dstSet = descriptorSet;
        writeStorage1.push_back ( writeInfo1 );
    };

    size_t i = 0U;

    for ( auto const& [material, call] : _sceneData )
    {
        writeInfo0.dstSet = descriptorSets[ i ];
        ++i;

        // Warning less casting.
        auto& m = const_cast<OpaqueMaterial&> (
            *static_cast<OpaqueMaterial const*> ( reinterpret_cast<void const*> ( &material ) )
        );

        textureBinder ( m.GetAlbedo (), _albedoDefault, 0U, 1U );
        textureBinder ( m.GetEmission (), _emissionDefault, 2U, 3U );
        textureBinder ( m.GetMask (), _maskDefault, 4U, 5U );
        textureBinder ( m.GetNormal (), _normalDefault, 6U, 7U );
        textureBinder ( m.GetParam (), _paramDefault, 8U, 9U );
    }

    vkUpdateDescriptorSets ( device,
        static_cast<uint32_t> ( writeStorage0.size () ),
        writeStorage0.data (),
        0U,
        nullptr
    );

    OpaqueProgram::InstanceData instanceData {};
    _uniformPool.Reset ();

    size_t uniformUsed = 0U;
    size_t const maxUniforms = _uniformPool.GetItemCount ();
    VkDescriptorSet const* instanceDescriptorSet = descriptorSets + opaqueCount;

    for ( auto const& call : _sceneData )
    {
        OpaqueCall const& opaqueCall = call.second;

        for ( auto const& [mesh, opaqueData] : opaqueCall.GetUniqueList () )
        {
            OpaqueProgram::ObjectData& objectData = instanceData._instanceData[ 0U ];

            if ( uniformUsed >= maxUniforms )
            {
                android_vulkan::LogError (
                    "GeometryPass::UpdateGPUData - Uniform pool overflow has been detected (branch 1)!"
                );

                return false;
            }

            GXMat4 const& local = opaqueData._local;
            auto& uniqueOpaqueData = const_cast<OpaqueData&> ( opaqueData );

            if ( !frustum.IsVisible ( uniqueOpaqueData._worldBounds ) )
            {
                uniqueOpaqueData._isVisible = false;
                continue;
            }

            uniqueOpaqueData._isVisible = true;
            objectData._localView.Multiply ( local, view );
            objectData._localViewProjection.Multiply ( local, viewProjection );

            objectData._color0 = opaqueData._color0;
            objectData._color1 = opaqueData._color1;
            objectData._color2 = opaqueData._color2;
            objectData._color3 = opaqueData._color3;

            uniformBinder (
                _uniformPool.Acquire ( renderer,
                    _transferCommandBuffer,
                    instanceData._instanceData,
                    syncFlags
                ),

                instanceDescriptorSet[ uniformUsed ]
            );

            ++uniformUsed;
        }

        for ( auto const& item : opaqueCall.GetBatchList () )
        {
            MeshGroup const& group = item.second;
            size_t instanceIndex = 0U;

            for ( auto const& opaqueData : group._opaqueData )
            {
                if ( uniformUsed >= maxUniforms )
                {
                    android_vulkan::LogError (
                        "GeometryPass::UpdateGPUData - Uniform pool overflow has been detected (branch 0)!"
                    );

                    return false;
                }

                if ( instanceIndex >= PBR_OPAQUE_MAX_INSTANCE_COUNT )
                {
                    uniformBinder (
                        _uniformPool.Acquire ( renderer,
                            _transferCommandBuffer,
                            instanceData._instanceData,
                            syncFlags
                        ),

                        instanceDescriptorSet[ uniformUsed ]
                    );

                    instanceIndex = 0U;
                    ++uniformUsed;
                }

                GXMat4 const& local = opaqueData._local;
                auto& batchOpaqueData = const_cast<OpaqueData&> ( opaqueData );

                if ( !frustum.IsVisible ( batchOpaqueData._worldBounds ) )
                {
                    batchOpaqueData._isVisible = false;
                    continue;
                }

                OpaqueProgram::ObjectData& objectData = instanceData._instanceData[ instanceIndex ];
                ++instanceIndex;
                batchOpaqueData._isVisible = true;

                objectData._localView.Multiply ( local, view );
                objectData._localViewProjection.Multiply ( local, viewProjection );

                objectData._color0 = opaqueData._color0;
                objectData._color1 = opaqueData._color1;
                objectData._color2 = opaqueData._color2;
                objectData._color3 = opaqueData._color3;
            }

            if ( !instanceIndex )
                continue;

            uniformBinder (
                _uniformPool.Acquire ( renderer,
                    _transferCommandBuffer,
                    instanceData._instanceData,
                    syncFlags
                ),

                instanceDescriptorSet[ uniformUsed ]
            );

            ++uniformUsed;
        }
    }

    vkUpdateDescriptorSets ( device,
        static_cast<uint32_t> ( writeStorage1.size () ),
        writeStorage1.data (),
        0U,
        nullptr
    );

    result = android_vulkan::Renderer::CheckVkResult ( vkEndCommandBuffer ( _transferCommandBuffer ),
        "GeometryPass::UpdateGPUData",
        "Can't end transfer command buffer"
    );

    if ( !result )
        return false;

    return android_vulkan::Renderer::CheckVkResult (
        vkQueueSubmit ( renderer.GetQueue (), 1U, &_submitInfoTransfer, VK_NULL_HANDLE ),
        "GeometryPass::UpdateGPUData",
        "Can't submit transfer command buffer"
    );
}

} // namespace pbr
