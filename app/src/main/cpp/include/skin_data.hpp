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
        GXAABB              _bounds {};
        VkBuffer            _buffer = VK_NULL_HANDLE;
        VkDeviceSize        _bufferSize = 0U;
        std::string         _fileName {};
        VkDeviceMemory      _memory = VK_NULL_HANDLE;
        VkDeviceSize        _minPoseRange = 0U;
        VkDeviceSize        _offset = std::numeric_limits<VkDeviceSize>::max ();

    public:
        SkinData () = default;

        SkinData ( SkinData const & ) = delete;
        SkinData &operator = ( SkinData const & ) = delete;

        SkinData ( SkinData && ) = default;
        SkinData &operator = ( SkinData && ) = default;

        ~SkinData () = default;

        void FreeResources ( Renderer &renderer ) noexcept;

        [[nodiscard]] GXAABB const &GetBounds () const noexcept;
        [[nodiscard]] BufferInfo GetSkinInfo () const noexcept;
        [[nodiscard]] VkDeviceSize GetMinPoseRange () const noexcept;
        [[nodiscard]] std::string const &GetName () const noexcept;

        [[nodiscard]] bool LoadSkin ( std::string &&skinFilename,
            std::string &&skeletonFilename,
            Renderer &renderer
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
