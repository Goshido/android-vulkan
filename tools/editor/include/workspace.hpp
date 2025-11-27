#ifndef EDITOR_WORKSPACE_HPP
#define EDITOR_WORKSPACE_HPP


#include <GXCommon/GXMath.hpp>
#include "mesh_geometry_ref.hpp"
#include "message_queue.hpp"
#include "opaque_mesh_node.hpp"
#include "rect.hpp"
#include <vulkan/vulkan_core.h>


namespace editor {

class Workspace final
{
    private:
        constexpr static size_t WORKERS = 4U;

    private:
        MessageQueue    &_messageQueue;

    public:
        Workspace () = delete;

        Workspace ( Workspace const & ) = delete;
        Workspace &operator = ( Workspace const & ) = delete;

        Workspace ( Workspace && ) = delete;
        Workspace &operator = ( Workspace && ) = delete;

        explicit Workspace ( MessageQueue &messageQueue ) noexcept;

        ~Workspace () = default;

        void Init () noexcept;
        void Destroy () noexcept;

        void Draw ( VkCommandBuffer commandBuffer ) noexcept;
        void Pick ( int32_t x, int32_t y, GXMat4 const &viewer, GXMat4 const &projection ) noexcept;
        void Pick ( Rect const &rect, GXMat4 const &viewer, GXMat4 const &projection ) noexcept;

        [[nodiscard]] OpaqueMeshNode RegisterOpaqueMesh ( MeshGeometryRef &mesh ) noexcept;
        void Unregister ( OpaqueMeshNode &node ) noexcept;

    private:
        void DrawOpaque ( VkCommandBuffer commandBuffer );
        void DrawGizmo ( VkCommandBuffer commandBuffer );
        void DrawUI ( VkCommandBuffer commandBuffer );
};

} // namespace editor


#endif // EDITOR_WORKSPACE_HPP
