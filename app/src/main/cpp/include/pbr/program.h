#ifndef PBR_MATERIAL_H
#define PBR_MATERIAL_H


#include <renderer.h>

GX_DISABLE_COMMON_WARNINGS

#include <string>

GX_RESTORE_WARNING_STATE


namespace pbr {

enum class eProgramState : uint8_t
{
    Bind,
    Initializing,
    Ready,
    Setup,
    Unknown
};

class Program
{
    protected:
        VkShaderModule                  _fragmentShader;
        VkShaderModule                  _vertexShader;

        const std::string               _name;
        VkPipeline                      _pipeline;
        VkPipelineLayout                _pipelineLayout;

        eProgramState                   _state;

    public:
        Program () = delete;
        Program ( const Program &other ) = delete;
        Program& operator = ( const Program &other ) = delete;

        // Methods must be called between changing input shader parameters: VkImageView, VkSampler, VkBuffer.
        [[maybe_unused]] virtual void BeginSetup () = 0;
        [[maybe_unused]] virtual void EndSetup () = 0;

        // Method commits active material parameters and assigns VkPipeline as active pipeline.
        // Method return true is success. Otherwise method returns false.
        [[maybe_unused]] [[nodiscard]] virtual bool Bind ( android_vulkan::Renderer &renderer ) = 0;

        // Method return true is success. Otherwise method returns false.
        [[maybe_unused]] [[nodiscard]] virtual bool Init ( android_vulkan::Renderer &renderer,
            VkRenderPass renderPass,
            const VkExtent2D &viewport
        ) = 0;

        [[maybe_unused]] virtual void Destroy ( android_vulkan::Renderer &renderer ) = 0;

    protected:
        explicit Program ( std::string &&name );
        virtual ~Program () = default;
};

} // namespace pbr


#endif // PBR_MATERIAL_H
