#include <precompiled_headers.hpp>
#include <av_assert.hpp>
#include <pbr/geometry_pass_binds.inc>
#include <pbr/geometry_pool.hpp>


namespace pbr {

namespace {

constexpr size_t BUFFERS_PER_DESCRIPTOR_SET = 3U;
constexpr size_t POSITION_INDEX = 0U;
constexpr size_t NORMAL_INDEX = 1U;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

VkDescriptorSet GeometryPool::Acquire () noexcept
{
    VkDescriptorSet set = _descriptorSets[ _readIndex ];
    _readIndex = ( _readIndex + 1U ) % _descriptorSets.size ();
    return set;
}

void GeometryPool::Commit () noexcept
{
    _baseIndex = _writeIndex;
    _readIndex = _writeIndex;
    _written = 0U;
}

bool GeometryPool::HasNewData () const noexcept
{
    return _readIndex != _writeIndex;
}

bool GeometryPool::IssueSync ( VkDevice device ) const noexcept
{
    size_t const count = _descriptorSets.size ();
    size_t const idx = _baseIndex + _written;
    size_t const cases[] = { 0U, idx - count };
    size_t const more = cases[ static_cast<size_t> ( idx > count ) ];
    size_t const available = _written - more;

    VkMappedMemoryRange const* vertexRanges = _vertexRanges.data ();
    auto const av = static_cast<uint32_t> ( available );
    VkMappedMemoryRange const* fragmentRanges = _fragmentRanges.data ();

    bool const result = android_vulkan::Renderer::CheckVkResult (
            vkFlushMappedMemoryRanges ( device, av << 1U, vertexRanges + ( _baseIndex << 1U ) ),
            "pbr::GeometryPool::IssueSync",
            "Can't flush vertex memory ranges (a)"
        ) &&

        android_vulkan::Renderer::CheckVkResult (
            vkFlushMappedMemoryRanges ( device, av, fragmentRanges + _baseIndex ),
            "pbr::GeometryPool::IssueSync",
            "Can't flush fragment memory ranges (a)"
        );

    if ( !result ) [[unlikely]]
        return false;

    if ( more < 1U ) [[likely]]
        return true;

    return android_vulkan::Renderer::CheckVkResult (
            vkFlushMappedMemoryRanges ( device, static_cast<uint32_t> ( more << 1U ), vertexRanges ),
            "pbr::GeometryPool::IssueSync",
            "Can't flush vertex memory ranges (b)"
        ) &&

        android_vulkan::Renderer::CheckVkResult (
            vkFlushMappedMemoryRanges ( device, static_cast<uint32_t> ( more ), fragmentRanges ),
            "pbr::GeometryPool::IssueSync",
            "Can't flush fragment memory ranges (b)"
        );
}

void GeometryPool::Push ( GeometryPassProgram::InstancePositionData const &positionData,
    GeometryPassProgram::InstanceNormalData const &normalData,
    GeometryPassProgram::InstanceColorData const &colorData,
    size_t items
) noexcept
{
    size_t const positionDataSize = items * sizeof ( GXMat4 );
    _positionPool.Push ( &positionData, positionDataSize );

    size_t const normalDataSize = ( ( items + 1U ) >> 1U ) * sizeof ( GeometryPassProgram::TBN64 );
    _normalPool.Push ( &normalData, normalDataSize );

    size_t const colorDataSize = items * sizeof ( GeometryPassProgram::ColorData );
    _colorPool.Push ( &colorData, colorDataSize );

    constexpr auto hwSize = [] ( size_t size, size_t nonCoherentAtomSize ) noexcept -> VkDeviceSize
    {
        size_t const alpha = size - 1U;
        return static_cast<VkDeviceSize> ( alpha + nonCoherentAtomSize - ( alpha % nonCoherentAtomSize ) );
    };

    VkMappedMemoryRange* vertexRanges = _vertexRanges.data () + ( _writeIndex << 1U );
    vertexRanges[ POSITION_INDEX ].size = hwSize ( positionDataSize, _nonCoherentAtomSize );
    vertexRanges[ NORMAL_INDEX ].size = hwSize ( normalDataSize, _nonCoherentAtomSize );

    _fragmentRanges[ _writeIndex ].size = hwSize ( colorDataSize, _nonCoherentAtomSize );
    _writeIndex = ( _writeIndex + 1U ) % _descriptorSets.size ();

    if ( _writeIndex == 0U ) [[unlikely]]
    {
        _positionPool.Reset ();
        _normalPool.Reset ();
        _colorPool.Reset ();
    }

    ++_written;
}

bool GeometryPool::Init ( android_vulkan::Renderer &renderer ) noexcept
{
    VkDevice device = renderer.GetDevice ();
    _nonCoherentAtomSize = renderer.GetNonCoherentAtomSize ();

    bool result = _descriptorSetLayout.Init ( device ) &&

        _positionPool.Init ( renderer,
            eUniformPoolSize::Big_32M,
            sizeof ( GeometryPassProgram::InstancePositionData ),
            "Position transform"
        ) &&

        _normalPool.Init ( renderer,
            eUniformPoolSize::Tiny_4M,
            sizeof ( GeometryPassProgram::InstanceNormalData ),
            "Normal transform"
        ) &&

        _colorPool.Init ( renderer,
            eUniformPoolSize::Small_8M,
            sizeof ( GeometryPassProgram::InstanceColorData ),
            "Color data"
        );

    if ( !result ) [[unlikely]]
        return false;

    size_t const positionCount = _positionPool.GetAvailableItemCount ();
    size_t const normalCount = _normalPool.GetAvailableItemCount ();
    size_t const colorCount = _colorPool.GetAvailableItemCount ();

    size_t const logicalCount = std::min ( { positionCount, normalCount, colorCount } );
    size_t const bufferCount = BUFFERS_PER_DESCRIPTOR_SET * logicalCount;

    VkDescriptorPoolSize const poolSizes =
    {
        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = static_cast<uint32_t> ( bufferCount )
    };

    VkDescriptorPoolCreateInfo const poolInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .maxSets = static_cast<uint32_t> ( logicalCount ),
        .poolSizeCount = 1U,
        .pPoolSizes = &poolSizes
    };

