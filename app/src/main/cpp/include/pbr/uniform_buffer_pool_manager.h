#ifndef PBR_UNIFORM_BUFFER_POOL_MANAGER_H
#define PBR_UNIFORM_BUFFER_POOL_MANAGER_H


#include "descriptor_set_layout.h"
#include "uniform_buffer_pool.h"


namespace pbr {

class UniformBufferPoolManager final
{
    private:
        std::vector<VkBufferMemoryBarrier>      _barriers {};
        std::vector<VkDescriptorBufferInfo>     _bufferInfo {};
        VkDescriptorPool                        _descriptorPool = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet>            _descriptorSets {};

        size_t                                  _uniformBaseIndex = 0U;
        size_t                                  _uniformReadIndex = 0U;
        size_t                                  _uniformWriteIndex = 0U;
        size_t                                  _uniformWritten = 0U;

        UniformBufferPool                       _uniformPool;
        std::vector<VkWriteDescriptorSet>       _writeSets {};

    public:
        UniformBufferPoolManager () = delete;

        UniformBufferPoolManager ( UniformBufferPoolManager const & ) = delete;
        UniformBufferPoolManager& operator = ( UniformBufferPoolManager const & ) = delete;

        UniformBufferPoolManager ( UniformBufferPoolManager && ) = delete;
        UniformBufferPoolManager& operator = ( UniformBufferPoolManager && ) = delete;

        explicit UniformBufferPoolManager ( eUniformPoolSize size ) noexcept;

        ~UniformBufferPoolManager () = default;

        [[nodiscard]] VkDescriptorSet Acquire () noexcept;
        void Commit () noexcept;
        void IssueSync ( VkDevice device, VkCommandBuffer commandBuffer ) const noexcept;
        void Push ( android_vulkan::Renderer &renderer, VkCommandBuffer commandBuffer, void const *item ) noexcept;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            DescriptorSetLayout const &descriptorSetLayout,
            size_t itemSize,
            char const *name
        ) noexcept;

        void Destroy ( VkDevice device, char const *name ) noexcept;
};

} // namespace pbr


#endif // PBR_UNIFORM_BUFFER_POOL_MANAGER_H
