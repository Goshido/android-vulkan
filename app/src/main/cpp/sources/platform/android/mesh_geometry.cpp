#include <precompiled_headers.hpp>
#include <android_vulkan_sdk/mesh2.hpp>
#include <av_assert.hpp>
#include <file.hpp>
#include <logger.hpp>
#include <platform/android/mesh_geometry.hpp>
#include <vulkan_utils.hpp>


namespace android_vulkan {

namespace {

constexpr VkBufferUsageFlags NO_INDEX_USAGE = AV_VK_FLAG ( VK_BUFFER_USAGE_VERTEX_BUFFER_BIT ) |
    AV_VK_FLAG ( VK_BUFFER_USAGE_STORAGE_BUFFER_BIT );

constexpr VkBufferUsageFlags INDEX_USAGE = AV_VK_FLAG ( VK_BUFFER_USAGE_INDEX_BUFFER_BIT ) |
    AV_VK_FLAG ( VK_BUFFER_USAGE_VERTEX_BUFFER_BIT ) |
    AV_VK_FLAG ( VK_BUFFER_USAGE_STORAGE_BUFFER_BIT );

constexpr VkMemoryPropertyFlags MEMORY_PROPERTIES = AV_VK_FLAG ( VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT ) |
    AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT ) |
    AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

void MeshGeometry::FreeResources ( Renderer &renderer ) noexcept
{
    _vertexCount = 0U;

    if ( VkBuffer &buffer = _meshBufferInfo._buffer; buffer != VK_NULL_HANDLE ) [[likely]]
        vkDestroyBuffer ( renderer.GetDevice (), std::exchange ( buffer, VK_NULL_HANDLE ), nullptr );

    if ( _gpuAllocation._memory == VK_NULL_HANDLE ) [[unlikely]]
        return;

    renderer.FreeMemory ( std::exchange ( _gpuAllocation._memory, VK_NULL_HANDLE ),
        std::exchange ( _gpuAllocation._offset, 0U )
    );
}

MeshBufferInfo const &MeshGeometry::GetMeshBufferInfo () const noexcept
{
    return _meshBufferInfo;
}

[[maybe_unused]] bool MeshGeometry::LoadMesh ( Renderer &renderer, std::string &&fileName ) noexcept
{
    if ( fileName.empty () ) [[unlikely]]
    {
        LogError ( "MeshGeometry::LoadMesh - Can't upload data. Filename is empty." );
        return false;
    }

    static std::regex const isMesh2 ( R"__(^.+?\.mesh2$)__" );
    std::smatch match;

    if ( !std::regex_match ( fileName, match, isMesh2 ) ) [[unlikely]]
    {
        LogError ( "MeshGeometry::LoadMesh - Mesh format is not supported: %s", fileName.c_str () );
        return false;
    }

    return LoadFromMesh2 ( renderer, std::move ( fileName ) );
}

[[maybe_unused]] bool MeshGeometry::LoadMesh ( Renderer &renderer,
    AbstractData data,
    uint32_t vertexCount
) noexcept
{
    FreeResources ( renderer );

    VkBufferCreateInfo const bufferInfo
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .size =  static_cast<VkDeviceSize> ( data.size () ),
        .usage = NO_INDEX_USAGE,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0U,
        .pQueueFamilyIndices = nullptr
    };

    bool result = CreateBuffer ( renderer,
        _meshBufferInfo._buffer,
        _gpuAllocation,
        bufferInfo,
        MEMORY_PROPERTIES,
        "Mesh"
    );

    if ( !result ) [[unlikely]]
        return false;

    UploadJob const job
    {
        ._data = data.data (),
        ._dstOffset = 0U,
        ._size = _gpuAllocation._range
    };

    result = GPUTransfer ( renderer, { &job, 1U } );

    if ( !result ) [[unlikely]]
        return false;

