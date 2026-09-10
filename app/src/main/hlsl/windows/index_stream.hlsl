#ifndef INDEX_STREAM_HLSL
#define INDEX_STREAM_HLSL


// From Vulkan spec
#define VK_INDEX_TYPE_UINT16        0U
#define VK_INDEX_TYPE_UINT32        1U
#define VK_INDEX_TYPE_NONE_KHR      1000165000U


enum class eIndex: uint32_t
{
    UINT16 = VK_INDEX_TYPE_UINT16,
    UINT32 = VK_INDEX_TYPE_UINT32,
    None = VK_INDEX_TYPE_NONE_KHR
};

typedef vk::BufferPointer<uint16_t, 2U>     Indices16;
typedef vk::BufferPointer<uint32_t, 4U>     Indices32;

//----------------------------------------------------------------------------------------------------------------------

uint32_t ResolveIndex ( in uint64_t indexStream, in eIndex indexType, in uint32_t vertexID )
{
    switch ( indexType )
    {
        case eIndex::None:
        return vertexID;

        case eIndex::UINT16:
        return (uint32_t)Indices16 ( indexStream + (uint64_t)( vertexID * sizeof ( uint16_t ) ) ).Get ();

        case eIndex::UINT32:
        return Indices32 ( indexStream + (uint64_t)( vertexID * sizeof ( uint32_t ) ) ).Get ();

        default:
            // IMPOSSIBLE
        break;
    }

    return 0U;
}


#endif // INDEX_STREAM_HLSL
