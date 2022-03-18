#include <pbr/stipple_pass.h>


namespace pbr {

bool StipplePass::Init ( android_vulkan::Renderer &renderer,
    VkCommandPool commandPool,
    VkExtent2D const &resolution,
    VkRenderPass renderPass
) noexcept
{
    VkDevice device = renderer.GetDevice ();

    if ( !_program.Init ( renderer, renderPass, 0U, resolution ) )
        return false;

    if ( !_uniformPool.Init ( renderer, sizeof ( StippleProgram::InstanceData ) ) )
        return false;

    VkCommandBufferAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1U
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateCommandBuffers ( device, &allocateInfo, &_transferCommandBuffer ),
        "StipplePass::Init",
        "Can't allocate transfer command buffer"
    );

    if ( !result )
        return false;

    _submitInfoTransfer.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    _submitInfoTransfer.pNext = nullptr;
    _submitInfoTransfer.waitSemaphoreCount = 0U;
    _submitInfoTransfer.pWaitSemaphores = nullptr;
    _submitInfoTransfer.pWaitDstStageMask = nullptr;
    _submitInfoTransfer.commandBufferCount = 1U;
    _submitInfoTransfer.pCommandBuffers = &_transferCommandBuffer;
    _submitInfoTransfer.signalSemaphoreCount = 0U;
    _submitInfoTransfer.pSignalSemaphores = nullptr;

    return true;
}

void StipplePass::Destroy ( VkDevice device ) noexcept
{
    auto freeVector = [] ( auto &v ) noexcept {
        v.clear ();
        v.shrink_to_fit ();
    };

    freeVector ( _descriptorSetStorage );
    freeVector ( _imageStorage );
    freeVector ( _layouts );
    freeVector ( _poolSizeStorage );
    freeVector ( _uniformStorage );
    freeVector ( _writeStorage0 );
    freeVector ( _writeStorage1 );

    _transferCommandBuffer = VK_NULL_HANDLE;
    _program.Destroy ( device );
}

bool StipplePass::Execute ( android_vulkan::Renderer &renderer,
    VkCommandBuffer commandBuffer,
    GXMat4 const &view,
    GXMat4 const &viewProjection,
    DefaultTextureManager const &defaultTextureManager,
    SamplerManager &samplerManager,
    RenderSessionStats &renderSessionStats
) noexcept
{
    if ( _sceneData.empty () )
        return true;

    bool const result = UpdateGPUData ( renderer,
        view,
        viewProjection,
        _descriptorSetStorage,
        defaultTextureManager,
        samplerManager
    );

    if ( !result )
        return false;

    VkDescriptorSet const* textureSets = _descriptorSetStorage.data ();
    AppendDrawcalls ( commandBuffer, textureSets, textureSets + _sceneData.size (), renderSessionStats );
    return true;
}

void StipplePass::Reset () noexcept
{
    _sceneData.clear ();
}

void StipplePass::Submit ( MeshRef &mesh,
    MaterialRef const &material,
    GXMat4 const &local,
    GXAABB const &worldBounds,
    GXColorRGB const &color0,
    GXColorRGB const &color1,
    GXColorRGB const &color2,
    GXColorRGB const &emission
) noexcept
{
    // NOLINTNEXTLINE - downcast
    auto& m = static_cast<GeometryPassMaterial&> ( *material );
    auto findResult = _sceneData.find ( m );

    if ( findResult != _sceneData.cend () )
    {
        findResult->second.Append ( mesh, local, worldBounds, color0, color1, color2, emission );
        return;
    }

    _sceneData.emplace (
        std::make_pair ( m, GeometryCall ( mesh, local, worldBounds, color0, color1, color2, emission ) )
    );
}

size_t StipplePass::AggregateUniformCount () const noexcept
{
    size_t count = 0U;
    constexpr size_t roundUpFactor = PBR_OPAQUE_MAX_INSTANCE_COUNT - 1U;

    for ( auto const& [material, call] : _sceneData )
    {
        count += call.GetUniqueList().size ();

        for ( auto const& [mesh, group] : call.GetBatchList () )
        {
            size_t const batchSize = group._geometryData.size ();
            count += ( batchSize + roundUpFactor ) / PBR_OPAQUE_MAX_INSTANCE_COUNT;
        }
    }

    return count;
}