    CommitMeshInfo ( VK_INDEX_TYPE_NONE_KHR,

        {
            ._offset = 0U,
            ._range = static_cast<size_t> ( _gpuAllocation._range )
        },

        std::nullopt
    );

    _vertexCount = vertexCount;
    _vertexBufferVertexCount = vertexCount;
    return true;
}

[[maybe_unused]] bool MeshGeometry::LoadMesh ( Renderer &renderer,
    Indices16 indices,
    Positions positions,
    GXAABB const &bounds
) noexcept
{
    FreeResources ( renderer );

    size_t const indexCount = indices.size ();
    size_t const positionCount = positions.size ();

    bool const result = Upload ( renderer,
        { reinterpret_cast<uint8_t const*> ( indices.data () ), indexCount * sizeof ( uint16_t ) },
        VK_INDEX_TYPE_UINT16,
        { reinterpret_cast<uint8_t const*> ( positions.data () ), positionCount * sizeof ( GXVec3 ) },
        {},
        static_cast<uint32_t> ( positionCount )
    );

    if ( !result ) [[unlikely]]
        return false;

    _vertexCount = static_cast<uint32_t> ( indexCount );
    _vertexBufferVertexCount = static_cast<uint32_t> ( positionCount );
    _bounds = bounds;
    return true;
}

[[maybe_unused]] bool MeshGeometry::LoadMesh ( Renderer &renderer,
    Indices32 indices,
    Positions positions,
    GXAABB const &bounds
) noexcept
{
    FreeResources ( renderer );

    size_t const indexCount = indices.size ();
    size_t const positionCount = positions.size ();

    bool const result = Upload ( renderer,
        { reinterpret_cast<uint8_t const*> ( indices.data () ), indexCount * sizeof ( uint32_t ) },
        VK_INDEX_TYPE_UINT32,
        { reinterpret_cast<uint8_t const*> ( positions.data () ), positionCount * sizeof ( GXVec3 ) },
        {},
        static_cast<uint32_t> ( positionCount )
    );

    if ( !result ) [[unlikely]]
        return false;

    _vertexCount = static_cast<uint32_t> ( indexCount );
    _vertexBufferVertexCount = static_cast<uint32_t> ( positionCount );
    _bounds = bounds;
    return true;
}

[[maybe_unused]] bool MeshGeometry::LoadMesh ( Renderer &renderer,
    Indices16 indices,
    Positions positions,
    Vertices vertices,
    GXAABB const &bounds
) noexcept
{
    FreeResources ( renderer );

    size_t const indexCount = indices.size ();
    size_t const positionCount = positions.size ();
    size_t const vertexCount = vertices.size ();

    AV_ASSERT ( positionCount == vertexCount )

    bool const result = Upload ( renderer,
        { reinterpret_cast<uint8_t const*> ( indices.data () ), indexCount * sizeof ( uint16_t ) },
        VK_INDEX_TYPE_UINT16,
        { reinterpret_cast<uint8_t const*> ( positions.data () ), positionCount * sizeof ( GXVec3 ) },
        { vertices.data (), vertexCount },
        static_cast<uint32_t> ( positionCount )
    );

    if ( !result ) [[unlikely]]
        return false;

    _vertexCount = static_cast<uint32_t> ( indexCount );
    _vertexBufferVertexCount = static_cast<uint32_t> ( positionCount );
    _bounds = bounds;
    return true;
}

