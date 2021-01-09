#ifndef ANDROID_VULKAN_UNIFORM_BUFFER_H
#define ANDROID_VULKAN_UNIFORM_BUFFER_H


#include "renderer.h"


namespace android_vulkan {

// This class wraps Vulkan creation and update routines of a uniform buffer.
// The class is supposed to used with single uniform buffer object.
class UniformBuffer final
{
    private:
        size_t                  _size;

        VkBuffer                _buffer;
        VkDeviceMemory          _bufferMemory;

        VkCommandBuffer         _commandBuffer;
        VkCommandPool           _commandPool;

        VkPipelineStageFlags    _targetStages;

        VkBuffer                _transfer;
        VkDeviceMemory          _transferMemory;

    public:
        UniformBuffer () noexcept;

        UniformBuffer ( UniformBuffer const & ) = delete;
        UniformBuffer& operator = ( UniformBuffer const & ) = delete;

        UniformBuffer ( UniformBuffer && ) = delete;
        UniformBuffer& operator = ( UniformBuffer && ) = delete;

        ~UniformBuffer () = default;

        void FreeResources ( VkDevice device );
        [[nodiscard]] VkBuffer GetBuffer () const;
        [[nodiscard]] size_t GetSize () const;

        // Note this method must be invoked before UniformBuffer::Update.
        // This method must be used only once except situation when user invokes UniformBuffer::FreeResources.
        // "targetStages" is a pipeline stage mask when buffer content will be used.
        // The method returns true if success. Otherwise the method returns false.
        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            VkCommandPool commandPool,
            VkPipelineStageFlags targetStages
        );

        // Method updates GPU side uniform buffer.
        // Note the method could be used after UniformBuffer::Init method.
        // The method returns true if success. Otherwise the method returns false.
        [[nodiscard]] bool Update ( android_vulkan::Renderer &renderer, uint8_t const* data, size_t size );

    private:
        [[nodiscard]] bool InitResources (android_vulkan::Renderer &renderer, size_t size );
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_UNIFORM_BUFFER_H
