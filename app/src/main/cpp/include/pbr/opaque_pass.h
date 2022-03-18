#ifndef PBR_OPAQUE_PASS_H
#define PBR_OPAQUE_PASS_H


#include "default_texture_manager.h"
#include "opaque_program.h"
#include "render_session_stats.h"
#include "sampler_manager.h"
#include "scene_data.h"
#include "uniform_buffer_pool.h"
#include <primitive_types.h>


namespace pbr {

class OpaquePass final
{
    private:
        VkDescriptorPool                        _descriptorPool = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet>            _descriptorSetStorage {};
        std::vector<VkDescriptorImageInfo>      _imageStorage {};
        std::vector<VkDescriptorSetLayout>      _layouts {};
        std::vector<VkDescriptorPoolSize>       _poolSizeStorage {};
        OpaqueProgram                           _program {};
        SceneData                               _sceneData {};
        VkSubmitInfo                            _submitInfoTransfer {};
        VkCommandBuffer                         _transferCommandBuffer = VK_NULL_HANDLE;
        UniformBufferPool                       _uniformPool { eUniformPoolSize::Huge_64M };
        std::vector<VkDescriptorBufferInfo>     _uniformStorage {};
        std::vector<VkWriteDescriptorSet>       _writeStorage0 {};
        std::vector<VkWriteDescriptorSet>       _writeStorage1 {};

    public:
        OpaquePass () = default;

        OpaquePass ( OpaquePass const & ) = delete;
        OpaquePass& operator = ( OpaquePass const & ) = delete;

        OpaquePass ( OpaquePass && ) = delete;
        OpaquePass& operator = ( OpaquePass && ) = delete;

        ~OpaquePass () = default;

        [[nodiscard]] SceneData& GetSceneData () noexcept;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            VkCommandPool commandPool,
            VkExtent2D const &resolution,
            VkRenderPass renderPass
        ) noexcept;

        void Destroy ( VkDevice device ) noexcept;

        [[nodiscard]] bool Execute ( android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer,
            GXProjectionClipPlanes const &frustum,
            GXMat4 const &view,
            GXMat4 const &viewProjection,
            DefaultTextureManager const &defaultTextureManager,
            SamplerManager &samplerManager,
            RenderSessionStats &renderSessionStats
        ) noexcept;

        void Reset () noexcept;

        void Submit ( MeshRef &mesh,
            MaterialRef const &material,
            GXMat4 const &local,
            GXAABB const &worldBounds,
            GXColorRGB const &color0,
            GXColorRGB const &color1,
            GXColorRGB const &color2,
            GXColorRGB const &emission
        ) noexcept;

    private:
        [[nodiscard]] size_t AggregateUniformCount () const noexcept;

        void AppendDrawcalls ( VkCommandBuffer commandBuffer,
            VkDescriptorSet const* textureSets,
            VkDescriptorSet const* instanceSets,
            RenderSessionStats &renderSessionStats
        ) noexcept;

        void DestroyDescriptorPool ( VkDevice device ) noexcept;

        [[nodiscard]] bool UpdateGPUData ( android_vulkan::Renderer &renderer,
            GXProjectionClipPlanes const &frustum,
            GXMat4 const &view,
            GXMat4 const &viewProjection,
            std::vector<VkDescriptorSet> &descriptorSetStorage,
            DefaultTextureManager const &defaultTextureManager,
            SamplerManager &samplerManager
        ) noexcept;
};

} // namespace pbr


#endif // PBR_OPAQUE_PASS_H