[[maybe_unused]] bool MeshGeometry::LoadMesh ( Renderer &renderer,
    Indices32 indices,
    Positions positions,
    Vertices vertices,
    GXAABB const &bounds
) noexcept
{
    FreeResources ( renderer );

    size_t const indexCount = indices.size ();
    size_t const positionCount = positions.size ();
    size_t const vertexCount = vertices.size ();

    AV_ASSERT ( positionCount == vertexCount )

    bool const result = Upload ( renderer,
        { reinterpret_cast<uint8_t const*> ( indices.data () ), indexCount * sizeof ( uint32_t ) },
        VK_INDEX_TYPE_UINT32,
        { reinterpret_cast<uint8_t const*> ( positions.data () ), positionCount * sizeof ( GXVec3 ) },
        { vertices.data (), vertexCount },
        static_cast<uint32_t> ( positionCount )
    );

    if ( !result ) [[unlikely]]
        return false;

    _vertexCount = static_cast<uint32_t> ( indexCount );
    _vertexBufferVertexCount = static_cast<uint32_t> ( positionCount );
    _bounds = bounds;
    return true;
}

void MeshGeometry::CommitMeshInfo ( VkIndexType indexType,
    StreamInfo &&stream0,
    std::optional<StreamInfo> &&stream1
) noexcept
{
    _meshBufferInfo._indexType = indexType;

    constexpr auto assign = [] ( VkDeviceSize &offset, VkDeviceSize &range, StreamInfo const &stream ) noexcept {
        offset = static_cast<VkDeviceSize> ( stream._offset );
        range = static_cast<VkDeviceSize> ( stream._range );
    };

    assign ( _meshBufferInfo._vertexDataOffsets[ 0U ], _meshBufferInfo._vertexDataRanges[ 0U ], stream0 );

    if ( stream1 )
    {
        assign ( _meshBufferInfo._vertexDataOffsets[ 1U ], _meshBufferInfo._vertexDataRanges[ 1U ], *stream1 );
    }
}

bool MeshGeometry::GPUTransfer ( Renderer &renderer, UploadJobs jobs ) noexcept
{
    uint8_t* writePtr = nullptr;

    bool const result = renderer.MapMemory ( reinterpret_cast<void* &> ( writePtr ),
        _gpuAllocation._memory,
        _gpuAllocation._offset,
        "MeshGeometry::GPUTransfer",
        "Can't map data"
    );

    if ( !result ) [[unlikely]]
        return false;

    for ( UploadJob const &job : jobs )
        std::memcpy ( writePtr + job._dstOffset, job._data , job._size );

    renderer.UnmapMemory ( _gpuAllocation._memory );
    return true;
}

bool MeshGeometry::LoadFromMesh2 ( Renderer &renderer, std::string &&fileName ) noexcept
{
    File file ( fileName );

    if ( !file.LoadContent () ) [[unlikely]]
        return false;

    std::vector<uint8_t> const &content = file.GetContent ();
    uint8_t const* rawData = content.data ();
    auto const &header = *reinterpret_cast<Mesh2Header const*> ( rawData );

    auto const selector = static_cast<size_t> ( header._indexCount >= INDEX16_LIMIT );
    constexpr VkIndexType const cases[] = { VK_INDEX_TYPE_UINT16, VK_INDEX_TYPE_UINT32 };

    bool const result = Upload ( renderer,

        {
            rawData + static_cast<size_t> ( header._indexDataOffset ),
            header._indexCount * INDEX_SIZES[ selector ]
        },

        cases[ selector ],

        {
            rawData + static_cast<size_t> ( header._positionDataOffset ),
            static_cast<size_t> ( header._vertexCount ) * sizeof ( GXVec3 )
        },

        {
            reinterpret_cast<VertexInfo const*> ( rawData + static_cast<size_t> ( header._vertexDataOffset ) ),
            static_cast<size_t> ( header._vertexCount )
        },

        static_cast<uint32_t> ( header._vertexCount )
    );

    if ( !result ) [[unlikely]]
        return false;

    Vec3 const &mins = header._bounds._min;
    Vec3 const &maxs = header._bounds._max;
    _bounds.Empty ();
    _bounds.AddVertex ( mins[ 0U ], mins[ 1U ], mins[ 2U ] );
    _bounds.AddVertex ( maxs[ 0U ], maxs[ 1U ], maxs[ 2U ] );

    _vertexCount = static_cast<uint32_t> ( header._indexCount );
    _vertexBufferVertexCount = header._vertexCount;
    _fileName = std::move ( fileName );
    return true;
}

