#ifndef TEXTURE_CUBE_H
#define TEXTURE_CUBE_H


#include "renderer.h"


namespace android_vulkan {

class TextureCube final
{
    private:
        VkFormat            _format;

        VkImage             _image;
        VkDeviceMemory      _imageDeviceMemory;
        VkImageView         _imageView;

        uint8_t             _mipLevels;
        VkExtent2D          _resolution;

    public:
        TextureCube () noexcept;

        TextureCube ( TextureCube const & ) = delete;
        TextureCube& operator = ( TextureCube const & ) = delete;

        TextureCube ( TextureCube && ) = delete;
        TextureCube& operator = ( TextureCube && ) = delete;

        ~TextureCube () = default;

        [[nodiscard]] bool CreateRenderTarget ( VkExtent2D const &resolution,
            VkFormat format,
            VkImageUsageFlags usage,
            Renderer &renderer
        );

        void FreeResources ( VkDevice device );

        [[maybe_unused]] [[nodiscard]] VkFormat GetFormat () const;
        [[maybe_unused]] [[nodiscard]] VkImage GetImage () const;
        [[nodiscard]] VkImageView GetImageView () const;
        [[maybe_unused]] [[nodiscard]] uint8_t GetMipLevelCount () const;
        [[maybe_unused]] [[nodiscard]] VkExtent2D const& GetResolution () const;
};

} // namespace android_vulkan


#endif // TEXTURE_CUBE_H