    result = android_vulkan::Renderer::CheckVkResult (
        vkCreateDescriptorPool ( device, &poolInfo, nullptr, &_descriptorPool ),
        "pbr::GeometryPool::Init",
        "Can't create descriptor pool"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _descriptorPool, VK_OBJECT_TYPE_DESCRIPTOR_POOL, "Geometry pool" )

    _descriptorSets.resize ( logicalCount, VK_NULL_HANDLE );
    VkDescriptorSet* descriptorSets = _descriptorSets.data ();
    std::vector<VkDescriptorSetLayout> const layouts ( logicalCount, _descriptorSetLayout.GetLayout () );

    VkDescriptorSetAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = _descriptorPool,
        .descriptorSetCount = poolInfo.maxSets,
        .pSetLayouts = layouts.data ()
    };

    result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateDescriptorSets ( device, &allocateInfo, descriptorSets ),
        "pbr::GeometryPool::Init",
        "Can't allocate descriptor sets"
    );

    if ( !result ) [[unlikely]]
        return false;

#if defined ( AV_ENABLE_VVL ) || defined ( AV_ENABLE_RENDERDOC )

    for ( size_t i = 0U; i < logicalCount; ++i )
        AV_SET_VULKAN_OBJECT_NAME ( device, descriptorSets[ i ], VK_OBJECT_TYPE_DESCRIPTOR_SET, "Geometry #%zu", i )

