#ifndef PBR_POINT_LIGHT_PASS_HPP
#define PBR_POINT_LIGHT_PASS_HPP


#include "point_light_lightup.hpp"
#include "point_light_shadowmap_generator_program.hpp"
#include "scene_data.hpp"
#include "shadow_casters.hpp"
#include "shadowmap_pool.hpp"
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

        ShadowmapPool                           _shadowmapPool {};
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
        [[nodiscard]] bool ExecuteShadowPhase ( VkCommandBuffer commandBuffer ) noexcept;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            VkExtent2D const &resolution,
            VkRenderPass lightupRenderPass
        ) noexcept;

        void Destroy ( android_vulkan::Renderer &renderer ) noexcept;

        [[nodiscard]] size_t GetPointLightCount () const noexcept;
        [[nodiscard]] PointLightInfo GetPointLightInfo ( size_t lightIndex ) const noexcept;

        void Reset () noexcept;
        void Submit ( LightRef const &light ) noexcept;

        [[nodiscard]] bool UploadGPUData ( android_vulkan::Renderer &renderer,
            SceneData const &sceneData,
            size_t opaqueMeshCount,
            GXMat4 const &viewerLocal,
            GXMat4 const &view,
            GXMat4 const &viewProjection
        ) noexcept;

    private:
        [[nodiscard]] bool CreateShadowmapRenderPass ( VkDevice device ) noexcept;
        [[nodiscard]] bool GenerateShadowmaps ( VkCommandBuffer commandBuffer ) noexcept;
        [[nodiscard]] bool ReserveShadowmaps ( android_vulkan::Renderer &renderer ) noexcept;

        [[nodiscard]] bool UpdateShadowmapGPUData ( VkDevice device,
            SceneData const &sceneData,
            size_t opaqueMeshCount
        ) noexcept;

        void UpdateLightGPUData ( GXMat4 const &viewProjection ) noexcept;
};

} // namespace pbr


#endif // PBR_POINT_LIGHT_PASS_HPP
