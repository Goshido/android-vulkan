#ifndef PBR_LIGHT_PASS_HPP
#define PBR_LIGHT_PASS_HPP


#include "dummy_light_program.hpp"
#include "lightup_common_descriptor_set.hpp"
#include <pbr/gbuffer.hpp>
#include "point_light_pass.hpp"
#include "reflection_global_pass.hpp"
#include "reflection_local_pass.hpp"


namespace pbr {

class LightPass final
{
    private:
        VkCommandPool                   _commandPool = VK_NULL_HANDLE;
        DummyLightProgram               _dummyLightProgram {};
        LightupCommonDescriptorSet      _lightupCommonDescriptorSet {};

        UMAUniformPool                  _volumeDataPool {};

        PointLightPass                  _pointLightPass { _volumeDataPool };
        ReflectionGlobalPass            _reflectionGlobalPass {};
        ReflectionLocalPass             _reflectionLocalPass { _volumeDataPool };
        android_vulkan::MeshGeometry    _unitCube {};

        bool                            _hasWork = false;

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

        [[nodiscard]] bool OnPreGeometryPass ( VkCommandBuffer commandBuffer ) noexcept;
        void OnPostGeometryPass ( VkDevice device, VkCommandBuffer commandBuffer, size_t commandBufferIndex ) noexcept;
        void Reset () noexcept;

        void SubmitPointLight ( LightRef const &light ) noexcept;
        void SubmitReflectionGlobal ( TextureCubeRef &prefilter ) noexcept;
        void SubmitReflectionLocal ( TextureCubeRef &prefilter, GXVec3 const &location, float size ) noexcept;

        [[nodiscard]] bool UploadGPUData ( android_vulkan::Renderer &renderer,
            size_t commandBufferIndex,
            SceneData const &sceneData,
            size_t opaqueMeshCount,
            VkExtent2D const &resolution,
            GXMat4 const &viewerLocal,
            GXMat4 const &view,
            GXMat4 const &viewProjection,
            GXMat4 const &cvvToView
        ) noexcept;

    private:
        [[nodiscard]] bool CreateUnitCube ( android_vulkan::Renderer &renderer ) noexcept;
};

} // namespace pbr


#endif // PBR_LIGHT_PASS_HPP
