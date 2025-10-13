#ifndef PBR_SHADOWMAP_POOL_HPP
#define PBR_SHADOWMAP_POOL_HPP


#include "point_light_shadowmap_generator_program.hpp"
#include "uma_uniform_buffer.hpp"


namespace pbr {

class ShadowmapPool final
{
    private:
        std::vector<VkMappedMemoryRange>                    _ranges {};

        VkDescriptorPool                                    _descriptorPool = VK_NULL_HANDLE;
        PointLightShadowmapGeneratorDescriptorSetLayout     _descriptorSetLayout {};
        std::vector<VkDescriptorSet>                        _descriptorSets {};

        size_t                                              _baseIndex = 0U;
        size_t                                              _readIndex = 0U;
        size_t                                              _writeIndex = 0U;
        size_t                                              _written = 0U;

        size_t                                              _nonCoherentAtomSize = 0U;

        UMAUniformBuffer                                    _uniformPool {};

    public:
        explicit ShadowmapPool () = default;

        ShadowmapPool ( ShadowmapPool const & ) = delete;
        ShadowmapPool &operator = ( ShadowmapPool const & ) = delete;

        ShadowmapPool ( ShadowmapPool && ) = delete;
        ShadowmapPool &operator = ( ShadowmapPool && ) = delete;

        ~ShadowmapPool () = default;

        [[nodiscard]] VkDescriptorSet Acquire () noexcept;
        void Commit () noexcept;
        [[nodiscard]] bool HasNewData () const noexcept;
        [[nodiscard]] bool IssueSync ( VkDevice device ) const noexcept;
        void Push ( PointLightShadowmapGeneratorProgram::InstanceData const &data, size_t items ) noexcept;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer ) noexcept;
        void Destroy ( android_vulkan::Renderer &renderer ) noexcept;
};

} // namespace pbr


#endif // PBR_SHADOWMAP_POOL_HPP
