#ifndef PBR_UNIFORM_BUFFER_POOL_HPP
#define PBR_UNIFORM_BUFFER_POOL_HPP


#include <renderer.hpp>


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
        std::vector<VkBuffer>       _buffers {};
        VkDeviceMemory              _memory = VK_NULL_HANDLE;
        size_t                      _index = 0U;
        size_t                      _itemSize = 0U;
        VkDeviceSize                _offset = std::numeric_limits<VkDeviceSize>::max ();
        size_t                      _size;

    public:
        UniformBufferPool () = delete;

        UniformBufferPool ( UniformBufferPool const & ) = delete;
        UniformBufferPool &operator = ( UniformBufferPool const & ) = delete;

        UniformBufferPool ( UniformBufferPool && ) = delete;
        UniformBufferPool &operator = ( UniformBufferPool && ) = delete;

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
            VkMemoryRequirements &requirements,
            size_t itemCount,
            VkBufferCreateInfo const &bufferInfo
        ) noexcept;

        [[nodiscard]] static bool ResolveMemoryRequirements ( android_vulkan::Renderer &renderer,
            VkMemoryRequirements &requirements,
            VkBufferCreateInfo const &bufferInfo
        ) noexcept;
};

} // namespace pbr


#endif // PBR_UNIFORM_BUFFER_POOL_HPP
