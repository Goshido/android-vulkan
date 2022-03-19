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

void GeometrySubpassBase::AppendDrawcalls ( VkCommandBuffer commandBuffer,
    GeometryPassProgram &program,
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
            program.Bind ( commandBuffer );
            isProgramBind = true;
        }

        bool isUniformBind = false;

        auto instanceDrawer = [ & ] ( MeshRef const &mesh, uint32_t batches ) noexcept {
            if ( isUniformBind )
            {
                program.SetDescriptorSet ( commandBuffer, instanceSets + uniformUsed, 1U, 1U );
            }
            else
            {
                VkDescriptorSet sets[] = { textureSet, instanceSets[ uniformUsed ] };

                program.SetDescriptorSet ( commandBuffer,
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

            renderSessionStats.RenderOpaque ( mesh->GetVertexCount (), batches );
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
        "GeometrySubpassBase::Init",
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

    freeVector ( _descriptorSetStorage );
    freeVector ( _imageStorage );
    freeVector ( _layouts );
    freeVector ( _poolSizeStorage );
    freeVector ( _uniformStorage );
    freeVector ( _writeStorage0 );
    freeVector ( _writeStorage1 );

    DestroyDescriptorPool ( device );

    _transferCommandBuffer = VK_NULL_HANDLE;
    _uniformPool.Destroy ( device );
}

void GeometrySubpassBase::DestroyDescriptorPool ( VkDevice device ) noexcept
{
    if ( _descriptorPool == VK_NULL_HANDLE )
        return;

    vkDestroyDescriptorPool ( device, _descriptorPool, nullptr );
    _descriptorPool = VK_NULL_HANDLE;
    AV_UNREGISTER_DESCRIPTOR_POOL ( "GeometrySubpassBase::_descriptorPool" )
}

bool GeometrySubpassBase::RecreateDescriptorPool ( VkDevice device, size_t maxSets ) noexcept
{
    DestroyDescriptorPool ( device );

    VkDescriptorPoolCreateInfo const poolInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .maxSets = static_cast<uint32_t> ( maxSets ),
        .poolSizeCount = static_cast<uint32_t> ( _poolSizeStorage.size () ),
        .pPoolSizes = _poolSizeStorage.data ()
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateDescriptorPool ( device, &poolInfo, nullptr, &_descriptorPool ),
        "GeometrySubpassBase::UpdateGPUData",
        "Can't create descriptor pool"
    );

    if ( !result )
        return false;

    AV_REGISTER_DESCRIPTOR_POOL ( "GeometrySubpassBase::_descriptorPool" )
    return true;
}

} // namespace pbr
