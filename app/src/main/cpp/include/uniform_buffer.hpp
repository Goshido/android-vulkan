#ifndef ANDROID_VULKAN_UNIFORM_BUFFER_HPP
#define ANDROID_VULKAN_UNIFORM_BUFFER_HPP


#include "renderer.hpp"


namespace android_vulkan {

// This class wraps Vulkan creation and update routines of a uniform buffer.
// The class is supposed to used with single uniform buffer object.
class UniformBuffer final
{
    private:
        size_t                  _size = 0U;

        VkBuffer                _buffer = VK_NULL_HANDLE;
        VkDeviceMemory          _bufferMemory = VK_NULL_HANDLE;
        VkDeviceSize            _bufferOffset = std::numeric_limits<VkDeviceSize>::max ();

        VkCommandBuffer         _commandBuffer = VK_NULL_HANDLE;
        VkCommandPool           _commandPool = VK_NULL_HANDLE;

        VkPipelineStageFlags    _targetStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

        VkBuffer                _transfer = VK_NULL_HANDLE;
        VkDeviceMemory          _transferMemory = VK_NULL_HANDLE;
        VkDeviceSize            _transferOffset = std::numeric_limits<VkDeviceSize>::max ();

    public:
        UniformBuffer () = default;

        UniformBuffer ( UniformBuffer const & ) = delete;
        UniformBuffer &operator = ( UniformBuffer const & ) = delete;

        UniformBuffer ( UniformBuffer && ) = delete;
        UniformBuffer &operator = ( UniformBuffer && ) = delete;

        ~UniformBuffer () = default;

        void FreeResources ( android_vulkan::Renderer &renderer ) noexcept;
        [[nodiscard]] VkBuffer GetBuffer () const noexcept;
        [[nodiscard]] size_t GetSize () const noexcept;

        // Note this method must be invoked before UniformBuffer::Update.
        // This method must be used only once except situation when user invokes UniformBuffer::FreeResources.
        // "targetStages" is a pipeline stage mask when buffer content will be used.
        // The method returns true if success. Otherwise the method returns false.
        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            VkCommandPool commandPool,
            VkPipelineStageFlags targetStages
        ) noexcept;

        // Method updates GPU side uniform buffer.
        // Note the method could be used after UniformBuffer::Init method.
        // The method returns true if success. Otherwise the method returns false.
        [[nodiscard]] bool Update ( android_vulkan::Renderer &renderer,
            VkFence fence,
            uint8_t const* data,
            size_t size
        ) noexcept;

    private:
        [[nodiscard]] bool InitResources (android_vulkan::Renderer &renderer, size_t size ) noexcept;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_UNIFORM_BUFFER_HPP
