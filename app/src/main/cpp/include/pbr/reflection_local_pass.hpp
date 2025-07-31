#ifndef PBR_REFLECTION_LOCAL_PASS_HPP
#define PBR_REFLECTION_LOCAL_PASS_HPP


#include "reflection_local_program.hpp"
#include "types.hpp"
#include "uma_uniform_buffer.hpp"
#include "uma_uniform_pool.hpp"


namespace pbr {

class ReflectionLocalPass final
{
    private:
        struct Call final
        {
            GXVec3                              _location { 0.0F, 0.0F, 0.0F };
            TextureCubeRef                      _prefilter;
            float                               _size = 0.0F;

            Call () = default;

            Call ( Call const & ) = default;
            Call &operator = ( Call const & ) = default;

            Call ( Call && ) = default;
            Call &operator = ( Call && ) = default;

            explicit Call ( GXVec3 const &location, TextureCubeRef &prefilter, float size ) noexcept;

            ~Call () = default;
        };

    private:
        UMAUniformPool                          &_volumeDataPool;

        std::vector<Call>                       _calls {};
        VkDescriptorPool                        _descriptorPool = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet>            _descriptorSets {};
        std::vector<VkDescriptorImageInfo>      _imageInfo {};

        size_t                                  _itemBaseIndex = 0U;
        size_t                                  _itemReadIndex = 0U;
        size_t                                  _itemWriteIndex = 0U;
        size_t                                  _itemWritten = 0U;

        ReflectionLocalProgram                  _program {};
        VkMappedMemoryRange                     _ranges[ 2U ]{};
        UMAUniformBuffer                        _uniformPool {};
        std::vector<VkWriteDescriptorSet>       _writeSets {};

    public:
        ReflectionLocalPass () = delete;

        ReflectionLocalPass ( ReflectionLocalPass const & ) = delete;
        ReflectionLocalPass &operator = ( ReflectionLocalPass const & ) = delete;

        ReflectionLocalPass ( ReflectionLocalPass && ) = delete;
        ReflectionLocalPass &operator = ( ReflectionLocalPass && ) = delete;

        explicit ReflectionLocalPass ( UMAUniformPool &volumeDataPool ) noexcept;

        ~ReflectionLocalPass () = default;

        void Append ( TextureCubeRef &prefilter, GXVec3 const &location, float size ) noexcept;

        void Execute ( VkCommandBuffer commandBuffer,
            android_vulkan::android::MeshGeometry &unitCube
        ) noexcept;

        [[nodiscard]] size_t GetReflectionLocalCount () const noexcept;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            VkRenderPass renderPass,
            VkExtent2D const &viewport
        ) noexcept;

        void Destroy ( android_vulkan::Renderer &renderer ) noexcept;
        void Reset () noexcept;
        [[nodiscard]] bool UploadGPUData ( VkDevice device, GXMat4 const &view, GXMat4 const &viewProjection ) noexcept;
};

} // namespace pbr

#endif // PBR_REFLECTION_LOCAL_PASS_HPP
