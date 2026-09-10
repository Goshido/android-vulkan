#ifndef INDEX_STREAM_HLSL
#define INDEX_STREAM_HLSL


#define VK_INDEX_TYPE_UINT16        0U
#define VK_INDEX_TYPE_UINT32        1U
#define VK_INDEX_TYPE_NONE_KHR      1000165000U


typedef vk::BufferPointer<uint16_t, 2U>     Indices16;
typedef vk::BufferPointer<uint32_t, 4U>     Indices32;


#endif // INDEX_STREAM_HLSL
