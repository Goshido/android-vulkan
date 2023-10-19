#ifndef ANDROID_VULKAN_SKIN_DATA_HPP
#define ANDROID_VULKAN_SKIN_DATA_HPP


#include "buffer_info.hpp"
#include "renderer.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <string>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

class SkinData final
{
    private:
        struct Buffer final
        {
            VkBuffer            _buffer = VK_NULL_HANDLE;
            VkDeviceMemory      _memory = VK_NULL_HANDLE;
            VkDeviceSize        _offset = std::numeric_limits<VkDeviceSize>::max ();
        };

    private:
        GXAABB                  _bounds {};
        std::string             _fileName {};
        Buffer                  _skin {};
        Buffer                  _transfer {};
        VkDeviceSize            _minPoseRange = 0U;

    public:
        SkinData () = default;

        SkinData ( SkinData const & ) = delete;
        SkinData &operator = ( SkinData const & ) = delete;

        SkinData ( SkinData && ) = default;
        SkinData &operator = ( SkinData && ) = default;

        ~SkinData () = default;

        void FreeResources ( Renderer &renderer ) noexcept;
        void FreeTransferResources ( Renderer &renderer ) noexcept;

        [[nodiscard]] GXAABB const &GetBounds () const noexcept;
        [[nodiscard]] BufferInfo GetSkinInfo () const noexcept;
        [[nodiscard]] VkDeviceSize GetMinPoseRange () const noexcept;
        [[nodiscard]] std::string const &GetName () const noexcept;

        [[nodiscard]] bool LoadSkin ( std::string &&skinFilename,
            std::string &&skeletonFilename,
            Renderer &renderer,
            VkCommandBuffer commandBuffer,
            VkFence fence
        ) noexcept;

        [[nodiscard]] constexpr static uint32_t MaxCommandBufferPerSkin () noexcept
        {
            return 1U;
        }

    private:
        void FreeResourceInternal ( Renderer &renderer ) noexcept;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_SKIN_DATA_HPP
