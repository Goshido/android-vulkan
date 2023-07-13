#ifndef PBR_UNIFORM_BUFFER_POOL_MANAGER_HPP
#define PBR_UNIFORM_BUFFER_POOL_MANAGER_HPP


#include "descriptor_set_layout.hpp"
#include "uniform_buffer_pool.hpp"


namespace pbr {

class UniformBufferPoolManager final
{
    private:
        std::vector<VkBufferMemoryBarrier>      _barriers {};
        std::vector<VkDescriptorBufferInfo>     _bufferInfo {};
        VkDescriptorPool                        _descriptorPool = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet>            _descriptorSets {};
        VkPipelineStageFlags const              _syncFlags;

        size_t                                  _uniformBaseIndex = 0U;
        size_t                                  _uniformReadIndex = 0U;
        size_t                                  _uniformWriteIndex = 0U;
        size_t                                  _uniformWritten = 0U;

        UniformBufferPool                       _uniformPool;
        std::vector<VkWriteDescriptorSet>       _writeSets {};

    public:
        UniformBufferPoolManager () = delete;

        UniformBufferPoolManager ( UniformBufferPoolManager const & ) = delete;
        UniformBufferPoolManager &operator = ( UniformBufferPoolManager const & ) = delete;

        UniformBufferPoolManager ( UniformBufferPoolManager && ) = delete;
        UniformBufferPoolManager &operator = ( UniformBufferPoolManager && ) = delete;

        explicit UniformBufferPoolManager ( eUniformPoolSize size, VkPipelineStageFlags syncFlags ) noexcept;

        ~UniformBufferPoolManager () = default;

        [[nodiscard]] VkDescriptorSet Acquire () noexcept;
        void Commit () noexcept;
        void IssueSync ( VkDevice device, VkCommandBuffer commandBuffer ) const noexcept;
        void Push ( VkCommandBuffer commandBuffer, void const* item, size_t size ) noexcept;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            DescriptorSetLayout const &descriptorSetLayout,
            size_t itemSize,
            uint32_t bind,
            char const* name
        ) noexcept;

        void Destroy ( android_vulkan::Renderer &renderer, char const* name ) noexcept;
};

} // namespace pbr


#endif // PBR_UNIFORM_BUFFER_POOL_MANAGER_HPP
