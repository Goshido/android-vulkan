#ifndef OPAQUE_PROGRAM_H
#define OPAQUE_PROGRAM_H


#include <texture2D.h>
#include "program.h"


namespace pbr {

class OpaqueProgram final : public Program
{
    private:
        android_vulkan::Texture2D*      _albedoTexture;
        VkSampler                       _albedoSampler;

        android_vulkan::Texture2D*      _emissionTexture;
        VkSampler                       _emissionSampler;

        android_vulkan::Texture2D*      _normalTexture;
        VkSampler                       _normalSampler;

        android_vulkan::Texture2D*      _paramTexture;
        VkSampler                       _paramSampler;

        VkDescriptorSetLayout           _descriptorSetLayouts[ 2U ];

    public:
        OpaqueProgram ();
        ~OpaqueProgram () override = default;

        OpaqueProgram ( const OpaqueProgram &other ) = delete;
        OpaqueProgram& operator = ( const OpaqueProgram &other ) = delete;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            VkRenderPass renderPass,
            const VkExtent2D &viewport
        ) override;

        void Destroy ( android_vulkan::Renderer &renderer ) override;

        [[maybe_unused]] void SetAlbedo ( android_vulkan::Texture2D &texture, VkSampler sampler );
        [[maybe_unused]] void SetEmission ( android_vulkan::Texture2D &texture, VkSampler sampler );
        [[maybe_unused]] void SetNormal ( android_vulkan::Texture2D &texture, VkSampler sampler );
        [[maybe_unused]] void SetParams ( android_vulkan::Texture2D &texture, VkSampler sampler );

    private:
        void BeginSetup () override;
        void EndSetup () override;

        bool Bind ( android_vulkan::Renderer &renderer ) override;
};

} // namespace pbr


#endif // OPAQUE_PROGRAM_H
