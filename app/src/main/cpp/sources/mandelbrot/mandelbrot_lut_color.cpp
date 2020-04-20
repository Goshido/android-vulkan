#include <mandelbrot/mandelbrot_lut_color.h>
#include <array>
#include <cassert>
#include <cmath>
#include <thread>
#include <vulkan_utils.h>


namespace mandelbrot {

constexpr static const char* FRAGMENT_SHADER = "shaders/mandelbrot-lut-color-ps.spv";
constexpr static const uint32_t LUT_SAMPLE_COUNT = 512U;
constexpr static const VkDeviceSize LUT_SAMPLE_SIZE = 4U;
constexpr static const VkDeviceSize LUT_SIZE = LUT_SAMPLE_COUNT * LUT_SAMPLE_SIZE;
constexpr static const size_t INIT_THREADS = 4U;

MandelbrotLUTColor::MandelbrotLUTColor ():
    MandelbrotBase ( FRAGMENT_SHADER ),
    _lut ( VK_NULL_HANDLE ),
    _lutDeviceMemory ( VK_NULL_HANDLE ),
    _transfer ( VK_NULL_HANDLE ),
    _transferDeviceMemory ( VK_NULL_HANDLE )
{
    // NOTHING
}

bool MandelbrotLUTColor::CreateCommandBuffer ( android_vulkan::Renderer& /*renderer*/ )
{
    assert ( !"MandelbrotLUTColor::CreateCommandBuffer - Implement me!" );
    return false;
}

void MandelbrotLUTColor::DestroyCommandBuffer ( android_vulkan::Renderer& /*renderer*/ )
{
    // TODO
    assert ( !"MandelbrotLUTColor::DestroyCommandBuffer - Implement me!" );
}

bool MandelbrotLUTColor::CreateLUT ( android_vulkan::Renderer &renderer )
{
    VkImageCreateInfo imageInfo;
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.pNext = nullptr;
    imageInfo.flags = 0U;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    imageInfo.imageType = VK_IMAGE_TYPE_1D;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.arrayLayers = 1U;
    imageInfo.mipLevels = 1U;
    imageInfo.extent.width = LUT_SAMPLE_COUNT;
    imageInfo.extent.height = 1U;
    imageInfo.extent.depth = 1U;
    imageInfo.queueFamilyIndexCount = 0U;
    imageInfo.pQueueFamilyIndices = nullptr;

    const VkDevice device = renderer.GetDevice ();

    bool result = renderer.CheckVkResult ( vkCreateImage ( device, &imageInfo, nullptr, &_lut ),
        "MandelbrotLUTColor::CreateLUT",
        "Can't create image"
    );

    if ( !result )
        return false;

    AV_REGISTER_IMAGE ( "MandelbrotLUTColor::_lut" )

    VkMemoryRequirements requirements;
    vkGetImageMemoryRequirements ( device, _lut, &requirements );

    result = TryAllocateMemory ( _lutDeviceMemory,
        renderer,
        requirements,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        "MandelbrotLUTColor::_lutDeviceMemory",
        "Can't allocate LUT memory"
    );

    if ( !result )
    {
        DestroyLUT ( renderer );
        return false;
    }

    VkBufferCreateInfo bufferInfo;
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = nullptr;
    bufferInfo.flags = 0U;
    bufferInfo.size = LUT_SIZE;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.queueFamilyIndexCount = 0U;
    bufferInfo.pQueueFamilyIndices = nullptr;

    result = renderer.CheckVkResult ( vkCreateBuffer ( device, &bufferInfo, nullptr, &_transfer ),
        "MandelbrotLUTColor::CreateLUT",
        "Can't create buffer"
    );

    if ( !result )
    {
        DestroyLUT ( renderer );
        return false;
    }

    AV_REGISTER_BUFFER ( "MandelbrotLUTColor::_transfer" )

    vkGetBufferMemoryRequirements ( device, _transfer, &requirements );

    result = TryAllocateMemory ( _transferDeviceMemory,
        renderer,
        requirements,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        "MandelbrotLUTColor::_transferDeviceMemory",
        "Can't allocate transfer memory"
    );

    if ( !result )
    {
        DestroyLUT ( renderer );
        return false;
    }

    void* data = nullptr;

    result = renderer.CheckVkResult ( vkMapMemory ( device, _transferDeviceMemory, 0U, LUT_SIZE, 0U, &data ),
        "MandelbrotLUTColor::CreateLUT",
        "Can't map transfer memory"
    );

    if ( !result )
    {
        DestroyLUT ( renderer );
        return false;
    }

    InitLUTSamples ( static_cast<uint8_t*> ( data ) );
    vkUnmapMemory ( device, _transferDeviceMemory );

    assert ( !"MandelbrotLUTColor::CreateLUT - Implement me!" );
    return false;
}

void MandelbrotLUTColor::DestroyLUT ( android_vulkan::Renderer &renderer )
{
    const VkDevice device = renderer.GetDevice ();

    if ( _transferDeviceMemory != VK_NULL_HANDLE )
    {
        vkFreeMemory ( device, _transferDeviceMemory, nullptr );
        _transferDeviceMemory = VK_NULL_HANDLE;
        AV_UNREGISTER_DEVICE_MEMORY ( "MandelbrotLUTColor::_transferDeviceMemory" )
    }

    if ( _transfer != VK_NULL_HANDLE )
    {
        vkDestroyBuffer ( device, _transfer, nullptr );
        _transfer = VK_NULL_HANDLE;
        AV_UNREGISTER_BUFFER ( "MandelbrotLUTColor::_transfer" )
    }

    if ( _lutDeviceMemory != VK_NULL_HANDLE )
    {
        vkFreeMemory ( device, _lutDeviceMemory, nullptr );
        _lutDeviceMemory = VK_NULL_HANDLE;
        AV_UNREGISTER_DEVICE_MEMORY ( "MandelbrotLUTColor::_lutDeviceMemory" )
    }

    if ( _lut == VK_NULL_HANDLE )
        return;

    vkDestroyImage ( renderer.GetDevice (), _lut, nullptr );
    _lut = VK_NULL_HANDLE;
    AV_UNREGISTER_IMAGE ( "MandelbrotLUTColor::_lut" )

    assert ( !"MandelbrotLUTColor::DestroyLUT - Implement me!" );
}

void MandelbrotLUTColor::InitLUTSamples ( uint8_t* samples ) const
{
    constexpr const auto samplePerThread = static_cast<const size_t> ( LUT_SAMPLE_COUNT / INIT_THREADS );

    auto job = [ samples ] ( size_t startIndex ) {
        constexpr const float twoPi = 6.28318F;
        constexpr const float hueOffsetGreen = 2.09439F;
        constexpr const float hueOffsetBlue = 4.18879F;
        constexpr const float sampleToAngle = twoPi / static_cast<float> ( LUT_SAMPLE_COUNT );

        auto evaluator = [] ( float angle ) -> uint8_t {
            const float n = std::sinf ( angle ) * 0.5F + 0.5F;
            return static_cast<uint8_t> ( n * 255.0F + 0.5F );
        };

        const size_t limit = startIndex + samplePerThread;
        uint8_t* write = samples + startIndex;

        for ( size_t i = startIndex; i < limit; ++i )
        {
            const float pivot = i * sampleToAngle;

            write[ 0U ] = evaluator ( pivot );
            write[ 1U ] = evaluator ( pivot + hueOffsetGreen );
            write[ 2U ] = evaluator ( pivot + hueOffsetBlue );
            write[ 3U ] = 0xFFU;

            write += LUT_SAMPLE_SIZE;
        }
    };

    std::array<std::thread, INIT_THREADS> threads;

    for ( size_t i = 0U; i < INIT_THREADS; ++i )
        threads[ i ] = std::thread ( job, i * samplePerThread );

    for ( size_t i = 0U; i < INIT_THREADS; ++i )
    {
        threads[ i ].join ();
    }
}

bool MandelbrotLUTColor::TryAllocateMemory ( VkDeviceMemory &memory,
    android_vulkan::Renderer &renderer,
    const VkMemoryRequirements &requirements,
    VkMemoryPropertyFlags memoryProperties,
    const char* where,
    const char* checkFailMessage
) const
{
    VkMemoryAllocateInfo allocateInfo;
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.pNext = nullptr;
    allocateInfo.allocationSize = requirements.size;

    bool result = renderer.SelectTargetMemoryTypeIndex ( allocateInfo.memoryTypeIndex,
        requirements,
        memoryProperties
    );

    if ( !result )
        return false;

    result = renderer.CheckVkResult (
        vkAllocateMemory ( renderer.GetDevice (), &allocateInfo, nullptr, &memory ),
        "MandelbrotLUTColor::TryAllocateMemory",
        checkFailMessage
    );

    if ( !result )
        return false;

    AV_REGISTER_DEVICE_MEMORY ( std::move ( where ) )
    return true;
}

} // namespace mandelbrot
