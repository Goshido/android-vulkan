#ifndef EDITOR_WORKSPACE_HPP
#define EDITOR_WORKSPACE_HPP


#include <deque>
#include <GXCommon/GXMath.hpp>
#include "mesh_geometry_ref.hpp"
#include "mesh_info.hpp"
#include "message_queue.hpp"
#include "mesh_node.hpp"
#include "point_light_node.hpp"
#include "rect.hpp"
#include <unordered_map>
#include <vulkan/vulkan_core.h>


namespace editor {

class Workspace final
{
    private:
        constexpr static size_t WORKERS = 4U;

        using Meshes = std::deque<MeshInfo*>;
        using MeshQueue = std::unordered_map<MeshGeometryRef, Meshes>;
        using MeshMap = std::unordered_map<MeshInfo const*, MeshGeometryRef::weak_type>;
        using PointLightQueue = std::unordered_set<PointLightInfo*>;

    private:
        MessageQueue        &_messageQueue;

        MeshQueue           _opaqueQueue {};
        MeshMap             _opaqueMap {};

        MeshQueue           _stippleQueue {};
        MeshMap             _stippleMap {};

        PointLightQueue     _pointLightQueue {};

        std::mutex          _mutex {};

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

        [[nodiscard]] MeshNode RegisterOpaqueMesh ( MeshGeometryRef &mesh ) noexcept;
        [[nodiscard]] MeshNode RegisterStippleMesh ( MeshGeometryRef &mesh ) noexcept;
        [[nodiscard]] PointLightNode RegisterPointLight () noexcept;

        void Unregister ( MeshNode const &node ) noexcept;
        void Unregister ( PointLightNode &node ) noexcept;

    private:
        void DrawOpaque ( VkCommandBuffer commandBuffer );
        void DrawGizmo ( VkCommandBuffer commandBuffer );
        void DrawUI ( VkCommandBuffer commandBuffer );

        [[nodiscard]] MeshNode Register ( MeshGeometryRef &mesh,
            MeshQueue &meshQueue,
            MeshMap &meshMap,
            MeshInfo &node
        ) noexcept;

        void Unregister ( MeshQueue &meshQueue, MeshMap &meshMap, MeshInfo const &node ) noexcept;
};

} // namespace editor


#endif // EDITOR_WORKSPACE_HPP
