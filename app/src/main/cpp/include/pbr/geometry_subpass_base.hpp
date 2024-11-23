#ifndef PBR_GEOMETRY_SUBPASS_BASE_HPP
#define PBR_GEOMETRY_SUBPASS_BASE_HPP


#include "geometry_pool.hpp"
#include "material_pool.hpp"
#include "render_session_stats.hpp"
#include "scene_data.hpp"
#include "uniform_buffer_pool_manager.hpp"


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
            GeometryPassProgram::ColorData const &colorData
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
            GeometryPool &geometryPool,
            MaterialPool &materialPool,
            RenderSessionStats &renderSessionStats
        ) noexcept;
};

} // namespace pbr


#endif // PBR_GEOMETRY_SUBPASS_BASE_HPP
