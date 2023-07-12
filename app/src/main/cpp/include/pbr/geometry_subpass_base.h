#ifndef PBR_GEOMETRY_SUBPASS_BASE_H
#define PBR_GEOMETRY_SUBPASS_BASE_H


#include "geometry_pass_program.h"
#include "material_pool.h"
#include "render_session_stats.h"
#include "scene_data.h"
#include "uniform_buffer_pool_manager.h"


namespace pbr {

class GeometrySubpassBase
{
    protected:
        SceneData       _sceneData {};

    public:
        GeometrySubpassBase ( GeometrySubpassBase const & ) = delete;
        GeometrySubpassBase &operator = ( GeometrySubpassBase const & ) = delete;

        GeometrySubpassBase ( GeometrySubpassBase && ) = delete;
        GeometrySubpassBase &operator = ( GeometrySubpassBase && ) = delete;

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

        void AppendDrawcalls ( VkCommandBuffer commandBuffer,
            GeometryPassProgram &program,
            MaterialPool &materialPool,
            UniformBufferPoolManager &uniformPool,
            RenderSessionStats &renderSessionStats
        ) noexcept;
};

} // namespace pbr


#endif // PBR_GEOMETRY_SUBPASS_BASE_H
