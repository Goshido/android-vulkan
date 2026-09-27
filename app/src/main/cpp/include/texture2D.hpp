#ifndef TEXTURE_2D_HPP
#define TEXTURE_2D_HPP


#include "color_space.hpp"
#include "ktx_media_container.hpp"
#include "renderer.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <string>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

class Texture2D final
{
    private:
        VkFormat                                _format = VK_FORMAT_UNDEFINED;

        VkImage                                 _image = VK_NULL_HANDLE;
        VkDeviceMemory                          _imageDeviceMemory = VK_NULL_HANDLE;
        VkDeviceSize                            _imageMemoryOffset = 0U;
        VkImageView                             _imageView = VK_NULL_HANDLE;

        VkExtent2D                              _resolution
        {
            .width = 0U,
            .height = 0U
        };

        VkBuffer                                _transfer = VK_NULL_HANDLE;
        VkDeviceMemory                          _transferDeviceMemory = VK_NULL_HANDLE;
        VkDeviceSize                            _transferMemoryOffset = 0U;

        std::unique_ptr<KTXMediaContainer>      _ktx {};
        std::string                             _fileName {};
        uint8_t                                 _mipLevels = 0U;
        bool                                    _isGenerateMipmaps = false;

    public:
        Texture2D () = default;

        Texture2D ( Texture2D const & ) = delete;
        Texture2D &operator = ( Texture2D const & ) = delete;

        Texture2D ( Texture2D &&other ) noexcept;
        Texture2D &operator = ( Texture2D &&other ) noexcept;

        ~Texture2D () = default;

        // The feature is used by rendering system to group draw calls by materials.
        void AssignName ( std::string &&name ) noexcept;

        [[nodiscard]] bool CreateRenderTarget ( VkExtent2D const &resolution,
            VkFormat format,
            VkImageUsageFlags usage,
            Renderer &renderer
        ) noexcept;

        void FreeResources ( Renderer &renderer ) noexcept;

        // optimization: _transfer and _transferDeviceMemory are needed only for uploading pixel data to the Vulkan
        // texture object. Uploading itself is done via command submit: vkCmdCopyBufferToImage. So you can make a
        // bunch of vkCmdCopyBufferToImage call for different textures and after completion you can free
        // _transfer and _transferDeviceMemory for Texture2D objects.
        void FreeTransferResources ( Renderer &renderer ) noexcept;

        [[nodiscard]] VkFormat GetFormat () const noexcept;
        [[maybe_unused, nodiscard]] VkImage const &GetImage () const noexcept;
        [[nodiscard]] VkImageView const &GetImageView () const noexcept;
        [[nodiscard]] uint8_t GetMipLevelCount () const noexcept;
        [[nodiscard]] std::string const &GetName () const noexcept;
        [[nodiscard]] VkExtent2D const &GetResolution () const noexcept;
        [[nodiscard]] bool IsInit () const noexcept;

        // Supported media containers: PNG, TGA, KTXv1 (ASTC with mipmaps)
        [[nodiscard]] bool UploadToStagingBuffer ( Renderer &renderer,
            std::string const &fileName,
            eColorSpace space,
            bool isGenerateMipmaps
        ) noexcept;

        // Supported media containers: PNG, TGA, KTXv1 (ASTC with mipmaps)
        [[nodiscard]] bool UploadToStagingBuffer ( Renderer &renderer,
            std::string &&fileName,
            eColorSpace space,
            bool isGenerateMipmaps
        ) noexcept;

        // Supported media containers: PNG, TGA, KTXv1 (ASTC with mipmaps)
        [[nodiscard]] bool UploadToStagingBuffer ( Renderer &renderer,
            std::string_view fileName,
            eColorSpace space,
            bool isGenerateMipmaps
        ) noexcept;

        [[nodiscard]] bool UploadToStagingBuffer ( Renderer &renderer,
            uint8_t const* data,
            size_t size,
            VkExtent2D const &resolution,
            VkFormat format,
            VkImageUsageFlags usage,
            bool isGenerateMipmaps
        ) noexcept;

        // The method invocation must be externally synchronized because it's could call vkQueueSubmit.
        [[nodiscard]] bool UploadToGPU ( Renderer &renderer,
            VkCommandBuffer commandBuffer,
            VkAccessFlagBits access,
            VkImageLayout layout,
            VkPipelineStageFlagBits stages,
            bool externalCommandBuffer,
            VkFence fence
        ) noexcept;

        [[nodiscard]] static uint8_t CountMipLevels ( VkExtent2D const &resolution ) noexcept;

    private:
        [[nodiscard]] bool CreateCommonResources ( Renderer &renderer,
            VkImageCreateInfo &imageInfo,
            VkExtent2D const &resolution,
            VkFormat format,
            VkImageUsageFlags usage,
            uint8_t mips
        ) noexcept;

        [[nodiscard]] bool CreateTransferResources ( Renderer &renderer,
            uint8_t* &mappedBuffer,
            VkDeviceSize size
        ) noexcept;

        void FreeResourceInternal ( Renderer &renderer ) noexcept;

        [[nodiscard]] bool UploadCompressedToStagingBuffer ( Renderer &renderer, std::string const &fileName ) noexcept;

        [[nodiscard]] bool UploadDataUncompressedToStagingBuffer ( Renderer &renderer,
            uint8_t const* data,
            size_t size,
            bool isGenerateMipmaps,
            VkImageCreateInfo const &imageInfo
        ) noexcept;

        [[nodiscard]] bool UploadCompressedToGPU ( VkCommandBuffer commandBuffer,
            VkAccessFlagBits access,
            VkImageLayout layout,
            VkPipelineStageFlagBits stages
        ) noexcept;

        [[nodiscard]] bool UploadUncompressedToGPU ( VkCommandBuffer commandBuffer,
            VkAccessFlagBits access,
            VkImageLayout layout,
            VkPipelineStageFlagBits stages
        ) noexcept;

        [[nodiscard]] static bool IsCompressed ( std::string const &fileName ) noexcept;

        [[nodiscard]] static bool LoadImageUncompressed ( std::vector<uint8_t> &pixelData,
            std::string const &fileName,
            int &width,
            int &height,
            int &channels
        ) noexcept;

        [[nodiscard]] static VkFormat PickupFormat ( int channels ) noexcept;
        [[nodiscard]] static VkFormat ResolveFormat ( VkFormat baseFormat, eColorSpace space ) noexcept;

        [[nodiscard]] static VkImageUsageFlags ResolveUsage ( VkImageUsageFlags usage,
            bool isGenerateMipmaps
        ) noexcept;
};

} // namespace android_vulkan


#endif // TEXTURE_2D_HPP
