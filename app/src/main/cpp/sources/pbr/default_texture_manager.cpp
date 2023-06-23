#include <pbr/default_texture_manager.h>
#include <half_types.h>


namespace pbr {

void DefaultTextureManager::FreeTransferResources ( android_vulkan::Renderer &renderer,
    VkCommandPool commandPool ) noexcept
{
    auto const freeJob = [ &renderer ] ( Texture2DRef &texture ) noexcept {
        if ( !texture )
            return;

        texture->FreeTransferResources ( renderer );
    };

    freeJob ( _albedo );
    freeJob ( _emission );
    freeJob ( _mask );
    freeJob ( _normal );
    freeJob ( _params );
    freeJob ( _transparent );

    vkFreeCommandBuffers ( renderer.GetDevice (),
        commandPool,
        static_cast<uint32_t> ( BUFFER_COUNT ),
        _commandBuffers
    );

    for ( auto& c : _commandBuffers )
    {
        c = VK_NULL_HANDLE;
    }
}

bool DefaultTextureManager::Init ( android_vulkan::Renderer &renderer, VkCommandPool commandPool ) noexcept
{
    VkCommandBufferAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = static_cast<uint32_t> ( BUFFER_COUNT )
    };

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateCommandBuffers ( renderer.GetDevice (), &allocateInfo, _commandBuffers ),
        "pbr::DefaultTextureManager::Init",
        "Can't allocate command buffers"
    );

    if ( !result )
        return false;

    auto const textureLoader = [ &renderer ] ( Texture2DRef &texture,
        uint8_t const* data,
        size_t size,
        VkFormat format,
        VkCommandBuffer commandBuffer
    ) noexcept -> bool {
        texture = std::make_shared<android_vulkan::Texture2D> ();

        constexpr VkExtent2D resolution
        {
            .width = 1U,
            .height = 1U
        };

        bool const result = texture->UploadData ( renderer,
            data,
            size,
            resolution,
            format,
            false,
            commandBuffer,
            VK_NULL_HANDLE
        );

        if ( result )
            return true;

        texture = nullptr;
        return false;
    };

    constexpr uint8_t const albedoData[ 4U ] = { 255U, 255U, 255U, 255U };

    if ( !textureLoader ( _albedo, albedoData, sizeof ( albedoData ), VK_FORMAT_R8G8B8A8_SRGB, _commandBuffers[ 0U ] ) )
        return false;

    android_vulkan::Half4 const emissionData ( 0.0F, 0.0F, 0.0F, 0.0F );

    result = textureLoader ( _emission,
        reinterpret_cast<const uint8_t*> ( emissionData._data ),
        sizeof ( emissionData ),
        VK_FORMAT_R16G16B16A16_SFLOAT,
        _commandBuffers[ 1U ]
    );

    if ( !result )
        return false;

    constexpr uint8_t const maskData[ 4U ] = { 255U, 0U, 0U, 0U };

    if ( !textureLoader ( _mask, maskData, sizeof ( maskData ), VK_FORMAT_R8G8B8A8_UNORM, _commandBuffers[ 2U ] ) )
        return false;

    constexpr uint8_t const normalData[ 4U ] = { 128U, 0U, 0U, 128U };

    result = textureLoader ( _normal,
        normalData,
        sizeof ( normalData ),
        VK_FORMAT_R8G8B8A8_UNORM,
        _commandBuffers[ 3U ]
    );

    if ( !result )
        return false;

    constexpr uint8_t const paramData[ 4U ] = { 128U, 128U, 128U, 128U };

    if ( !textureLoader ( _params, paramData, sizeof ( paramData ), VK_FORMAT_R8G8B8A8_UNORM, _commandBuffers[ 4U ] ) )
        return false;

    constexpr uint8_t const transparentData[ 4U ] = { 0U, 0U, 0U, 0U };

    return textureLoader ( _transparent,
        transparentData,
        sizeof ( transparentData ),
        VK_FORMAT_R8G8B8A8_UNORM,
        _commandBuffers[ 5U ]
    );
}

void DefaultTextureManager::Destroy ( android_vulkan::Renderer &renderer ) noexcept
{
    auto const freeJob = [ &renderer ] ( Texture2DRef &texture ) noexcept {
        if ( !texture )
            return;

        texture->FreeResources ( renderer );
        texture = nullptr;
    };

    freeJob ( _params );
    freeJob ( _normal );
    freeJob ( _mask );
    freeJob ( _emission );
    freeJob ( _albedo );
    freeJob ( _transparent );
}

Texture2DRef const& DefaultTextureManager::GetAlbedo () const noexcept
{
    return _albedo;
}

Texture2DRef const& DefaultTextureManager::GetEmission () const noexcept
{
    return _emission;
}

Texture2DRef const& DefaultTextureManager::GetMask () const noexcept
{
    return _mask;
}

Texture2DRef const& DefaultTextureManager::GetNormal () const noexcept
{
    return _normal;
}

Texture2DRef const& DefaultTextureManager::GetParams () const noexcept
{
    return _params;
}

Texture2DRef const& DefaultTextureManager::GetTransparent () const noexcept
{
    return _transparent;
}

} // namespace pbr
