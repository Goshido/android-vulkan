#ifndef PBR_GEOMETRY_SUBPASS_BASE_H
#define PBR_GEOMETRY_SUBPASS_BASE_H


#include "geometry_pass_program.h"
#include "render_session_stats.h"
#include "scene_data.h"
#include "uniform_buffer_pool.h"


namespace pbr {

class GeometrySubpassBase {
    protected:
        VkDescriptorPool                        _descriptorPool = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet>            _descriptorSetStorage {};
        std::vector<VkDescriptorImageInfo>      _imageStorage {};
        std::vector<VkDescriptorSetLayout>      _layouts {};
        std::vector<VkDescriptorPoolSize>       _poolSizeStorage {};
        SceneData                               _sceneData {};
        VkSubmitInfo                            _submitInfoTransfer {};
        VkCommandBuffer                         _transferCommandBuffer = VK_NULL_HANDLE;
        UniformBufferPool                       _uniformPool { eUniformPoolSize::Huge_64M };
        std::vector<VkDescriptorBufferInfo>     _uniformStorage {};
        std::vector<VkWriteDescriptorSet>       _writeStorage0 {};
        std::vector<VkWriteDescriptorSet>       _writeStorage1 {};

    public:
        GeometrySubpassBase ( GeometrySubpassBase const & ) = delete;
        GeometrySubpassBase& operator = ( GeometrySubpassBase const & ) = delete;

        GeometrySubpassBase ( GeometrySubpassBase && ) = delete;
        GeometrySubpassBase& operator = ( GeometrySubpassBase && ) = delete;

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

    protected:
        GeometrySubpassBase () = default;
        ~GeometrySubpassBase () = default;

        virtual void ReportGeometry ( RenderSessionStats &renderSessionStats,
            uint32_t vertexCount,
            uint32_t instanceCount
        ) noexcept = 0;

        [[nodiscard]] size_t AggregateUniformCount () const noexcept;

        void AppendDrawcalls ( VkCommandBuffer commandBuffer,
            GeometryPassProgram &program,
            VkDescriptorSet const* textureSets,
            VkDescriptorSet const* instanceSets,
            RenderSessionStats &renderSessionStats
        ) noexcept;

        // Must be called from child class.
        [[nodiscard]] bool InitBase ( android_vulkan::Renderer &renderer, VkCommandPool commandPool ) noexcept;

        // Must be called from child class.
        void DestroyBase ( VkDevice device ) noexcept;

        void DestroyDescriptorPool ( VkDevice device ) noexcept;
        [[nodiscard]] bool RecreateDescriptorPool ( VkDevice device, size_t maxSets ) noexcept;
};

} // namespace pbr


#endif // PBR_GEOMETRY_SUBPASS_BASE_H
