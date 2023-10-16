#ifndef PBR_LIGHT_PASS_HPP
#define PBR_LIGHT_PASS_HPP


#include "gbuffer.hpp"
#include "lightup_common_descriptor_set.hpp"
#include "point_light_pass.hpp"
#include "reflection_global_pass.hpp"
#include "reflection_local_pass.hpp"


namespace pbr {

class LightPass final
{
    private:
        VkCommandPool                   _commandPool = VK_NULL_HANDLE;
        LightupCommonDescriptorSet      _lightupCommonDescriptorSet {};
        PointLightPass                  _pointLightPass {};
        ReflectionGlobalPass            _reflectionGlobalPass {};
        ReflectionLocalPass             _reflectionLocalPass {};
        android_vulkan::MeshGeometry    _unitCube {};

        UniformBufferPoolManager        _volumeBufferPool
        {
            eUniformPoolSize::Nanoscopic_64KB,
            VK_PIPELINE_STAGE_VERTEX_SHADER_BIT
        };

    public:
        LightPass () = default;

        LightPass ( LightPass const & ) = delete;
        LightPass &operator = ( LightPass const & ) = delete;

        LightPass ( LightPass && ) = delete;
        LightPass &operator = ( LightPass && ) = delete;

        ~LightPass () = default;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            VkRenderPass renderPass,
            GBuffer &gBuffer
        ) noexcept;

        void Destroy ( android_vulkan::Renderer &renderer ) noexcept;

        [[nodiscard]] size_t GetPointLightCount () const noexcept;
        [[nodiscard]] size_t GetReflectionLocalCount () const noexcept;
        void OnFreeTransferResources ( android_vulkan::Renderer &renderer ) noexcept;

        [[nodiscard]] bool OnPreGeometryPass ( android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer,
            size_t commandBufferIndex,
            VkExtent2D const &resolution,
            SceneData const &sceneData,
            size_t opaqueMeshCount,
            GXMat4 const &viewerLocal,
            GXMat4 const &view,
            GXMat4 const &viewProjection,
            GXMat4 const &cvvToView
        ) noexcept;

        void OnPostGeometryPass ( VkDevice device, VkCommandBuffer commandBuffer, size_t commandBufferIndex ) noexcept;
        void Reset () noexcept;

        void SubmitPointLight ( LightRef const &light ) noexcept;
        void SubmitReflectionGlobal ( TextureCubeRef &prefilter ) noexcept;
        void SubmitReflectionLocal ( TextureCubeRef &prefilter, GXVec3 const &location, float size ) noexcept;

    private:
        [[nodiscard]] bool CreateUnitCube ( android_vulkan::Renderer &renderer ) noexcept;
};

} // namespace pbr


#endif // PBR_LIGHT_PASS_HPP
