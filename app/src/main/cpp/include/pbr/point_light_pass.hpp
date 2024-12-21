#ifndef PBR_POINT_LIGHT_PASS_HPP
#define PBR_POINT_LIGHT_PASS_HPP


#include "point_light_lightup.hpp"
#include "point_light_shadowmap_generator_program.hpp"
#include "scene_data.hpp"
#include "shadow_casters.hpp"

// FUCK remove me
#include "uniform_buffer_pool_manager.hpp"

#include "uma_uniform_pool.hpp"


namespace pbr {

class PointLightPass final
{
    public:
        using PointLightInfo = std::pair<PointLight*, android_vulkan::TextureCube const*>;

    private:
        using Interact = std::pair<LightRef, ShadowCasters>;
        using PointLightShadowmapInfo = std::pair<TextureCubeRef, VkFramebuffer>;

    private:
        UMAUniformPool                          &_volumeDataPool;

        std::vector<Interact>                   _interacts {};
        PointLightLightup                       _lightup {};

        // FUCK replace me by UMASparseUniformBuffer
        UniformBufferPoolManager                _shadowmapBufferPool
        {
            eUniformPoolSize::Huge_64M,
            VK_PIPELINE_STAGE_VERTEX_SHADER_BIT
        };

        PointLightShadowmapGeneratorProgram     _shadowmapProgram {};
        VkRenderPass                            _shadowmapRenderPass = VK_NULL_HANDLE;
        VkRenderPassBeginInfo                   _shadowmapRenderPassInfo {};
        std::vector<PointLightShadowmapInfo>    _shadowmaps {};

        size_t                                  _usedShadowmaps = 0U;

    public:
        PointLightPass () = delete;

        PointLightPass ( PointLightPass const & ) = delete;
        PointLightPass &operator = ( PointLightPass const & ) = delete;

        PointLightPass ( PointLightPass && ) = delete;
        PointLightPass &operator = ( PointLightPass && ) = delete;

        explicit PointLightPass ( UMAUniformPool &volumeDataPool ) noexcept;

        ~PointLightPass () = default;

        void ExecuteLightupPhase ( VkCommandBuffer commandBuffer, android_vulkan::MeshGeometry &unitCube ) noexcept;

        [[nodiscard]] bool ExecuteShadowPhase ( android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer,
            SceneData const &sceneData,
            size_t opaqueMeshCount
        ) noexcept;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            VkExtent2D const &resolution,
            VkRenderPass lightupRenderPass
        ) noexcept;

        void Destroy ( android_vulkan::Renderer &renderer ) noexcept;

        [[nodiscard]] size_t GetPointLightCount () const noexcept;
        [[nodiscard]] PointLightInfo GetPointLightInfo ( size_t lightIndex ) const noexcept;

        void Reset () noexcept;
        void Submit ( LightRef const &light ) noexcept;

        [[nodiscard]] bool UploadGPUData ( VkDevice device,
            GXMat4 const &viewerLocal,
            GXMat4 const &view,
            GXMat4 const &viewProjection
        ) noexcept;

    private:
        // The method returns nullptr if it fails. Otherwise the method returns a valid pointer.
        [[nodiscard]] PointLightShadowmapInfo* AcquirePointLightShadowmap (
            android_vulkan::Renderer &renderer
        ) noexcept;

        [[nodiscard]] bool CreateShadowmapRenderPass ( VkDevice device ) noexcept;

        [[nodiscard]] bool GenerateShadowmaps ( android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer
        ) noexcept;

        void UpdateShadowmapGPUData ( VkDevice device,
            VkCommandBuffer commandBuffer,
            SceneData const &sceneData,
            size_t opaqueMeshCount
        ) noexcept;

        void UpdateLightGPUData ( GXMat4 const &viewProjection ) noexcept;
};

} // namespace pbr


#endif // PBR_POINT_LIGHT_PASS_HPP
