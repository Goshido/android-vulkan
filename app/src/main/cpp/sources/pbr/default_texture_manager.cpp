#include <precompiled_headers.hpp>
#include <half_types.hpp>
#include <pbr/default_texture_manager.hpp>
#include <vulkan_utils.hpp>


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

    for ( auto &c : _commandBuffers )
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

    if ( !result ) [[unlikely]]
        return false;

    constexpr size_t albedoIdx = 0U;
    constexpr size_t emissionIdx = 1U;
    constexpr size_t maskIdx = 2U;
    constexpr size_t normalIdx = 3U;
    constexpr size_t paramIdx = 4U;
    constexpr size_t transparentIdx = 5U;

#if defined ( AV_ENABLE_VVL ) || defined ( AV_ENABLE_RENDERDOC )

    VkDevice device = renderer.GetDevice ();

    AV_SET_VULKAN_OBJECT_NAME ( device,
        _commandBuffers[ albedoIdx ],
        VK_OBJECT_TYPE_COMMAND_BUFFER,
        "Default albedo image"
    )

    AV_SET_VULKAN_OBJECT_NAME ( device,
        _commandBuffers[ emissionIdx ],
        VK_OBJECT_TYPE_COMMAND_BUFFER,
        "Default emission image"
    )

    AV_SET_VULKAN_OBJECT_NAME ( device,
        _commandBuffers[ maskIdx ],
        VK_OBJECT_TYPE_COMMAND_BUFFER,
        "Default mask image"
    )

    AV_SET_VULKAN_OBJECT_NAME ( device,
        _commandBuffers[ normalIdx ],
        VK_OBJECT_TYPE_COMMAND_BUFFER,
        "Default normal image"
    )

    AV_SET_VULKAN_OBJECT_NAME ( device,
        _commandBuffers[ paramIdx ],
        VK_OBJECT_TYPE_COMMAND_BUFFER,
        "Default params image"
    )

    AV_SET_VULKAN_OBJECT_NAME ( device,
        _commandBuffers[ transparentIdx ],
        VK_OBJECT_TYPE_COMMAND_BUFFER,
        "Default transparent image"
    )

#endif // AV_ENABLE_VVL || AV_ENABLE_RENDERDOC

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

        if ( result ) [[likely]]
            return true;

        texture = nullptr;
        return false;
    };

    constexpr uint8_t const albedoData[ 4U ] = { 255U, 255U, 255U, 255U };

    result = textureLoader ( _albedo,
        albedoData,
        sizeof ( albedoData ),
        VK_FORMAT_R8G8B8A8_SRGB,
        _commandBuffers[ albedoIdx ]
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _albedo->GetImage (), VK_OBJECT_TYPE_IMAGE, "Default albedo" )
    AV_SET_VULKAN_OBJECT_NAME ( device, _albedo->GetImageView (), VK_OBJECT_TYPE_IMAGE_VIEW, "Default albedo" )

    android_vulkan::Half4 const emissionData ( 0.0F, 0.0F, 0.0F, 0.0F );

    result = textureLoader ( _emission,
        reinterpret_cast<const uint8_t*> ( emissionData._data ),
        sizeof ( emissionData ),
        VK_FORMAT_R16G16B16A16_SFLOAT,
        _commandBuffers[ emissionIdx ]
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _emission->GetImage (), VK_OBJECT_TYPE_IMAGE, "Default emission" )
    AV_SET_VULKAN_OBJECT_NAME ( device, _emission->GetImageView (), VK_OBJECT_TYPE_IMAGE_VIEW, "Default emission" )

    constexpr uint8_t const maskData[ 4U ] = { 255U, 0U, 0U, 0U };

    if ( !textureLoader ( _mask, maskData, sizeof ( maskData ), VK_FORMAT_R8G8B8A8_UNORM, _commandBuffers[ maskIdx ] ) )
    {
        [[unlikely]]
        return false;
    }

    AV_SET_VULKAN_OBJECT_NAME ( device, _mask->GetImage (), VK_OBJECT_TYPE_IMAGE, "Default mask" )
    AV_SET_VULKAN_OBJECT_NAME ( device, _mask->GetImageView (), VK_OBJECT_TYPE_IMAGE_VIEW, "Default mask" )

    constexpr uint8_t const normalData[ 4U ] = { 128U, 0U, 0U, 128U };

    result = textureLoader ( _normal,
        normalData,
        sizeof ( normalData ),
        VK_FORMAT_R8G8B8A8_UNORM,
        _commandBuffers[ normalIdx ]
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _normal->GetImage (), VK_OBJECT_TYPE_IMAGE, "Default normal" )
    AV_SET_VULKAN_OBJECT_NAME ( device, _normal->GetImageView (), VK_OBJECT_TYPE_IMAGE_VIEW, "Default normal" )

    constexpr uint8_t const paramData[ 4U ] = { 128U, 128U, 128U, 128U };

    result = textureLoader ( _params,
        paramData,
        sizeof ( paramData ),
        VK_FORMAT_R8G8B8A8_UNORM,
        _commandBuffers[ paramIdx ]
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _params->GetImage (), VK_OBJECT_TYPE_IMAGE, "Default params" )
    AV_SET_VULKAN_OBJECT_NAME ( device, _params->GetImageView (), VK_OBJECT_TYPE_IMAGE_VIEW, "Default params" )

    constexpr uint8_t const transparentData[ 4U ] = { 0U, 0U, 0U, 0U };

    return textureLoader ( _transparent,
        transparentData,
        sizeof ( transparentData ),
        VK_FORMAT_R8G8B8A8_UNORM,
        _commandBuffers[ transparentIdx ]
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

Texture2DRef const &DefaultTextureManager::GetAlbedo () const noexcept
{
    return _albedo;
}

Texture2DRef const &DefaultTextureManager::GetEmission () const noexcept
{
    return _emission;
}

Texture2DRef const &DefaultTextureManager::GetMask () const noexcept
{
    return _mask;
}

Texture2DRef const &DefaultTextureManager::GetNormal () const noexcept
{
    return _normal;
}

Texture2DRef const &DefaultTextureManager::GetParams () const noexcept
{
    return _params;
}

Texture2DRef const &DefaultTextureManager::GetTransparent () const noexcept
{
    return _transparent;
}

} // namespace pbr
