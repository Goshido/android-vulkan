#ifndef PBR_UNIFORM_BUFFER_POOL_H
#define PBR_UNIFORM_BUFFER_POOL_H


#include <renderer.h>


namespace pbr {

enum class eUniformPoolSize : size_t
{
    Nanoscopic_64KB = 64U,
    Microscopic_1M [[maybe_unused]] = 1024U,
    Tiny_4M = 4096U,
    Small_8M [[maybe_unused]] = 8192U,
    Medium_16M [[maybe_unused]] = 16384U,
    Big_32M [[maybe_unused]] = 32768U,
    Huge_64M = 65536U
};

// Note the class is NOT thread safe.
class UniformBufferPool final
{
    private:
        struct Item final
        {
            VkBuffer                _buffer = VK_NULL_HANDLE;
            VkDeviceMemory          _memory = VK_NULL_HANDLE;
            VkDeviceSize            _offset = std::numeric_limits<VkDeviceSize>::max ();
        };

    private:
        size_t                      _index = 0U;
        size_t                      _itemSize = 0U;
        std::vector<Item>           _pool {};
        size_t                      _size;

    public:
        UniformBufferPool () = delete;

        UniformBufferPool ( UniformBufferPool const & ) = delete;
        UniformBufferPool& operator = ( UniformBufferPool const & ) = delete;

        UniformBufferPool ( UniformBufferPool && ) = delete;
        UniformBufferPool& operator = ( UniformBufferPool && ) = delete;

        explicit UniformBufferPool ( eUniformPoolSize size ) noexcept;
        ~UniformBufferPool () = default;

        // The method acquires one uniform buffer from the pool and fills it with data.
        // Method return buffer which has been just written.
        VkBuffer Push ( VkCommandBuffer commandBuffer, void const* data, size_t size ) noexcept;

        [[nodiscard]] size_t GetAvailableItemCount () const noexcept;
        [[nodiscard]] VkBuffer GetBuffer ( size_t bufferIndex ) const noexcept;

        // The method return all items to the pool.
        void Reset () noexcept;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer, size_t itemSize ) noexcept;
        void Destroy ( android_vulkan::Renderer &renderer ) noexcept;

    private:
        [[nodiscard]] bool AllocateBuffers ( android_vulkan::Renderer &renderer,
            size_t itemCount,
            size_t itemSize
        ) noexcept;

        [[nodiscard]] static bool ResolveAlignment ( android_vulkan::Renderer &renderer,
            size_t &alignment,
            size_t itemSize
        ) noexcept;
};

} // namespace pbr


#endif // PBR_UNIFORM_BUFFER_POOL_H