bool MeshGeometry::Upload ( Renderer &renderer,
    AbstractData indices,
    VkIndexType indexType,
    AbstractData vertexStream0,
    Vertices vertexStream1,
    uint32_t vertexCount
) noexcept
{
    AV_ASSERT ( indexType == VK_INDEX_TYPE_UINT16 || indexType == VK_INDEX_TYPE_UINT32 )

    size_t const indSize = indices.size ();

    size_t const posSize = vertexStream0.size ();
    auto const restSize = static_cast<size_t> ( vertexCount * sizeof ( VertexInfo ) );

    size_t const vertexAlignment = std::max ( static_cast<size_t> ( 4U ),
        renderer.GetMinStorageBufferOffsetAlignment ()
    );

    size_t sizeLeft = vertexAlignment + posSize + vertexAlignment + restSize;
    auto* ptr = reinterpret_cast<void*> ( indSize );

    auto const posOffset = reinterpret_cast<size_t> ( std::align ( vertexAlignment, posSize, ptr, sizeLeft ) );
    ptr = static_cast<uint8_t*> ( ptr ) + posSize;
    sizeLeft -= posSize;

    auto const restOffset = reinterpret_cast<size_t> ( std::align ( vertexAlignment, restSize, ptr, sizeLeft ) );
    constexpr VkBufferUsageFlags const usageCases[] = { INDEX_USAGE, NO_INDEX_USAGE };

    VkBufferCreateInfo const bufferInfo
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .size = static_cast<VkDeviceSize> ( restOffset + restSize ),
        .usage = usageCases[ static_cast<size_t> ( indexType == VK_INDEX_TYPE_NONE_KHR ) ],
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0U,
        .pQueueFamilyIndices = nullptr
    };

    bool result = CreateBuffer ( renderer,
        _meshBufferInfo._buffer,
        _gpuAllocation,
        bufferInfo,
        MEMORY_PROPERTIES,
        "Mesh"
    );

    if ( !result ) [[unlikely]]
        return false;

    if ( vertexStream1.empty () )
    {
        UploadJob const jobs[]
            {
                {
                    ._data = indices.data (),
                    ._dstOffset = 0U,
                    ._size = static_cast<VkDeviceSize> ( indSize )
                },
                {
                    ._data = vertexStream0.data (),
                    ._dstOffset = static_cast<VkDeviceSize> ( posOffset ),
                    ._size = static_cast<VkDeviceSize> ( posSize )
                }
            };

        if ( result = GPUTransfer ( renderer, { jobs, std::size ( jobs ) } ); !result ) [[unlikely]]
            return false;

        CommitMeshInfo ( indexType,

            {
                ._offset = posOffset,
                ._range = posSize
            },

            std::nullopt
        );

        return true;
    }

    UploadJob const jobs[]
    {
        {
            ._data = indices.data (),
            ._dstOffset = 0U,
            ._size = static_cast<VkDeviceSize> ( indSize )
        },
        {
            ._data = vertexStream0.data (),
            ._dstOffset = static_cast<VkDeviceSize> ( posOffset ),
            ._size = static_cast<VkDeviceSize> ( posSize )
        },
        {
            ._data = vertexStream1.data (),
            ._dstOffset = static_cast<VkDeviceSize> ( restOffset ),
            ._size = static_cast<VkDeviceSize> ( restSize )
        }
    };

    if ( result = GPUTransfer ( renderer, { jobs, std::size ( jobs ) } ); !result ) [[unlikely]]
        return false;

    CommitMeshInfo ( indexType,

        {
            ._offset = posOffset,
            ._range = posSize
        },

        std::optional<StreamInfo> {
            {
                ._offset = restOffset,
                ._range = restSize
            }
        }
    );

    return true;
}

} // namespace android_vulkan
