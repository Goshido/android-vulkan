#ifndef PBR_REFLECTION_GLOBAL_PASS_H
#define PBR_REFLECTION_GLOBAL_PASS_H


#include "reflection_global_program.h"
#include "types.h"


namespace pbr {

class ReflectionGlobalPass final
{
    private:
        VkDescriptorPool                        _descriptorPool = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet>            _descriptorSets {};
        std::vector<VkDescriptorImageInfo>      _imageInfo {};

        size_t                                  _itemBaseIndex = 0U;
        size_t                                  _itemReadIndex = 0U;
        size_t                                  _itemWriteIndex = 0U;

        std::vector<TextureCubeRef>             _prefilters {};
        ReflectionGlobalProgram                 _program {};

        std::vector<VkWriteDescriptorSet>       _writeSets {};

    public:
        ReflectionGlobalPass () = default;

        ReflectionGlobalPass ( ReflectionGlobalPass const & ) = delete;
        ReflectionGlobalPass &operator = ( ReflectionGlobalPass const & ) = delete;

        ReflectionGlobalPass ( ReflectionGlobalPass && ) = delete;
        ReflectionGlobalPass &operator = ( ReflectionGlobalPass && ) = delete;

        ~ReflectionGlobalPass () = default;

        void Append ( TextureCubeRef &prefilter ) noexcept;
        void Execute ( VkDevice device, VkCommandBuffer commandBuffer ) noexcept;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            VkRenderPass renderPass,
            uint32_t subpass,
            VkExtent2D const &viewport
        ) noexcept;

        void Destroy ( VkDevice device ) noexcept;

        [[nodiscard]] size_t GetReflectionCount () const noexcept;
        void Reset () noexcept;

    private:
        [[nodiscard]] bool AllocateDescriptorSets ( VkDevice device ) noexcept;
        void UpdateGPUData ( VkDevice device, size_t count ) noexcept;
};

} // namespace pbr

#endif // PBR_REFLECTION_GLOBAL_PASS_H
