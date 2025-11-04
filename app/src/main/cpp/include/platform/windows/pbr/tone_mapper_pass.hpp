#ifndef PBR_TONE_MAPPER_PASS_HPP
#define PBR_TONE_MAPPER_PASS_HPP


#include "resource_heap.hpp"
#include "tone_mapper_program.hpp"


namespace pbr {

class ToneMapperPass final
{
    private:
        BrightnessInfo                      _brightnessInfo { 0.0F };
        ToneMapperProgram                   _program {};
        ToneMapperProgram::PushConstants    _pushConstants {};

    public:
        explicit ToneMapperPass () = default;

        ToneMapperPass ( ToneMapperPass const & ) = delete;
        ToneMapperPass &operator = ( ToneMapperPass const & ) = delete;

        ToneMapperPass ( ToneMapperPass && ) = delete;
        ToneMapperPass &operator = ( ToneMapperPass && ) = delete;

        ~ToneMapperPass () = default;

        void Destroy ( VkDevice device ) noexcept;
        void Execute ( VkCommandBuffer commandBuffer, ResourceHeap &resourceHeap ) noexcept;

        [[nodiscard]] bool SetBrightness ( android_vulkan::Renderer const &renderer, float brightnessBalance ) noexcept;
        [[nodiscard]] bool SetTarget ( android_vulkan::Renderer const &renderer, uint32_t hdrImage, uint32_t exposure ) noexcept;

    private:
        [[nodiscard]] bool RecreateProgram ( android_vulkan::Renderer const &renderer ) noexcept;
        void UpdateTransform ( android_vulkan::Renderer const &renderer ) noexcept;
};

} // namespace pbr


#endif // PBR_TONE_MAPPER_PASS_HPP