#endif // AV_ENABLE_VVL || AV_ENABLE_RENDERDOC

    // Initialize all immutable constant fields.

    constexpr VkDescriptorBufferInfo bufferTemplate
    {
        .buffer = VK_NULL_HANDLE,
        .offset = 0U,
        .range = 0U
    };

    std::vector<VkDescriptorBufferInfo> bufferInfoStorage ( bufferCount, bufferTemplate );

    constexpr VkWriteDescriptorSet writeSetTemplate
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

    std::vector<VkWriteDescriptorSet> writeSetStorage ( bufferCount, writeSetTemplate );

    constexpr VkMappedMemoryRange rangeTemplate
    {
        .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
        .pNext = nullptr,
        .memory = VK_NULL_HANDLE,
        .offset = 0U,
        .size = 0U
    };

    _vertexRanges.resize ( logicalCount << 1U, rangeTemplate );
    _fragmentRanges.resize ( logicalCount, rangeTemplate );

    VkMappedMemoryRange* vertexRange = _vertexRanges.data ();
    VkMappedMemoryRange* fragmentRange = _fragmentRanges.data ();

    VkDescriptorBufferInfo* bufferInfo = bufferInfoStorage.data ();
    VkWriteDescriptorSet* writeSet = writeSetStorage.data ();

    UMAUniformBuffer::BufferInfo positionInfo = _positionPool.GetBufferInfo ();
    auto const pSize = static_cast<VkDeviceSize> ( positionInfo._stepSize );
    VkDeviceSize positionBufferOffset = 0U;

    UMAUniformBuffer::BufferInfo normalInfo = _normalPool.GetBufferInfo ();
    auto const nSize = static_cast<VkDeviceSize> ( normalInfo._stepSize );
    VkDeviceSize normalBufferOffset = 0U;

    UMAUniformBuffer::BufferInfo colorInfo = _colorPool.GetBufferInfo ();
    auto const cSize = static_cast<VkDeviceSize> ( colorInfo._stepSize );
    VkDeviceSize colorBufferOffset = 0U;

    for ( size_t i = 0U; i < logicalCount; ++i )
    {
        VkDescriptorSet descriptorSet = descriptorSets[ i ];

        vertexRange->memory = positionInfo._memory;
        ( vertexRange++ )->offset = positionInfo._offset;

        VkDescriptorBufferInfo* positionBufferInfo = bufferInfo++;

        *positionBufferInfo =
        {
            .buffer = positionInfo._buffer,
            .offset = positionBufferOffset,
            .range = pSize
        };

        positionBufferOffset += pSize;
        positionInfo._offset += pSize;

        VkWriteDescriptorSet &positionWrite = *writeSet++;
        positionWrite.dstSet = descriptorSet;
        positionWrite.dstBinding = BIND_INSTANCE_POSITON_DATA;
        positionWrite.pBufferInfo = positionBufferInfo;

        vertexRange->memory = normalInfo._memory;
        ( vertexRange++ )->offset = normalInfo._offset;

        VkDescriptorBufferInfo* normalBufferInfo = bufferInfo++;

        *normalBufferInfo =
        {
            .buffer = normalInfo._buffer,
            .offset = normalBufferOffset,
            .range = nSize
        };

        normalBufferOffset += nSize;
        normalInfo._offset += nSize;

        VkWriteDescriptorSet &normalWrite = *writeSet++;
        normalWrite.dstSet = descriptorSet;
        normalWrite.dstBinding = BIND_INSTANCE_NORMAL_DATA;
        normalWrite.pBufferInfo = normalBufferInfo;

        fragmentRange->memory = colorInfo._memory;
        ( fragmentRange++ )->offset = colorInfo._offset;

        VkDescriptorBufferInfo* colorBufferInfo = bufferInfo++;

        *colorBufferInfo =
        {
            .buffer = colorInfo._buffer,
            .offset = colorBufferOffset,
            .range = cSize
        };

        colorBufferOffset += cSize;
        colorInfo._offset += cSize;

        VkWriteDescriptorSet &colorWrite = *writeSet++;
        colorWrite.dstSet = descriptorSet;
        colorWrite.dstBinding = BIND_INSTANCE_COLOR_DATA;
        colorWrite.pBufferInfo = colorBufferInfo;
    }

    vkUpdateDescriptorSets ( device,
        static_cast<uint32_t> ( writeSetStorage.size () ),
        writeSetStorage.data (),
        0U,
        nullptr
    );

    // Now all what is needed to do is to init "VkBufferMemoryBarrier::size".
    _baseIndex = 0U;
    return true;
}

void GeometryPool::Destroy ( android_vulkan::Renderer &renderer ) noexcept
{
    VkDevice device = renderer.GetDevice ();

    if ( _descriptorPool != VK_NULL_HANDLE ) [[likely]]
    {
        vkDestroyDescriptorPool ( device, _descriptorPool, nullptr );
        _descriptorPool = VK_NULL_HANDLE;
    }

    _colorPool.Destroy ( renderer );

    constexpr auto clean = [] ( auto &vector ) noexcept {
        vector.clear ();
        vector.shrink_to_fit ();
    };

    clean ( _vertexRanges );
    clean ( _fragmentRanges );
    clean ( _descriptorSets );

    _positionPool.Destroy ( renderer );
    _normalPool.Destroy ( renderer );
    _colorPool.Destroy ( renderer );

    _descriptorSetLayout.Destroy ( device );
}

} // namespace pbr
