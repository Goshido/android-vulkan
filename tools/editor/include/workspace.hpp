#ifndef EDITOR_WORKSPACE_HPP
#define EDITOR_WORKSPACE_HPP


#include "actor.hpp"
#include "gizmo_node.hpp"
#include "history.hpp"
#include "mesh_geometry_ref.hpp"
#include "mesh_info.hpp"
#include "mesh_node.hpp"
#include "point_light_node.hpp"
#include <platform/windows/pbr/opaque_program.hpp>
#include <platform/windows/pbr/stream_buffer.hpp>
#include "rect.hpp"
#include "reflection_probe_global_node.hpp"
#include "reflection_probe_local_node.hpp"
#include "viewport_widget.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <deque>
#include <memory>
#include <unordered_map>
#include <vulkan/vulkan_core.h>

GX_RESTORE_WARNING_STATE


namespace editor {

class Workspace final
{
    private:
        constexpr static size_t WORKERS = 4U;

        using Meshes = std::deque<MeshInfo*>;
        using MeshQueue = std::unordered_map<MeshGeometryRef, Meshes>;
        using MeshMap = std::unordered_map<MeshInfo const*, MeshGeometryRef::weak_type>;

        using Gizmos = std::deque<GizmoInfo*>;
        using GizmoQueue = std::unordered_map<MeshGeometryRef, Gizmos>;
        using GizmoMap = std::unordered_map<GizmoInfo const*, MeshGeometryRef::weak_type>;

        using PointLightQueue = std::unordered_set<PointLightInfo*>;
        using ReflectionProbeLocalQueue = std::unordered_set<ReflectionProbeLocalInfo*>;
        using ReflectionProbeGlobalQueue = std::unordered_set<ReflectionProbeGlobalInfo*>;

    private:
        History                                         _history {};
        std::unordered_map<Actor const*, ActorRef>      _actors {};

        MeshQueue                                       _opaqueQueue {};
        MeshMap                                         _opaqueMap {};
        std::vector<size_t>                             _opaqueVisible {};

        MeshQueue                                       _stippleQueue {};
        MeshMap                                         _stippleMap {};
        std::vector<size_t>                             _stippleVisible {};

        GizmoQueue                                      _gizmoQueue {};
        GizmoMap                                        _gizmoMap {};

        PointLightQueue                                 _pointLightQueue {};
        ReflectionProbeLocalQueue                       _reflectionProbeLocalQueue {};
        ReflectionProbeGlobalQueue                      _reflectionProbeGlobalQueue {};

        std::unique_ptr<pbr::OpaqueProgram>             _opaqueProgram {};
        std::unique_ptr<pbr::StreamBuffer>              _frameStream {};
        std::unique_ptr<pbr::StreamBuffer>              _transformStream {};
        std::unique_ptr<pbr::StreamBuffer>              _shadingStream {};

        ViewportWidget*                                 _viewport = nullptr;

        std::mutex                                      _mutex {};
        bool                                            _ready = false;

    public:
        explicit Workspace () = default;

        Workspace ( Workspace const & ) = delete;
        Workspace &operator = ( Workspace const & ) = delete;

        Workspace ( Workspace && ) = delete;
        Workspace &operator = ( Workspace && ) = delete;

        ~Workspace () = default;

        void Init () noexcept;
        void Destroy () noexcept;

        void Load ( std::string_view scene ) noexcept;
        void Close () noexcept;

        void ComputeTransform ( float deltaTime ) noexcept;
        void UploadToGPU ( VkCommandBuffer commandBuffer ) noexcept;

        void DrawOpaque ( VkCommandBuffer commandBuffer ) noexcept;
        void DrawGizmo ( VkCommandBuffer commandBuffer ) noexcept;

        void Pick ( int32_t x, int32_t y, GXMat4 const &viewer, GXMat4 const &projection ) noexcept;
        void Pick ( Rect const &rect, GXMat4 const &viewer, GXMat4 const &projection ) noexcept;

        [[nodiscard]] MeshNode RegisterOpaqueMesh ( MeshGeometryRef &mesh ) noexcept;
        [[nodiscard]] MeshNode RegisterStippleMesh ( MeshGeometryRef &mesh ) noexcept;
        [[nodiscard]] GizmoNode RegisterGizmo ( MeshGeometryRef &mesh ) noexcept;
        [[nodiscard]] PointLightNode RegisterPointLight () noexcept;
        [[nodiscard]] ReflectionProbeLocalNode RegisterReflectionProbeLocal () noexcept;
        [[nodiscard]] ReflectionProbeGlobalNode RegisterReflectionProbeGlobal () noexcept;

        void Unregister ( MeshNode const &node ) noexcept;
        void Unregister ( GizmoNode const &node ) noexcept;
        void Unregister ( PointLightNode &node ) noexcept;
        void Unregister ( ReflectionProbeLocalNode &node ) noexcept;
        void Unregister ( ReflectionProbeGlobalNode &node ) noexcept;

    private:
        void FUCK () noexcept;

        [[nodiscard]] MeshNode RegisterMesh ( MeshGeometryRef &mesh,
            MeshQueue &meshQueue,
            MeshMap &meshMap,
            MeshInfo &nodeMeshInfo
        ) noexcept;

        void UnregisterMesh ( MeshQueue &meshQueue, MeshMap &meshMap, MeshInfo const &nodeMeshInfo ) noexcept;
};

} // namespace editor


#endif // EDITOR_WORKSPACE_HPP
