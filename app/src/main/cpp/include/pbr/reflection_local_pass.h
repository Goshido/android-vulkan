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

            explicit Call ( GXVec3 const &location, TextureCubeRef &prefilter, float size ) noexcept;

            ~Call () = default;
        };

    private:
        std::vector<VkDescriptorBufferInfo>     _bufferInfo {};
        std::vector<Call>                       _calls {};
        VkCommandPool                           _commandPool = VK_NULL_HANDLE;
        VkDescriptorPool                        _descriptorPool = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet>            _descriptorSets {};
        std::vector<VkDescriptorImageInfo>      _imageInfo {};
        LightPassNotifier*                      _lightPassNotifier = nullptr;
        UniformBufferPool                       _lightVolumeUniforms { eUniformPoolSize::Tiny_4M };
        ReflectionLocalProgram                  _program {};
        UniformBufferPool                       _reflectionUniforms { eUniformPoolSize::Tiny_4M };
        VkCommandBuffer                         _transfer = VK_NULL_HANDLE;
        VkFence                                 _transferFence = VK_NULL_HANDLE;
        VkSubmitInfo                            _transferSubmit {};
        std::vector<VkWriteDescriptorSet>       _writeSets {};

    public:
        ReflectionLocalPass () = default;

        ReflectionLocalPass ( ReflectionLocalPass const & ) = delete;
        ReflectionLocalPass& operator = ( ReflectionLocalPass const & ) = delete;

        ReflectionLocalPass ( ReflectionLocalPass && ) = delete;
        ReflectionLocalPass& operator = ( ReflectionLocalPass && ) = delete;

        ~ReflectionLocalPass () = default;

        void Append ( TextureCubeRef &prefilter, GXVec3 const &location, float size ) noexcept;

        bool Execute ( android_vulkan::Renderer &renderer,
            LightVolume &lightVolume,
            android_vulkan::MeshGeometry &unitCube,
            VkCommandBuffer commandBuffer
        ) noexcept;

        [[nodiscard]] size_t GetReflectionLocalCount () const noexcept;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            LightPassNotifier &notifier,
            VkCommandPool commandPool,
            VkRenderPass renderPass,
            uint32_t subpass,
            VkExtent2D const &viewport
        ) noexcept;

        void Destroy ( VkDevice device ) noexcept;

        void Reset () noexcept;

        [[nodiscard]] bool UploadGPUData ( android_vulkan::Renderer &renderer,
            GXMat4 const &view,
            GXMat4 const &viewProjection
        ) noexcept;

    private:
        [[nodiscard]] bool AllocateDescriptorSets ( android_vulkan::Renderer &renderer, size_t neededCalls ) noexcept;
        void DestroyDescriptorPool ( VkDevice device ) noexcept;
};

} // namespace pbr

#endif // PBR_REFLECTION_LOCAL_PASS_H
