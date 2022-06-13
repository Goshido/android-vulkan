#include <pbr/opaque_subpass.h>


namespace pbr {

SceneData& OpaqueSubpass::GetSceneData () noexcept
{
    return _sceneData;
}

bool OpaqueSubpass::Init ( android_vulkan::Renderer &renderer,
    VkCommandPool commandPool,
    VkExtent2D const &resolution,
    VkRenderPass renderPass
) noexcept
{
    if ( !InitBase ( renderer, commandPool ) )
        return false;

    return _program.Init ( renderer, renderPass, 0U, resolution );
}

void OpaqueSubpass::Destroy ( VkDevice device ) noexcept
{
    _program.Destroy ( device );
    DestroyBase ( device );
}

bool OpaqueSubpass::Execute ( android_vulkan::Renderer &renderer,
    VkCommandBuffer commandBuffer,
    GXProjectionClipPlanes const &frustum,
    GXMat4 const &view,
    GXMat4 const &viewProjection,
    DefaultTextureManager const &defaultTextureManager,
    RenderSessionStats &renderSessionStats,
    VkDescriptorSet samplerDescriptorSet,
    bool &isSamplerUsed
) noexcept
{
    if ( _sceneData.empty () )
        return true;

    if ( !UpdateGPUData ( renderer, frustum, view, viewProjection, defaultTextureManager ) )
        return false;

    if ( _uniformStorage.empty () )
        return true;

    if ( !isSamplerUsed )
    {
        _program.SetDescriptorSet ( commandBuffer, &samplerDescriptorSet, 0U, 1U );
        isSamplerUsed = true;
    }

    VkDescriptorSet const* textureSets = _descriptorSetStorage.data ();
    AppendDrawcalls ( commandBuffer, _program, textureSets, textureSets + _sceneData.size (), renderSessionStats );
    return true;
}

void OpaqueSubpass::ReportGeometry ( RenderSessionStats &renderSessionStats,
    uint32_t vertexCount,
    uint32_t instanceCount
) noexcept
{
    renderSessionStats.RenderOpaque ( vertexCount, instanceCount );
}

bool OpaqueSubpass::UpdateGPUData ( android_vulkan::Renderer &renderer,
    GXProjectionClipPlanes const &frustum,
    GXMat4 const &view,
    GXMat4 const &viewProjection,
    DefaultTextureManager const &defaultTextureManager
) noexcept
{
    size_t const opaqueCount = _sceneData.size ();
    size_t const textureCount = opaqueCount * GeometryPassTextureDescriptorSetLayout::TEXTURE_SLOTS;

    _imageStorage.clear ();
    _imageStorage.reserve ( textureCount );

    _writeStorage0.clear ();
    _writeStorage0.reserve ( textureCount );

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
        "pbr::OpaqueSubpass::UpdateGPUData",
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
                .descriptorCount = static_cast<uint32_t> ( item.descriptorCount * opaqueCount + 1U )
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

    size_t const maxSets = opaqueCount + uniformCount;
    VkDevice device = renderer.GetDevice ();

    if ( !RecreateDescriptorPool ( device, maxSets ) )
        return false;

    GeometryPassTextureDescriptorSetLayout const materialTextureLayout {};
    VkDescriptorSetLayout materialTextureLayoutNative = materialTextureLayout.GetLayout ();

    _layouts.clear ();
    _layouts.reserve ( maxSets );

    for ( size_t i = 0U; i < opaqueCount; ++i )
        _layouts.push_back ( materialTextureLayoutNative );

    GeometryPassInstanceDescriptorSetLayout const instanceLayout;
    VkDescriptorSetLayout instanceLayoutNative = instanceLayout.GetLayout ();

    for ( size_t i = opaqueCount; i < maxSets; ++i )
        _layouts.push_back ( instanceLayoutNative );

    VkDescriptorSetAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = _descriptorPool,
        .descriptorSetCount = static_cast<uint32_t> ( maxSets ),
        .pSetLayouts = _layouts.data ()
    };

    _descriptorSetStorage.resize ( maxSets );
    VkDescriptorSet* descriptorSets = _descriptorSetStorage.data ();

    result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateDescriptorSets ( device, &allocateInfo, descriptorSets ),
        "pbr::OpaqueSubpass::UpdateGPUData",
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

    auto textureBinder = [ & ] ( Texture2DRef const &texture,
        Texture2DRef const &defaultTexture,
        uint32_t imageBindSlot
    ) noexcept {
        Texture2DRef const& t = texture ? texture : defaultTexture;

        writeInfo0.pImageInfo = &_imageStorage.emplace_back (
            VkDescriptorImageInfo
            {
                .sampler = VK_NULL_HANDLE,
                .imageView = t->GetImageView (),
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            }
        );

        writeInfo0.dstBinding = imageBindSlot;
        writeInfo0.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
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
                .range = static_cast<VkDeviceSize> ( sizeof ( OpaqueProgram::InstanceData ) )
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

        textureBinder ( m.GetAlbedo (), defaultTextureManager.GetAlbedo (), 0U );
        textureBinder ( m.GetEmission (), defaultTextureManager.GetEmission (), 1U );
        textureBinder ( m.GetMask (), defaultTextureManager.GetMask (), 2U );
        textureBinder ( m.GetNormal (), defaultTextureManager.GetNormal (), 3U );
        textureBinder ( m.GetParam (), defaultTextureManager.GetParams (), 4U );
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
    VkDescriptorSet const* instanceDescriptorSet = descriptorSets + opaqueCount;

    for ( auto const& call : _sceneData )
    {
        GeometryCall const& geometryCall = call.second;

        for ( auto const& [mesh, geometryData] : geometryCall.GetUniqueList () )
        {
            GeometryPassProgram::ObjectData& objectData = instanceData._instanceData[ 0U ];

            if ( uniformUsed >= maxUniforms )
            {
                android_vulkan::LogError (
                    "pbr::OpaqueSubpass::UpdateGPUData - Uniform pool overflow has been detected (branch 1)!"
                );

                return false;
            }

            GXMat4 const& local = geometryData._local;
            auto& uniqueGeometryData = const_cast<GeometryData&> ( geometryData );

            if ( !frustum.IsVisible ( uniqueGeometryData._worldBounds ) )
            {
                uniqueGeometryData._isVisible = false;
                continue;
            }

            uniqueGeometryData._isVisible = true;
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
                        "pbr::OpaqueSubpass::UpdateGPUData - Uniform pool overflow has been detected (branch 0)!"
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
                auto& batchGeometryData = const_cast<GeometryData&> ( opaqueData );

                if ( !frustum.IsVisible ( batchGeometryData._worldBounds ) )
                {
                    batchGeometryData._isVisible = false;
                    continue;
                }

                GeometryPassProgram::ObjectData& objectData = instanceData._instanceData[ instanceIndex ];
                ++instanceIndex;
                batchGeometryData._isVisible = true;

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
        "pbr::OpaqueSubpass::UpdateGPUData",
        "Can't end transfer command buffer"
    );

    if ( !result )
        return false;

    return android_vulkan::Renderer::CheckVkResult (
        vkQueueSubmit ( renderer.GetQueue (), 1U, &_submitInfoTransfer, VK_NULL_HANDLE ),
        "pbr::OpaqueSubpass::UpdateGPUData",
        "Can't submit transfer command buffer"
    );
}

} // namespace pbr
