#ifndef PBR_UNIFORM_BUFFER_POOL_H
#define PBR_UNIFORM_BUFFER_POOL_H


#include <renderer.h>


namespace pbr {

// Note the method is NOT thread safe.
class UniformBufferPool final
{
    private:
        VkDeviceMemory              _gpuMemory;
        size_t                      _index;
        VkDeviceSize                _itemSize;
        std::vector<VkBuffer>       _pool;

    public:
        UniformBufferPool ();

        UniformBufferPool ( UniformBufferPool const &other ) = delete;
        UniformBufferPool& operator = ( UniformBufferPool const &other ) = delete;

        UniformBufferPool ( UniformBufferPool &&other ) = delete;
        UniformBufferPool& operator = ( UniformBufferPool &&other ) = delete;

        // The method acquires one uniform buffer from the pool, inits it with data and returns the buffer to the user.
        [[nodiscard]] VkBuffer Acquire ( VkCommandBuffer commandBuffer,
            void const* data,
            VkPipelineStageFlags targetStages,
            android_vulkan::Renderer &renderer
        );

        [[nodiscard]] size_t GetItemCount () const;

        // The method return all items to the pool.
        void Reset ();

        [[nodiscard]] bool Init ( size_t itemSize, android_vulkan::Renderer &renderer );
        void Destroy ( android_vulkan::Renderer &renderer );

    private:
        [[nodiscard]] bool AllocateItem ( android_vulkan::Renderer &renderer );
};

} // namespace pbr


#endif // PBR_UNIFORM_BUFFER_POOL_H
