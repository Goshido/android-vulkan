#ifndef PBR_UNIFORM_BUFFER_POOL_H
#define PBR_UNIFORM_BUFFER_POOL_H


#include <renderer.h>


namespace pbr {

enum class eUniformPoolSize : size_t
{
    Tiny_4M [[maybe_unused]] = 4U,
    Small_8M [[maybe_unused]] = 8U,
    Medium_16M [[maybe_unused]] = 16U,
    Big_32M [[maybe_unused]] = 32U,
    Huge_64M = 64U
};

// Note the method is NOT thread safe.
class UniformBufferPool final
{
    private:
        VkDeviceMemory              _gpuMemory;
        size_t                      _index;
        VkDeviceSize                _itemSize;
        std::vector<VkBuffer>       _pool;
        size_t                      _size;

    public:
        UniformBufferPool () = delete;

        UniformBufferPool ( UniformBufferPool const & ) = delete;
        UniformBufferPool& operator = ( UniformBufferPool const & ) = delete;

        UniformBufferPool ( UniformBufferPool && ) = delete;
        UniformBufferPool& operator = ( UniformBufferPool && ) = delete;

        explicit UniformBufferPool ( eUniformPoolSize size ) noexcept;
        ~UniformBufferPool () = default;

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
