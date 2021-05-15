#ifndef PBR_REFLECTION_GLOBAL_PASS_H
#define PBR_REFLECTION_GLOBAL_PASS_H


#include "reflection_global_program.h"
#include "types.h"


namespace pbr {

class ReflectionGlobalPass final
{
    private:
        VkDescriptorPool                        _descriptorPool;
        std::vector<VkDescriptorSet>            _descriptorSets;
        std::vector<VkDescriptorImageInfo>      _imageInfo;
        std::vector<TextureCubeRef>             _prefilters;
        ReflectionGlobalProgram                 _program;
        std::vector<VkWriteDescriptorSet>       _writeSets;

    public:
        ReflectionGlobalPass () noexcept;

        ReflectionGlobalPass ( ReflectionGlobalPass const & ) = delete;
        ReflectionGlobalPass& operator = ( ReflectionGlobalPass const & ) = delete;

        ReflectionGlobalPass ( ReflectionGlobalPass && ) = delete;
        ReflectionGlobalPass& operator = ( ReflectionGlobalPass && ) = delete;

        ~ReflectionGlobalPass () = default;

        void Append ( TextureCubeRef &prefilter );

        [[nodiscard]] bool Execute ( android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer,
            GXMat4 const &viewToWorld
        );

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            VkRenderPass renderPass,
            uint32_t subpass,
            VkExtent2D const &viewport
        );

        void Destroy ( VkDevice device );

        [[nodiscard]] size_t GetReflectionCount () const;
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
