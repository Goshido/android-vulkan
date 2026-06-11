#ifndef EDITOR_WORKSPACE_HPP
#define EDITOR_WORKSPACE_HPP


#include "actor.hpp"
#include "gizmo_node.hpp"
#include "history.hpp"
#include "hotkey.hpp"
#include "mesh_geometry_ref.hpp"
#include "mesh_info.hpp"
#include "mesh_node.hpp"
#include "point_light_node.hpp"
#include <platform/windows/pbr/opaque_program.hpp>
#include "rect.hpp"
#include "reflection_probe_global_node.hpp"
#include "reflection_probe_local_node.hpp"
#include "stream_buffer_ref.hpp"
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

        struct MeshInstance final
        {
            android_vulkan::MeshGeometry*               _mesh = nullptr;
            uint32_t                                    _count = 0U;
        };

    private:
        History                                         _history {};
        std::unordered_map<Actor const*, ActorRef>      _actors {};

        MeshQueue                                       _opaqueQueue {};
        MeshMap                                         _opaqueMap {};
        std::vector<MeshInstance>                       _opaqueVisible {};

        MeshQueue                                       _stippleQueue {};
        MeshMap                                         _stippleMap {};
        std::vector<MeshInstance>                       _stippleVisible {};

        GizmoQueue                                      _gizmoQueue {};
        GizmoMap                                        _gizmoMap {};

        PointLightQueue                                 _pointLightQueue {};
        ReflectionProbeLocalQueue                       _reflectionProbeLocalQueue {};
        ReflectionProbeGlobalQueue                      _reflectionProbeGlobalQueue {};

        std::unique_ptr<pbr::OpaqueProgram>             _opaqueProgram {};
        StreamBufferRef                                 _frameStream {};
        StreamBufferRef                                 _transformStream {};
        StreamBufferRef                                 _shadingStream {};

        Texture2DRef                                    _defaultAlbedo {};
        Texture2DRef                                    _defaultEmission {};
        Texture2DRef                                    _defaultMask {};
        Texture2DRef                                    _defaultParam {};
        Texture2DRef                                    _defaultNormal {};

        ViewportWidget*                                 _viewport = nullptr;
        std::mutex                                      _mutex {};

        Hotkey                                          _useSelectTool {};
        Hotkey                                          _useMoveTool {};
        Hotkey                                          _useRotateTool {};
        Hotkey                                          _useScaleTool {};

        Hotkey                                          _saveWorkspace {};
        Hotkey                                          _saveAsWorkspace {};

        bool                                            _ready = false;

        static Workspace*                               _instance;

    public:
        explicit Workspace () noexcept;

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

        void FillGBuffer ( VkCommandBuffer commandBuffer ) noexcept;
        void DrawGizmo ( VkCommandBuffer commandBuffer ) noexcept;

        void Pick ( int32_t x, int32_t y, GXMat4 const &viewer, GXMat4 const &projection ) noexcept;
        void Pick ( Rect const &rect, GXMat4 const &viewer, GXMat4 const &projection ) noexcept;

        [[nodiscard]] MeshNode RegisterOpaqueMesh ( MeshGeometryRef &mesh ) noexcept;
        [[nodiscard]] MeshNode RegisterStippleMesh ( MeshGeometryRef &mesh ) noexcept;
        [[nodiscard]] GizmoNode RegisterGizmo ( MeshGeometryRef &mesh ) noexcept;
        [[nodiscard]] PointLightNode RegisterPointLight () noexcept;
        [[nodiscard]] ReflectionProbeLocalNode RegisterReflectionProbeLocal () noexcept;
        [[nodiscard]] ReflectionProbeGlobalNode RegisterReflectionProbeGlobal () noexcept;

        void Unregister ( MeshNode &node ) noexcept;
        void Unregister ( GizmoNode const &node ) noexcept;
        void Unregister ( PointLightNode &node ) noexcept;
        void Unregister ( ReflectionProbeLocalNode &node ) noexcept;
        void Unregister ( ReflectionProbeGlobalNode &node ) noexcept;

        [[nodiscard]] static Workspace &Instance () noexcept;

    private:
        void FUCK () noexcept;
        [[nodiscard]] bool IsReady () noexcept;

        void InitGraphicsResources () noexcept;
        void InitHotkeys () noexcept;
        void InitWidgets () noexcept;

        [[nodiscard]] MeshNode RegisterMesh ( MeshGeometryRef &mesh,
            MeshQueue &meshQueue,
            MeshMap &meshMap,
            MeshInfo &nodeMeshInfo
        ) noexcept;

        void UnregisterMesh ( MeshQueue &meshQueue, MeshMap &meshMap, MeshInfo &nodeMeshInfo ) noexcept;
};

} // namespace editor


#endif // EDITOR_WORKSPACE_HPP
