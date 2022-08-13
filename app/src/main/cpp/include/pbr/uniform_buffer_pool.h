#ifndef PBR_UNIFORM_BUFFER_POOL_H
#define PBR_UNIFORM_BUFFER_POOL_H


#include <renderer.h>


namespace pbr {

enum class eUniformPoolSize : size_t
{
    Tiny_4M = 4U,
    Small_8M [[maybe_unused]] = 8U,
    Medium_16M [[maybe_unused]] = 16U,
    Big_32M [[maybe_unused]] = 32U,
    Huge_64M = 64U
};

// Note the method is NOT thread safe.
class UniformBufferPool final
{
    private:
        VkBufferMemoryBarrier       _barrier;
        VkBufferCreateInfo          _bufferInfo;
        VkDeviceMemory              _gpuMemory;
        VkDeviceSize                _gpuSpecificItemOffset;
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
        [[nodiscard]] VkBuffer Acquire ( android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer,
            void const* data,
            VkPipelineStageFlags targetStages
        ) noexcept;

        [[nodiscard]] size_t GetAvailableItemCount () const noexcept;
        [[nodiscard]] size_t GetItemCount () const noexcept;

        // The method return all items to the pool.
        void Reset () noexcept;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer, size_t itemSize ) noexcept;
        void Destroy ( VkDevice device ) noexcept;

    private:
        [[nodiscard]] bool AllocateItem ( android_vulkan::Renderer &renderer ) noexcept;

        [[nodiscard]] static bool ResolveAlignment ( android_vulkan::Renderer &renderer,
            size_t &alignment,
            size_t itemSize
        ) noexcept;
};

} // namespace pbr


#endif // PBR_UNIFORM_BUFFER_POOL_H
