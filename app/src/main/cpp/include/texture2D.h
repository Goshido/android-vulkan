#ifndef TEXTURE_2D_H
#define TEXTURE_2D_H


#include <GXCommon/GXWarning.h>

GX_DISABLE_COMMON_WARNINGS

#include <string>

GX_RESTORE_WARNING_STATE

#include "renderer.h"


namespace android_vulkan {

class Texture2D final
{
    private:
        VkFormat            _format;

        VkImage             _image;
        VkDeviceMemory      _imageDeviceMemory;
        VkImageView         _imageView;

        bool                _isGenerateMipmaps;

        uint8_t             _mipLevels;
        VkExtent2D          _resolution;

        VkBuffer            _transfer;
        VkDeviceMemory      _transferDeviceMemory;

        std::string         _fileName;

    public:
        Texture2D ();

        Texture2D ( const Texture2D &other ) = delete;
        Texture2D& operator = ( const Texture2D &other ) = delete;

        [[nodiscard]] bool CreateRenderTarget ( const VkExtent2D &resolution,
            VkFormat format,
            VkImageUsageFlags usage,
            android_vulkan::Renderer &renderer
        );

        void FreeResources ( android_vulkan::Renderer &renderer );

        // optimization: _transfer and _transferDeviceMemory are needed only for uploading pixel data to the Vulkan
        // texture object. Uploading itself is done via command submit: vkCmdCopyBufferToImage. So you can make a
        // bunch of vkCmdCopyBufferToImage call for different textures and after completion you can free
        // _transfer and _transferDeviceMemory for Texture2D objects.
        void FreeTransferResources ( android_vulkan::Renderer &renderer );

        [[nodiscard]] VkFormat GetFormat () const;
        [[nodiscard]] VkImageView GetImageView () const;
        [[nodiscard]] uint8_t GetMipLevelCount () const;
        [[nodiscard]] const std::string& GetName () const;

        // Method is used when file name and format are passed via constructor.
        [[maybe_unused]] [[nodiscard]] bool UploadData ( android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer
        );

        [[maybe_unused]] [[nodiscard]] bool UploadData ( std::string &fileName,
            VkFormat format,
            bool isGenerateMipmaps,
            android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer
        );

        [[nodiscard]] bool UploadData ( std::string &&fileName,
            VkFormat format,
            bool isGenerateMipmaps,
            android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer
        );

        [[nodiscard]] bool UploadData ( const uint8_t* data,
            size_t size,
            const VkExtent2D &resolution,
            VkFormat format,
            bool isGenerateMipmaps,
            android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer
        );

    private:
        [[nodiscard]] uint32_t CountMipLevels ( const VkExtent2D &resolution ) const;

        [[nodiscard]] bool CreateCommonResources ( VkImageCreateInfo &imageInfo,
            const VkExtent2D &resolution,
            VkFormat format,
            VkImageUsageFlags usage,
            bool isGenerateMipmaps,
            android_vulkan::Renderer &renderer
        );

        void FreeResourceInternal ( android_vulkan::Renderer &renderer );

        [[nodiscard]] bool IsFormatCompatible ( VkFormat target,
            VkFormat candidate,
            android_vulkan::Renderer &renderer
        ) const;

        [[nodiscard]] VkFormat PickupFormat ( int channels ) const;

        [[nodiscard]] bool UploadDataInternal ( const uint8_t* data,
            size_t size,
            bool isGenerateMipmaps,
            const VkImageCreateInfo &imageInfo,
            android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer
        );

        [[nodiscard]] static bool LoadImage ( std::vector<uint8_t> &pixelData,
            const std::string &fileName,
            int &width,
            int &height,
            int &channels
        );
};

} // namespace android_vulkan


#endif // TEXTURE_2D_H
