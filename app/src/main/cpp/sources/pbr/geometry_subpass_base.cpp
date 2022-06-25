#include <pbr/geometry_subpass_base.h>


namespace pbr {

void GeometrySubpassBase::Reset () noexcept
{
    _sceneData.clear ();
}

void GeometrySubpassBase::Submit ( MeshRef &mesh,
    MaterialRef const &material,
    GXMat4 const &local,
    GXAABB const &worldBounds,
    GXColorRGB const &color0,
    GXColorRGB const &color1,
    GXColorRGB const &color2,
    GXColorRGB const &emission
) noexcept
{
    // NOLINTNEXTLINE - downcast.
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

size_t GeometrySubpassBase::AggregateUniformCount () const noexcept
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

bool GeometrySubpassBase::AllocateMaterialSystem ( VkDevice device, size_t materials, size_t textureCount ) noexcept
{
    DestroyMaterialDescriptorPool ( device );

    VkDescriptorPoolSize const poolSize
    {
        .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        .descriptorCount = static_cast<uint32_t> ( textureCount )
    };

    VkDescriptorPoolCreateInfo const poolInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .maxSets = static_cast<uint32_t> ( materials ),
        .poolSizeCount = 1U,
        .pPoolSizes = &poolSize
    };

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateDescriptorPool ( device, &poolInfo, nullptr, &_materialTransfer._pool ),
        "pbr::GeometrySubpassBase::AllocateMaterialSystem",
        "Can't create descriptor pool"
    );

    if ( !result )
        return false;

    AV_REGISTER_DESCRIPTOR_POOL ( "pbr::GeometrySubpassBase::_materialTransfer._pool" )

    std::vector<VkDescriptorSetLayout> const layouts ( materials,
        GeometryPassTextureDescriptorSetLayout ().GetLayout ()
    );

    VkDescriptorSetAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = _materialTransfer._pool,
        .descriptorSetCount = poolInfo.maxSets,
        .pSetLayouts = layouts.data ()
    };

    std::vector<VkDescriptorSet>& sets = _materialTransfer._sets;
    sets.resize ( materials );

    result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateDescriptorSets ( device, &allocateInfo, sets.data () ),
        "pbr::GeometrySubpassBase::AllocateMaterialSystem",
        "Can't allocate descriptor sets"
    );

    if ( !result )
        return false;

    constexpr VkDescriptorImageInfo imageTemplate
    {
        .sampler = VK_NULL_HANDLE,
        .imageView = VK_NULL_HANDLE,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };

    _imageStorage.resize ( textureCount, imageTemplate );

    constexpr VkWriteDescriptorSet writeTemplate
    {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = nullptr,
        .dstSet = VK_NULL_HANDLE,
        .dstBinding = 0U,
        .dstArrayElement = 0U,
        .descriptorCount = 1U,
        .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        .pImageInfo = nullptr,
        .pBufferInfo = nullptr,
        .pTexelBufferView = nullptr
    };

    std::vector<VkWriteDescriptorSet>& writes = _materialTransfer._writes;
    writes.resize ( textureCount, writeTemplate );

    for ( size_t i = 0U; i < textureCount; ++i )
    {
        VkWriteDescriptorSet& set = writes[ i ];
        set.dstSet = sets[ i / GeometryPassTextureDescriptorSetLayout::TEXTURE_SLOTS ];
        set.dstBinding = i % GeometryPassTextureDescriptorSetLayout::TEXTURE_SLOTS;
        set.pImageInfo = &_imageStorage[ i ];
    }

    return true;
}

bool GeometrySubpassBase::AllocateTransferSystem ( VkDevice device ) noexcept
{
    size_t const materials = _sceneData.size ();
    size_t const textures = materials * GeometryPassTextureDescriptorSetLayout::TEXTURE_SLOTS;

    if ( _imageStorage.size () < textures )
    {
        if ( !AllocateMaterialSystem ( device, materials, textures ) )
        {
            return false;
        }
    }

    // Note reserve size is an estimation from the top.
    size_t const uniformBuffers = AggregateUniformCount ();

    if ( _uniformStorage.size () >= uniformBuffers )
        return true;

    return AllocateUniformBufferSystem ( device, uniformBuffers );
}

