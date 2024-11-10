#ifndef PBR_GEOMETRY_POOL_HPP
#define PBR_GEOMETRY_POOL_HPP


#include "geometry_pass_instance_descriptor_set_layout.hpp"
#include "geometry_pass_program.hpp"
#include "uniform_buffer_pool.hpp"


namespace pbr {

class GeometryPool final
{
    private:
        std::vector<VkBufferMemoryBarrier>          _vertexBarriers {};
        std::vector<VkBufferMemoryBarrier>          _fragmentBarriers {};

        std::vector<VkDescriptorBufferInfo>         _bufferInfo {};
        VkDescriptorPool                            _descriptorPool = VK_NULL_HANDLE;
        GeometryPassInstanceDescriptorSetLayout     _descriptorSetLayout {};
        std::vector<VkDescriptorSet>                _descriptorSets {};

        size_t                                      _baseIndex = 0U;
        size_t                                      _readIndex = 0U;
        size_t                                      _writeIndex = 0U;
        size_t                                      _written = 0U;

        UniformBufferPool                           _positionPool { eUniformPoolSize::Big_32M };
        UniformBufferPool                           _normalPool { eUniformPoolSize::Small_8M };
        UniformBufferPool                           _colorPool { eUniformPoolSize::Big_32M };
        std::vector<VkWriteDescriptorSet>           _writeSets {};

    public:
        explicit GeometryPool () = default;

        GeometryPool ( GeometryPool const & ) = delete;
        GeometryPool &operator = ( GeometryPool const & ) = delete;

        GeometryPool ( GeometryPool && ) = delete;
        GeometryPool &operator = ( GeometryPool && ) = delete;

        ~GeometryPool () = default;

        [[nodiscard]] VkDescriptorSet Acquire () noexcept;
        void Commit () noexcept;
        void IssueSync ( VkDevice device, VkCommandBuffer commandBuffer ) const noexcept;

        void Push ( VkCommandBuffer commandBuffer,
            GeometryPassProgram::InstancePositionData const &positionData,
            GeometryPassProgram::InstanceNormalData const &normalData,
            GeometryPassProgram::InstanceColorData const &colorData,
            size_t items
        ) noexcept;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer ) noexcept;
        void Destroy ( android_vulkan::Renderer &renderer ) noexcept;
};

} // namespace pbr


#endif // PBR_GEOMETRY_POOL_HPP
