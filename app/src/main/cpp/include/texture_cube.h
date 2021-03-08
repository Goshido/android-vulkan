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
        VkFormat            _format;

        VkImage             _image;
        VkDeviceMemory      _imageDeviceMemory;
        VkImageView         _imageView;

        uint8_t             _mipLevels;
        VkExtent2D          _resolution;

        VkBuffer            _transfer;
        VkDeviceMemory      _transferDeviceMemory;

    public:
        TextureCube () noexcept;

        TextureCube ( TextureCube const & ) = delete;
        TextureCube& operator = ( TextureCube const & ) = delete;

        TextureCube ( TextureCube && ) = delete;
        TextureCube& operator = ( TextureCube && ) = delete;

        ~TextureCube () = default;

        [[nodiscard]] bool CreateRenderTarget ( Renderer &renderer,
            VkExtent2D const &resolution,
            VkFormat format,
            VkImageUsageFlags usage
        );

        // Supported media containers: KTXv1 (ASTC with mipmaps).
        [[nodiscard]] bool UploadData ( Renderer &renderer,
            TextureCubeData const &data,
            VkCommandBuffer commandBuffer
        );

        void FreeResources ( VkDevice device );

        // optimization: _transfer and _transferDeviceMemory are needed only for uploading pixel data to the Vulkan
        // texture object. Uploading itself is done via command submit: vkCmdCopyBufferToImage. So you can make a
        // bunch of vkCmdCopyBufferToImage call for different textures and after completion you can free
        // _transfer and _transferDeviceMemory for Texture2D objects.
        void FreeTransferResources ( VkDevice device );

        [[nodiscard]] VkFormat GetFormat () const;
        [[nodiscard]] VkImage GetImage () const;
        [[nodiscard]] VkImageView GetImageView () const;
        [[maybe_unused, nodiscard]] uint8_t GetMipLevelCount () const;
        [[maybe_unused, nodiscard]] VkExtent2D const& GetResolution () const;

    private:
        [[maybe_unused, nodiscard]] bool CreateImageResources ( Renderer &renderer,
            VkExtent2D const &resolution,
            VkFormat format,
            VkImageUsageFlags usage,
            uint32_t mipLevels
        );
};

} // namespace android_vulkan


#endif // TEXTURE_CUBE_H