bool GeometrySubpassBase::AllocateUniformBufferSystem ( VkDevice device, size_t uniformCount ) noexcept
{
    DestroyUniformDescriptorPool ( device );

    VkDescriptorPoolSize const poolSize
    {
        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = static_cast<uint32_t> ( uniformCount )
    };

    VkDescriptorPoolCreateInfo const poolInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .maxSets = static_cast<uint32_t> ( uniformCount ),
        .poolSizeCount = 1U,
        .pPoolSizes = &poolSize
    };

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateDescriptorPool ( device, &poolInfo, nullptr, &_uniformTransfer._pool ),
        "pbr::GeometrySubpassBase::AllocateUniformBufferSystem",
        "Can't create descriptor pool"
    );

    if ( !result )
        return false;

    AV_REGISTER_DESCRIPTOR_POOL ( "pbr::GeometrySubpassBase::_uniformTransfer._pool" )

    std::vector<VkDescriptorSetLayout> const layouts ( uniformCount,
        GeometryPassInstanceDescriptorSetLayout ().GetLayout ()
    );

    VkDescriptorSetAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = _uniformTransfer._pool,
        .descriptorSetCount = poolInfo.maxSets,
        .pSetLayouts = layouts.data ()
    };

    std::vector<VkDescriptorSet>& sets = _uniformTransfer._sets;
    sets.resize ( uniformCount );

    result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateDescriptorSets ( device, &allocateInfo, sets.data () ),
        "pbr::GeometrySubpassBase::AllocateUniformBufferSystem",
        "Can't allocate descriptor sets"
    );

    if ( !result )
        return false;

    constexpr VkDescriptorBufferInfo bufferTemplate
    {
        .buffer = VK_NULL_HANDLE,
        .offset = 0U,
        .range = static_cast<VkDeviceSize> ( sizeof ( GeometryPassProgram::InstanceData ) )
    };

    _uniformStorage.resize ( uniformCount, bufferTemplate );

    constexpr VkWriteDescriptorSet writeTemplate
    {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = nullptr,
        .dstSet = VK_NULL_HANDLE,
        .dstBinding = 0U,
        .dstArrayElement = 0U,
        .descriptorCount = 1U,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .pImageInfo = nullptr,
        .pBufferInfo = nullptr,
        .pTexelBufferView = nullptr
    };

    std::vector<VkWriteDescriptorSet>& writes = _uniformTransfer._writes;
    writes.resize ( uniformCount, writeTemplate );

    for ( size_t i = 0U; i < uniformCount; ++i )
    {
        VkWriteDescriptorSet& set = writes[ i ];
        set.dstSet = sets[ i ];
        set.pBufferInfo = &_uniformStorage[ i ];
    }

    return true;
}

void GeometrySubpassBase::AppendDrawcalls ( VkCommandBuffer commandBuffer,
    GeometryPassProgram &program,

    RenderSessionStats &renderSessionStats
) noexcept
{
    size_t textureSetIndex = 0U;
    size_t uniformUsed = 0U;
    bool isProgramBind = false;

    VkDescriptorSet const* textureSets = _materialTransfer._sets.data ();
    VkDescriptorSet const* instanceSets = _uniformTransfer._sets.data ();

    constexpr VkDeviceSize offset = 0U;

    for ( auto const& call : _sceneData )
    {
        GeometryCall const& geometryCall = call.second;

        VkDescriptorSet textureSet = textureSets[ textureSetIndex ];
        ++textureSetIndex;

        if ( !isProgramBind )
        {
            program.Bind ( commandBuffer );
            isProgramBind = true;
        }

        bool isUniformBind = false;

        auto instanceDrawer = [ & ] ( MeshRef const &mesh, uint32_t batches ) noexcept {
            if ( isUniformBind )
            {
                program.SetDescriptorSet ( commandBuffer, instanceSets + uniformUsed, 2U, 1U );
            }
            else
            {
                VkDescriptorSet sets[] = { textureSet, instanceSets[ uniformUsed ] };

                program.SetDescriptorSet ( commandBuffer,
                    sets,
                    1U,
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

            ReportGeometry ( renderSessionStats, mesh->GetVertexCount (), batches );
            ++uniformUsed;
        };

        for ( auto const& [mesh, geometryData] : geometryCall.GetUniqueList () )
        {
            if ( !geometryData._isVisible )
                continue;

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

bool GeometrySubpassBase::InitBase ( android_vulkan::Renderer &renderer, VkCommandPool commandPool ) noexcept
{
    VkDevice device = renderer.GetDevice ();

    if ( !_uniformPool.Init ( renderer, sizeof ( GeometryPassProgram::InstanceData ) ) )
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
        "pbr::GeometrySubpassBase::Init",
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

void GeometrySubpassBase::DestroyBase ( VkDevice device ) noexcept
{
    auto freeVector = [] ( auto &v ) noexcept {
        v.clear ();
        v.shrink_to_fit ();
    };

    freeVector ( _materialTransfer._sets );
    freeVector ( _materialTransfer._writes );
    freeVector ( _imageStorage );
    freeVector ( _uniformTransfer._sets );
    freeVector ( _uniformTransfer._writes );
    freeVector ( _uniformStorage );

    DestroyUniformDescriptorPool ( device );
    DestroyMaterialDescriptorPool ( device );

    _transferCommandBuffer = VK_NULL_HANDLE;
    _uniformPool.Destroy ( device );
}

void GeometrySubpassBase::DestroyMaterialDescriptorPool ( VkDevice device ) noexcept
{
    if ( _materialTransfer._pool == VK_NULL_HANDLE )
        return;

    vkDestroyDescriptorPool ( device, _materialTransfer._pool, nullptr );
    _materialTransfer._pool = VK_NULL_HANDLE;
    AV_UNREGISTER_DESCRIPTOR_POOL ( "pbr::GeometrySubpassBase::_materialTransfer._pool" )
}

void GeometrySubpassBase::DestroyUniformDescriptorPool ( VkDevice device ) noexcept
{
    if ( _uniformTransfer._pool == VK_NULL_HANDLE )
        return;

    vkDestroyDescriptorPool ( device, _uniformTransfer._pool, nullptr );
    _uniformTransfer._pool = VK_NULL_HANDLE;
    AV_UNREGISTER_DESCRIPTOR_POOL ( "pbr::GeometrySubpassBase::_uniformTransfer._pool" )
}

} // namespace pbr
