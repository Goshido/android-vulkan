#ifndef TEXTURE_2D_H
#define TEXTURE_2D_H


#include <GXCommon/GXWarning.h>

GX_DISABLE_COMMON_WARNINGS

#include <string>

GX_RESTORE_WARNING_STATE

#include "renderer.h"


namespace android_vulkan {

enum class eFormat : uint8_t
{
    Unorm,
    sRGB
};

class Texture2D final
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

        std::string         _fileName;

    public:
        Texture2D ();

        Texture2D ( Texture2D const & ) = delete;
        Texture2D& operator = ( Texture2D const & ) = delete;

        Texture2D ( Texture2D && ) = delete;
        Texture2D& operator = ( Texture2D && ) = delete;

        ~Texture2D () = default;

        [[nodiscard]] bool CreateRenderTarget ( VkExtent2D const &resolution,
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
        [[nodiscard]] VkImage GetImage () const;
        [[nodiscard]] VkImageView GetImageView () const;
        [[nodiscard]] uint8_t GetMipLevelCount () const;
        [[nodiscard]] std::string const& GetName () const;

        // Supported formats: PNG.
        [[nodiscard]] [[maybe_unused]] bool UploadData ( std::string const &fileName,
            eFormat format,
            bool isGenerateMipmaps,
            android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer
        );

        // Supported media containers:
        // - PNG
        // - KTXv1 (ASTC with mipmaps)
        [[nodiscard]] bool UploadData ( std::string &&fileName,
            eFormat format,
            bool isGenerateMipmaps,
            android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer
        );

        // Supported media containers:
        // - PNG
        // - KTXv1 (ASTC with mipmaps)
        [[nodiscard]] bool UploadData ( std::string_view const &fileName,
            eFormat format,
            bool isGenerateMipmaps,
            android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer
        );

        // Supported media containers:
        // - PNG
        // - KTXv1 (ASTC with mipmaps)
        [[nodiscard]] bool UploadData ( char const* fileName,
            eFormat format,
            bool isGenerateMipmaps,
            android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer
        );

        [[nodiscard]] bool UploadData ( uint8_t const* data,
            size_t size,
            VkExtent2D const &resolution,
            VkFormat format,
            bool isGenerateMipmaps,
            android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer
        );

    private:
        [[nodiscard]] bool CreateCommonResources ( VkImageCreateInfo &imageInfo,
            VkExtent2D const &resolution,
            VkFormat format,
            VkImageUsageFlags usage,
            bool isGenerateMipmaps,
            android_vulkan::Renderer &renderer
        );

        // The method returns true if success. Otherwise the method returns false.
        // Note the method maps "_transferDeviceMemory" to the "mappedBuffer". So user code MUST invoke vkUnmapMemory.
        [[nodiscard]] bool CreateTransferResources ( uint8_t* &mappedBuffer,
            VkDeviceSize size,
            android_vulkan::Renderer &renderer
        );

        void FreeResourceInternal ( android_vulkan::Renderer &renderer );

        [[nodiscard]] bool UploadCompressed ( std::string const &fileName,
            eFormat format,
            android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer
        );

        [[nodiscard]] bool UploadDataInternal ( uint8_t const* data,
            size_t size,
            bool isGenerateMipmaps,
            VkImageCreateInfo const &imageInfo,
            android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer
        );

        [[nodiscard]] static bool IsCompressed ( std::string const &fileName );

        [[nodiscard]] static bool LoadImage ( std::vector<uint8_t> &pixelData,
            std::string const &fileName,
            int &width,
            int &height,
            int &channels
        );

        [[nodiscard]] static uint32_t CountMipLevels ( VkExtent2D const &resolution );
        [[nodiscard]] static VkFormat PickupFormat ( int channels );

        [[nodiscard]] static VkFormat ResolveFormat ( VkFormat baseFormat,
            eFormat format,
            android_vulkan::Renderer &renderer
        );
};

} // namespace android_vulkan


#endif // TEXTURE_2D_H
