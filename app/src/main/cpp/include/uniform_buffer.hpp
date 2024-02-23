#ifndef ANDROID_VULKAN_UNIFORM_BUFFER_HPP
#define ANDROID_VULKAN_UNIFORM_BUFFER_HPP


#include "renderer.hpp"


namespace android_vulkan {

// This class wraps Vulkan creation and update routines of a uniform buffer.
// The class is supposed to used with single uniform buffer object.
class UniformBuffer final
{
    private:
        VkDeviceSize                _size = 0U;
        size_t                      _index = 0U;

        std::vector<VkBuffer>       _buffers {};
        VkDeviceMemory              _memory = VK_NULL_HANDLE;
        VkDeviceSize                _offset = std::numeric_limits<VkDeviceSize>::max ();

    public:
        UniformBuffer () = default;

        UniformBuffer ( UniformBuffer const & ) = delete;
        UniformBuffer &operator = ( UniformBuffer const & ) = delete;

        UniformBuffer ( UniformBuffer && ) = delete;
        UniformBuffer &operator = ( UniformBuffer && ) = delete;

        ~UniformBuffer () = default;

        void FreeResources ( android_vulkan::Renderer &renderer ) noexcept;
        [[nodiscard]] VkDeviceSize GetSize () const noexcept;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer, size_t size, size_t amount ) noexcept;
        [[nodiscard]] VkBuffer Update ( VkCommandBuffer commandBuffer, uint8_t const* data ) noexcept;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_UNIFORM_BUFFER_HPP
