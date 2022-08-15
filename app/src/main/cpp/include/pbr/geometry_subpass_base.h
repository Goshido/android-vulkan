#ifndef PBR_GEOMETRY_SUBPASS_BASE_H
#define PBR_GEOMETRY_SUBPASS_BASE_H


#include "geometry_pass_program.h"
#include "render_session_stats.h"
#include "scene_data.h"
#include "uniform_buffer_pool.h"


namespace pbr {

class GeometrySubpassBase
{
    private:
        struct TransferInfo final
        {
            std::vector<VkDescriptorSet>            _sets {};
            VkDescriptorPool                        _pool = VK_NULL_HANDLE;
            std::vector<VkWriteDescriptorSet>       _writes {};
        };

    protected:
        std::vector<VkDescriptorImageInfo>          _imageStorage {};
        TransferInfo                                _materialTransfer {};
        SceneData                                   _sceneData {};
        VkSubmitInfo                                _submitInfoTransfer {};
        VkCommandBuffer                             _transferCommandBuffer = VK_NULL_HANDLE;
        UniformBufferPool                           _uniformPool { eUniformPoolSize::Huge_64M, true };
        std::vector<VkDescriptorBufferInfo>         _uniformStorage {};
        TransferInfo                                _uniformTransfer {};

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
        [[nodiscard]] bool AllocateMaterialSystem ( VkDevice device, size_t materials, size_t textureCount ) noexcept;
        [[nodiscard]] bool AllocateTransferSystem ( VkDevice device ) noexcept;
        [[nodiscard]] bool AllocateUniformBufferSystem ( VkDevice device, size_t uniformCount ) noexcept;

        void AppendDrawcalls ( VkCommandBuffer commandBuffer,
            GeometryPassProgram &program,
            RenderSessionStats &renderSessionStats
        ) noexcept;

        // Must be called from child class.
        [[nodiscard]] bool InitBase ( android_vulkan::Renderer &renderer, VkCommandPool commandPool ) noexcept;

        // Must be called from child class.
        void DestroyBase ( VkDevice device ) noexcept;

        void DestroyMaterialDescriptorPool ( VkDevice device ) noexcept;
        void DestroyUniformDescriptorPool ( VkDevice device ) noexcept;
};

} // namespace pbr


#endif // PBR_GEOMETRY_SUBPASS_BASE_H
