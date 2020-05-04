#ifndef ROTATING_MESH_TEXTURE_2D_H
#define ROTATING_MESH_TEXTURE_2D_H


#include "warning.h"

AV_DISABLE_COMMON_WARNINGS

#include <string>
#include <vector>

AV_RESTORE_WARNING_STATE

#include "renderer.h"


namespace rotating_mesh {

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
        Texture2D ( std::string &fileName, VkFormat format );
        Texture2D ( std::string &&fileName, VkFormat format );

        Texture2D ( const Texture2D &other ) = delete;
        Texture2D& operator = ( const Texture2D &other ) = delete;

        void FreeResources ( android_vulkan::Renderer &renderer );

        // optimization: _transfer and _transferDeviceMemory are needed only for uploading pixel data to the Vulkan
        // texture object. Uploading itself is done via command submit: vkCmdCopyBufferToImage. So you can make a
        // bunch of vkCmdCopyBufferToImage call for different textures and after completion you can free
        // _transfer and _transferDeviceMemory for Texture2D objects.
        void FreeTransferResources ( android_vulkan::Renderer &renderer );

        uint8_t GetMipLevelCount () const;

        // Method is used when file name and format are passed via constructor.
        bool UploadData ( android_vulkan::Renderer &renderer, VkCommandBuffer commandBuffer );

        bool UploadData ( std::string &fileName,
            VkFormat format,
            android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer
        );

        bool UploadData ( std::string &&fileName,
            VkFormat format,
            android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer
        );

        bool UploadData ( const std::vector<uint8_t> &data,
            const VkExtent2D &resolution,
            VkFormat format,
            android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer
        );

    private:
        uint32_t CountMipLevels ( const VkExtent2D &resolution ) const;
        void FreeResourceInternal ( android_vulkan::Renderer &renderer );
        bool IsFormatCompatible ( VkFormat target, VkFormat candidate, android_vulkan::Renderer &renderer ) const;

        bool LoadImage ( std::vector<uint8_t> &pixelData,
            const std::string &fileName,
            int &width,
            int &height,
            int &channels
        );

        VkFormat PickupFormat ( int channels ) const;

        bool UploadDataInternal ( const std::vector<uint8_t> &data,
            const VkExtent2D &resolution,
            VkFormat format,
            android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer
        );
};

} // namespace rotating_mesh


#endif // ROTATING_MESH_TEXTURE_2D_H
