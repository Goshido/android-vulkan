#ifndef PBR_GEOMETRY_POOL_HPP
#define PBR_GEOMETRY_POOL_HPP


#include "geometry_pass_program.hpp"
#include "uma_uniform_buffer.hpp"


namespace pbr {

class GeometryPool final
{
    private:
        std::vector<VkMappedMemoryRange>            _vertexRanges {};
        std::vector<VkMappedMemoryRange>            _fragmentRanges {};

        VkDescriptorPool                            _descriptorPool = VK_NULL_HANDLE;
        GeometryPassInstanceDescriptorSetLayout     _descriptorSetLayout {};
        std::vector<VkDescriptorSet>                _descriptorSets {};

        size_t                                      _baseIndex = 0U;
        size_t                                      _readIndex = 0U;
        size_t                                      _writeIndex = 0U;
        size_t                                      _written = 0U;

        size_t                                      _nonCoherentAtomSize = 0U;

        UMAUniformBuffer                            _positionPool {};
        UMAUniformBuffer                            _normalPool {};
        UMAUniformBuffer                            _colorPool {};

    public:
        explicit GeometryPool () = default;

        GeometryPool ( GeometryPool const & ) = delete;
        GeometryPool &operator = ( GeometryPool const & ) = delete;

        GeometryPool ( GeometryPool && ) = delete;
        GeometryPool &operator = ( GeometryPool && ) = delete;

        ~GeometryPool () = default;

        [[nodiscard]] VkDescriptorSet Acquire () noexcept;
        void Commit () noexcept;
        [[nodiscard]] bool HasNewData () const noexcept;
        [[nodiscard]] bool IssueSync ( VkDevice device ) const noexcept;

        void Push ( GeometryPassProgram::InstancePositionData const &positionData,
            GeometryPassProgram::InstanceNormalData const &normalData,
            GeometryPassProgram::InstanceColorData const &colorData,
            size_t items
        ) noexcept;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer ) noexcept;
        void Destroy ( android_vulkan::Renderer &renderer ) noexcept;
};

} // namespace pbr


#endif // PBR_GEOMETRY_POOL_HPP
