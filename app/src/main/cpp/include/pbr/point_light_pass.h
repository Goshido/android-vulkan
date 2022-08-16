#ifndef PBR_POINT_LIGHT_PASS_H
#define PBR_POINT_LIGHT_PASS_H


#include "point_light_lightup.h"
#include "point_light_shadowmap_generator_program.h"
#include "scene_data.h"
#include "shadow_casters.h"
#include "uniform_buffer_pool_manager.h"


namespace pbr {

class PointLightPass final
{
    public:
        using PointLightInfo = std::pair<PointLight*, android_vulkan::TextureCube const*>;

    private:
        using Interact = std::pair<LightRef, ShadowCasters>;
        using PointLightShadowmapInfo = std::pair<TextureCubeRef, VkFramebuffer>;

    private:
        std::vector<Interact>                   _interacts {};
        PointLightLightup                       _lightup {};

        UniformBufferPoolManager                _shadowmapBufferPool { eUniformPoolSize::Huge_64M };
        PointLightShadowmapGeneratorProgram     _shadowmapProgram {};
        VkRenderPass                            _shadowmapRenderPass = VK_NULL_HANDLE;
        VkRenderPassBeginInfo                   _shadowmapRenderPassInfo {};
        std::vector<PointLightShadowmapInfo>    _shadowmaps {};

        size_t                                  _usedShadowmaps = 0U;

    public:
        PointLightPass () = default;

        PointLightPass ( PointLightPass const & ) = delete;
        PointLightPass& operator = ( PointLightPass const & ) = delete;

        PointLightPass ( PointLightPass && ) = delete;
        PointLightPass& operator = ( PointLightPass && ) = delete;

        ~PointLightPass () = default;

        void ExecuteLightupPhase ( VkCommandBuffer commandBuffer,
            android_vulkan::MeshGeometry &unitCube,
            UniformBufferPoolManager &volumeBufferPool
        ) noexcept;

        [[nodiscard]] bool ExecuteShadowPhase ( android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer,
            SceneData const &sceneData,
            size_t opaqueMeshCount
        ) noexcept;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            VkExtent2D const &resolution,
            VkRenderPass lightupRenderPass
        ) noexcept;

        void Destroy ( VkDevice device ) noexcept;

        [[nodiscard]] size_t GetPointLightCount () const noexcept;
        [[nodiscard]] PointLightInfo GetPointLightInfo ( size_t lightIndex ) const noexcept;

        void Reset () noexcept;
        void Submit ( LightRef const &light ) noexcept;

        void UploadGPUData ( android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer,
            UniformBufferPoolManager &volumeBufferPool,
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

        void UpdateShadowmapGPUData ( android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer,
            SceneData const &sceneData,
            size_t opaqueMeshCount
        ) noexcept;

        void UpdateLightGPUData ( android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer,
            UniformBufferPoolManager &volumeBufferPool,
            GXMat4 const &viewProjection
        ) noexcept;
};

} // namespace pbr


#endif // PBR_POINT_LIGHT_PASS_H
