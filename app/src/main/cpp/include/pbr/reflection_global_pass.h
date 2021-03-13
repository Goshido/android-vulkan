#ifndef PBR_REFLECTION_GLOBAL_PASS_H
#define PBR_REFLECTION_GLOBAL_PASS_H


#include "lightup_common_descriptor_set.h"
#include "reflection_global_program.h"
#include "sampler.h"
#include "types.h"
#include "uniform_buffer_pool.h"


namespace pbr {

class ReflectionGlobalPass final
{
    private:
        VkCommandPool                           _commandPool;
        VkDescriptorPool                        _descriptorPool;
        std::vector<VkDescriptorSet>            _descriptorSets;
        std::vector<VkDescriptorImageInfo>      _imageInfo;
        std::vector<TextureCubeRef>             _prefilters;
        ReflectionGlobalProgram                 _program;
        Sampler                                 _sampler;
        VkSubmitInfo                            _submitInfoTransfer;
        VkCommandBuffer                         _transferCommandBuffer;
        std::vector<VkDescriptorBufferInfo>     _uniformInfo;
        UniformBufferPool                       _uniformPool;
        std::vector<VkWriteDescriptorSet>       _writeSets;

    public:
        ReflectionGlobalPass () noexcept;

        ReflectionGlobalPass ( ReflectionGlobalPass const & ) = delete;
        ReflectionGlobalPass& operator = ( ReflectionGlobalPass const & ) = delete;

        ReflectionGlobalPass ( ReflectionGlobalPass && ) = delete;
        ReflectionGlobalPass& operator = ( ReflectionGlobalPass && ) = delete;

        ~ReflectionGlobalPass () = default;

        void Append ( TextureCubeRef &prefilter );

        [[maybe_unused]] bool Execute ( android_vulkan::Renderer &renderer,
            bool &isCommonSetBind,
            LightupCommonDescriptorSet &lightupCommonDescriptorSet,
            VkCommandBuffer commandBuffer,
            GXMat4 const &viewToWorld
        );

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            VkRenderPass renderPass,
            uint32_t subpass,
            VkExtent2D const &viewport
        );

        void Destroy ( VkDevice device );
        void Reset ();

    private:
        [[nodiscard]] bool AllocateDescriptorSets ( android_vulkan::Renderer &renderer, size_t neededSets );
        void DestroyDescriptorPool ( VkDevice device );

        [[nodiscard]] bool UpdateGPUData ( android_vulkan::Renderer &renderer,
            size_t count,
            GXMat4 const &viewToWorld
        );
};

} // namespace pbr

#endif // PBR_REFLECTION_GLOBAL_PASS_H