void StipplePass::AppendDrawcalls ( VkCommandBuffer commandBuffer,
    VkDescriptorSet const* textureSets,
    VkDescriptorSet const* instanceSets,
    RenderSessionStats &renderSessionStats
) noexcept
{
    size_t textureSetIndex = 0U;
    size_t uniformUsed = 0U;
    bool isProgramBind = false;

    constexpr VkDeviceSize offset = 0U;

    for ( auto const& call : _sceneData )
    {
        GeometryCall const& geometryCall = call.second;

        VkDescriptorSet textureSet = textureSets[ textureSetIndex ];
        ++textureSetIndex;

        if ( !isProgramBind )
        {
            _program.Bind ( commandBuffer );
            isProgramBind = true;
        }

        bool isUniformBind = false;

        auto instanceDrawer = [ & ] ( MeshRef const &mesh, uint32_t batches ) noexcept {
            if ( isUniformBind )
            {
                _program.SetDescriptorSet ( commandBuffer, instanceSets + uniformUsed, 1U, 1U );
            }
            else
            {
                VkDescriptorSet sets[] = { textureSet, instanceSets[ uniformUsed ] };

                _program.SetDescriptorSet ( commandBuffer,
                    sets,
                    0U,
                    static_cast<uint32_t> ( std::size ( sets ) )
                );

                isUniformBind = true;
            }

            vkCmdBindVertexBuffers ( commandBuffer, 0U, 1U, &mesh->GetVertexBuffer (), &offset );
            vkCmdBindIndexBuffer ( commandBuffer, mesh->GetIndexBuffer (), 0U, VK_INDEX_TYPE_UINT32 );

            vkCmdDrawIndexed ( commandBuffer,
                mesh->GetVertexCount (),
                batches,
                0U,
                0U,
                0U
            );

            renderSessionStats.RenderStipple ( mesh->GetVertexCount (), batches );
            ++uniformUsed;
        };

        for ( auto const& [mesh, geometryData] : geometryCall.GetUniqueList () )
        {
            instanceDrawer ( mesh, 1U );
        }

        for ( auto const& item : geometryCall.GetBatchList () )
        {
            MeshGroup const& group = item.second;
            MeshRef const& mesh = group._mesh;
            size_t instanceCount = 0U;

            for ( auto const& geometryData : group._geometryData )
            {
                if ( !geometryData._isVisible )
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

void StipplePass::DestroyDescriptorPool ( VkDevice device ) noexcept
{
    if ( _descriptorPool == VK_NULL_HANDLE )
        return;

    vkDestroyDescriptorPool ( device, _descriptorPool, nullptr );
    _descriptorPool = VK_NULL_HANDLE;
    AV_UNREGISTER_DESCRIPTOR_POOL ( "StipplePass::_descriptorPool" )
}

bool StipplePass::UpdateGPUData ( android_vulkan::Renderer &renderer,
    GXMat4 const &view,
    GXMat4 const &viewProjection,
    std::vector<VkDescriptorSet> &descriptorSetStorage,
    DefaultTextureManager const &defaultTextureManager,
    SamplerManager &samplerManager
) noexcept
{
    size_t const stippleCount = _sceneData.size ();
    size_t const textureCount = stippleCount * GeometryPassTextureDescriptorSetLayout::TEXTURE_SLOTS;

    _imageStorage.clear ();
    _imageStorage.reserve ( textureCount );

    _writeStorage0.clear ();
    _writeStorage0.reserve ( textureCount * 2U );

    Program::DescriptorSetInfo const& descriptorSetInfo = _program.GetResourceInfo ();
    Program::SetItem const& descriptorSet0 = descriptorSetInfo[ 0U ];
    size_t uniqueFeatures = descriptorSet0.size ();

    _uniformStorage.clear ();
    _writeStorage1.clear ();

    constexpr VkCommandBufferBeginInfo beginInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr
    };

    bool result = android_vulkan::Renderer::CheckVkResult ( vkBeginCommandBuffer ( _transferCommandBuffer, &beginInfo ),
        "StipplePass::UpdateGPUData",
        "Can't begin command buffer"
    );

    if ( !result )
        return false;

    // Note reserve size is an estimation from the top.
    size_t const uniformCount = AggregateUniformCount ();
    _uniformStorage.reserve ( uniformCount );
    _writeStorage1.reserve ( uniformCount );
    ++uniqueFeatures;

    _poolSizeStorage.clear ();
    _poolSizeStorage.reserve ( uniqueFeatures );

    for ( auto const& item : descriptorSet0 )
    {
        _poolSizeStorage.emplace_back (
            VkDescriptorPoolSize
            {
                .type = item.type,
                .descriptorCount = static_cast<uint32_t> ( item.descriptorCount * stippleCount + 1U )
            }
        );
    }

    _poolSizeStorage.emplace_back (
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
        .maxSets = static_cast<uint32_t> ( stippleCount + uniformCount ),
        .poolSizeCount = static_cast<uint32_t> ( _poolSizeStorage.size () ),
        .pPoolSizes = _poolSizeStorage.data ()
    };

    result = android_vulkan::Renderer::CheckVkResult (
        vkCreateDescriptorPool ( device, &poolInfo, nullptr, &_descriptorPool ),
        "StipplePass::UpdateGPUData",
        "Can't create descriptor pool"
    );

    if ( !result )
        return false;

    AV_REGISTER_DESCRIPTOR_POOL ( "StipplePass::_descriptorPool" )

    GeometryPassTextureDescriptorSetLayout const materialTextureLayout {};
    VkDescriptorSetLayout materialTextureLayoutNative = materialTextureLayout.GetLayout ();

    _layouts.clear ();
    _layouts.reserve ( static_cast<size_t> ( poolInfo.maxSets ) );

    for ( size_t i = 0U; i < stippleCount; ++i )
        _layouts.push_back ( materialTextureLayoutNative );

    GeometryPassInstanceDescriptorSetLayout const instanceLayout;
    VkDescriptorSetLayout instanceLayoutNative = instanceLayout.GetLayout ();

    for ( auto i = static_cast<uint32_t> ( stippleCount ); i < poolInfo.maxSets; ++i )
        _layouts.push_back ( instanceLayoutNative );

    VkDescriptorSetAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = _descriptorPool,
        .descriptorSetCount = poolInfo.maxSets,
        .pSetLayouts = _layouts.data ()
    };

    descriptorSetStorage.resize ( static_cast<size_t> ( poolInfo.maxSets ) );
    VkDescriptorSet* descriptorSets = descriptorSetStorage.data ();

    result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateDescriptorSets ( device, &allocateInfo, descriptorSets ),
        "StipplePass::UpdateGPUData",
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

    VkSampler materialSampler = samplerManager.GetMaterialSampler ()->GetSampler ();

    auto textureBinder = [ & ] ( Texture2DRef const &texture,
        Texture2DRef const &defaultTexture,
        uint32_t imageBindSlot,
        uint32_t samplerBindSlot
    ) noexcept {
        Texture2DRef const& t = texture ? texture : defaultTexture;

        writeInfo0.pImageInfo = &_imageStorage.emplace_back (
            VkDescriptorImageInfo
            {
                .sampler = materialSampler,
                .imageView = t->GetImageView (),
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            }
        );

        writeInfo0.dstBinding = imageBindSlot;
        writeInfo0.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        _writeStorage0.push_back ( writeInfo0 );

        writeInfo0.dstBinding = samplerBindSlot;
        writeInfo0.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
        _writeStorage0.push_back ( writeInfo0 );
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

    constexpr VkPipelineStageFlags syncFlags = AV_VK_FLAG ( VK_PIPELINE_STAGE_VERTEX_SHADER_BIT ) |
        AV_VK_FLAG ( VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT );

    auto uniformBinder = [ & ] ( VkBuffer uniformBuffer, VkDescriptorSet descriptorSet ) noexcept {
        writeInfo1.pBufferInfo = &_uniformStorage.emplace_back (
            VkDescriptorBufferInfo
            {
                .buffer = uniformBuffer,
                .offset = 0U,
                .range = static_cast<VkDeviceSize> ( sizeof ( StippleProgram::InstanceData ) )
            }
        );

        writeInfo1.dstSet = descriptorSet;
        _writeStorage1.push_back ( writeInfo1 );
    };

    size_t i = 0U;

    for ( auto const& [material, call] : _sceneData )
    {
        writeInfo0.dstSet = descriptorSets[ i ];
        ++i;

        auto& m = const_cast<GeometryPassMaterial&> ( material );

        textureBinder ( m.GetAlbedo (), defaultTextureManager.GetAlbedo (), 0U, 1U );
        textureBinder ( m.GetEmission (), defaultTextureManager.GetEmission (), 2U, 3U );
        textureBinder ( m.GetMask (), defaultTextureManager.GetMask (), 4U, 5U );
        textureBinder ( m.GetNormal (), defaultTextureManager.GetNormal (), 6U, 7U );
        textureBinder ( m.GetParam (), defaultTextureManager.GetParams (), 8U, 9U );
    }

    vkUpdateDescriptorSets ( device,
        static_cast<uint32_t> ( _writeStorage0.size () ),
        _writeStorage0.data (),
        0U,
        nullptr
    );

    GeometryPassProgram::InstanceData instanceData {};
    _uniformPool.Reset ();

    size_t uniformUsed = 0U;
    size_t const maxUniforms = _uniformPool.GetItemCount ();
    VkDescriptorSet const* instanceDescriptorSet = descriptorSets + stippleCount;

    for ( auto const& call : _sceneData )
    {
        GeometryCall const& geometryCall = call.second;

        for ( auto const& [mesh, geometryData] : geometryCall.GetUniqueList () )
        {
            GeometryPassProgram::ObjectData& objectData = instanceData._instanceData[ 0U ];

            if ( uniformUsed >= maxUniforms )
            {
                android_vulkan::LogError (
                    "StipplePass::UpdateGPUData - Uniform pool overflow has been detected (branch 1)!"
                );

                return false;
            }

            GXMat4 const& local = geometryData._local;

            objectData._localView.Multiply ( local, view );
            objectData._localViewProjection.Multiply ( local, viewProjection );

            objectData._color0 = geometryData._color0;
            objectData._color1 = geometryData._color1;
            objectData._color2 = geometryData._color2;
            objectData._emission = geometryData._emission;

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

        for ( auto const& item : geometryCall.GetBatchList () )
        {
            MeshGroup const& group = item.second;
            size_t instanceIndex = 0U;

            for ( auto const& opaqueData : group._geometryData )
            {
                if ( uniformUsed >= maxUniforms )
                {
                    android_vulkan::LogError (
                        "StipplePass::UpdateGPUData - Uniform pool overflow has been detected (branch 0)!"
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

                GeometryPassProgram::ObjectData& objectData = instanceData._instanceData[ instanceIndex ];
                ++instanceIndex;

                objectData._localView.Multiply ( local, view );
                objectData._localViewProjection.Multiply ( local, viewProjection );

                objectData._color0 = opaqueData._color0;
                objectData._color1 = opaqueData._color1;
                objectData._color2 = opaqueData._color2;
                objectData._emission = opaqueData._emission;
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
        static_cast<uint32_t> ( _writeStorage1.size () ),
        _writeStorage1.data (),
        0U,
        nullptr
    );

    result = android_vulkan::Renderer::CheckVkResult ( vkEndCommandBuffer ( _transferCommandBuffer ),
        "StipplePass::UpdateGPUData",
        "Can't end transfer command buffer"
    );

    if ( !result )
        return false;

    return android_vulkan::Renderer::CheckVkResult (
        vkQueueSubmit ( renderer.GetQueue (), 1U, &_submitInfoTransfer, VK_NULL_HANDLE ),
        "StipplePass::UpdateGPUData",
        "Can't submit transfer command buffer"
    );
}

} // namespace pbr
