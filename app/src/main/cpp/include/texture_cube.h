#ifndef TEXTURE_CUBE_H
#define TEXTURE_CUBE_H


#include "renderer.h"


namespace android_vulkan {

struct TextureCubeData final
{
    char const*     _xPlusFile;
    char const*     _xMinusFile;

    char const*     _yPlusFile;
    char const*     _yMinusFile;

    char const*     _zPlusFile;
    char const*     _zMinusFile;
};

class TextureCube final
{
    private:
        VkFormat            _format = VK_FORMAT_UNDEFINED;

        VkImage             _image = VK_NULL_HANDLE;
        VkDeviceMemory      _imageMemory = VK_NULL_HANDLE;
        VkDeviceSize        _imageOffset = std::numeric_limits<VkDeviceSize>::max ();
        VkImageView         _imageView = VK_NULL_HANDLE;

        uint8_t             _mipLevels = 0U;
        VkExtent2D          _resolution { .width = 0U, .height = 0U };

        VkBuffer            _transfer = VK_NULL_HANDLE;
        VkDeviceMemory      _transferMemory = VK_NULL_HANDLE;
        VkDeviceSize        _transferOffset = std::numeric_limits<VkDeviceSize>::max ();

    public:
        TextureCube () = default;

        TextureCube ( TextureCube const & ) = delete;
        TextureCube &operator = ( TextureCube const & ) = delete;

        TextureCube ( TextureCube && ) = delete;
        TextureCube &operator = ( TextureCube && ) = delete;

        ~TextureCube () = default;

        [[nodiscard]] bool CreateRenderTarget ( Renderer &renderer,
            VkExtent2D const &resolution,
            VkFormat format,
            VkImageUsageFlags usage
        ) noexcept;

        // Supported media containers: KTXv1 (ASTC with mipmaps).
        [[nodiscard]] bool UploadData ( Renderer &renderer,
            TextureCubeData const &data,
            VkCommandBuffer commandBuffer
        ) noexcept;

        void FreeResources ( Renderer &renderer ) noexcept;

        // optimization: _transfer and _transferDeviceMemory are needed only for uploading pixel data to the Vulkan
        // texture object. Uploading itself is done via command submit: vkCmdCopyBufferToImage. So you can make a
        // bunch of vkCmdCopyBufferToImage call for different textures and after completion you can free
        // _transfer and _transferDeviceMemory for Texture2D objects.
        void FreeTransferResources ( Renderer &renderer ) noexcept;

        [[maybe_unused, nodiscard]] VkFormat GetFormat () const noexcept;
        [[maybe_unused, nodiscard]] VkImage GetImage () const noexcept;
        [[nodiscard]] VkImageView GetImageView () const noexcept;
        [[maybe_unused, nodiscard]] uint8_t GetMipLevelCount () const noexcept;
        [[maybe_unused, nodiscard]] VkExtent2D const &GetResolution () const noexcept;

        [[nodiscard]] constexpr static uint32_t GetLayerCount () noexcept
        {
            // Vulkan feature: Cubemaps are treated as layered textures with 6 layers.
            return 6U;
        }

    private:
        [[nodiscard]] bool CreateImageResources ( Renderer &renderer,
            VkExtent2D const &resolution,
            VkFormat format,
            VkImageUsageFlags usage,
            uint32_t mipLevels
        ) noexcept;
};

} // namespace android_vulkan


#endif // TEXTURE_CUBE_H
