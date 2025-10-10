#ifndef PBR_UNIFORM_POOL_HPP
#define PBR_UNIFORM_POOL_HPP


#include <pbr/descriptor_set_layout.hpp>
#include "uniform_buffer.hpp"


namespace pbr {

class UniformPool final
{
    private:
        std::vector<VkBufferMemoryBarrier>      _barriers {};
        std::vector<VkDescriptorBufferInfo>     _bufferInfo {};
        VkDescriptorPool                        _descriptorPool = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet>            _descriptorSets {};
        VkPipelineStageFlags const              _syncFlags;

        size_t                                  _baseIndex = 0U;
        size_t                                  _readIndex = 0U;
        size_t                                  _writeIndex = 0U;
        size_t                                  _written = 0U;

        UniformBuffer                           _uniformBuffer;
        std::vector<VkWriteDescriptorSet>       _writeSets {};

    public:
        UniformPool () = delete;

        UniformPool ( UniformPool const & ) = delete;
        UniformPool &operator = ( UniformPool const & ) = delete;

        UniformPool ( UniformPool && ) = delete;
        UniformPool &operator = ( UniformPool && ) = delete;

        explicit UniformPool ( eUniformSize size, VkPipelineStageFlags syncFlags ) noexcept;

        ~UniformPool () = default;

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

        void Destroy ( android_vulkan::Renderer &renderer ) noexcept;
};

} // namespace pbr


#endif // PBR_UNIFORM_POOL_HPP
