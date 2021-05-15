#ifndef PBR_REFLECTION_LOCAL_PASS_H
#define PBR_REFLECTION_LOCAL_PASS_H


#include "light_pass_notifier.h"
#include "light_volume.h"
#include "reflection_local_program.h"
#include "types.h"
#include "uniform_buffer_pool.h"


namespace pbr {

class ReflectionLocalPass final
{
    private:
        struct Call final
        {
            GXVec3                              _location;
            TextureCubeRef                      _prefilter;
            float                               _size;

            Call () = default;

            Call ( Call const & ) = default;
            Call& operator = ( Call const & ) = default;

            Call ( Call && ) = default;
            Call& operator = ( Call && ) = default;

            explicit Call ( GXVec3 const &location, TextureCubeRef &prefilter, float size );

            ~Call () = default;
        };

    private:
        std::vector<VkDescriptorBufferInfo>     _bufferInfo;
        std::vector<Call>                       _calls;
        VkCommandPool                           _commandPool;
        VkDescriptorPool                        _descriptorPool;
        std::vector<VkDescriptorSet>            _descriptorSets;
        std::vector<VkDescriptorImageInfo>      _imageInfo;
        LightPassNotifier*                      _lightPassNotifier;
        UniformBufferPool                       _lightVolumeUniforms;
        ReflectionLocalProgram                  _program;
        UniformBufferPool                       _reflectionUniforms;
        VkCommandBuffer                         _transfer;
        VkFence                                 _transferFence;
        VkSubmitInfo                            _transferSubmit;
        std::vector<VkWriteDescriptorSet>       _writeSets;

    public:
        ReflectionLocalPass () noexcept;

        ReflectionLocalPass ( ReflectionLocalPass const & ) = delete;
        ReflectionLocalPass& operator = ( ReflectionLocalPass const & ) = delete;

        ReflectionLocalPass ( ReflectionLocalPass && ) = delete;
        ReflectionLocalPass& operator = ( ReflectionLocalPass && ) = delete;

        ~ReflectionLocalPass () = default;

        void Append ( TextureCubeRef &prefilter, GXVec3 const &location, float size );

        bool Execute ( android_vulkan::Renderer &renderer,
            LightVolume &lightVolume,
            android_vulkan::MeshGeometry &unitCube,
            VkCommandBuffer commandBuffer
        );

        [[nodiscard]] size_t GetReflectionLocalCount () const;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            LightPassNotifier &notifier,
            VkCommandPool commandPool,
            VkRenderPass renderPass,
            uint32_t subpass,
            VkExtent2D const &viewport
        );

        void Destroy ( VkDevice device );

        void Reset ();

        [[nodiscard]] bool UploadGPUData ( android_vulkan::Renderer &renderer,
            GXMat4 const &view,
            GXMat4 const &viewProjection
        );

    private:
        [[nodiscard]] bool AllocateDescriptorSets ( android_vulkan::Renderer &renderer, size_t neededCalls );
        void DestroyDescriptorPool ( VkDevice device );
};

} // namespace pbr

#endif // PBR_REFLECTION_LOCAL_PASS_H
