#ifndef EDITOR_WORKSPACE_HPP
#define EDITOR_WORKSPACE_HPP


#include "gizmo_node.hpp"
#include "history.hpp"
#include "mesh_geometry_ref.hpp"
#include "mesh_info.hpp"
#include "mesh_node.hpp"
#include <platform/windows/pbr/opaque_program.hpp>
#include <platform/windows/pbr/opaque_with_id_program.hpp>
#include "point_light_node.hpp"
#include "reflection_probe_global_node.hpp"
#include "reflection_probe_local_node.hpp"
#include "selection.hpp"
#include "stream_buffer_ref.hpp"
#include "viewport_widget.hpp"


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
            android_vulkan::MeshGeometry*                   _mesh = nullptr;
            uint32_t                                        _count = 0U;
        };

    private:
        History                                             _history {};
        Selection                                           _selection {};
        std::unordered_map<Actor const*, ActorRef>          _actors {};

        MeshQueue                                           _opaqueQueue {};
        MeshMap                                             _opaqueMap {};
        std::vector<MeshInstance>                           _opaqueVisible {};

        MeshQueue                                           _stippleQueue {};
        MeshMap                                             _stippleMap {};
        std::vector<MeshInstance>                           _stippleVisible {};

        GizmoQueue                                          _gizmoQueue {};
        GizmoMap                                            _gizmoMap {};

        PointLightQueue                                     _pointLightQueue {};
        ReflectionProbeLocalQueue                           _reflectionProbeLocalQueue {};
        ReflectionProbeGlobalQueue                          _reflectionProbeGlobalQueue {};

        std::unique_ptr<pbr::OpaqueProgram>                 _opaqueProgram {};
        std::unique_ptr<pbr::OpaqueWithIDProgram>           _opaqueWithIDProgram {};

        StreamBufferRef                                     _frameStream {};
        StreamBufferRef                                     _transformStream {};
        StreamBufferRef                                     _shadingStream {};
        StreamBufferRef                                     _idStream {};

        Texture2DRef                                        _defaultAlbedo {};
        Texture2DRef                                        _defaultEmission {};
        Texture2DRef                                        _defaultMask {};
        Texture2DRef                                        _defaultParam {};
        Texture2DRef                                        _defaultNormal {};

        ViewportWidget*                                     _viewport = nullptr;
        std::mutex                                          _mutex {};

        Hotkey                                              _delete {};
        Hotkey                                              _openWorkspace {};
        Hotkey                                              _saveWorkspace {};
        Hotkey                                              _saveAsWorkspace {};

        bool                                                _ready = false;

        static Workspace*                                   _instance;

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

        void UploadGPUData ( VkCommandBuffer commandBuffer, float deltaTime ) noexcept;
        void PrepareIDBuffer ( VkCommandBuffer commandBuffer ) noexcept;
        void FillGBuffer ( VkCommandBuffer commandBuffer ) noexcept;
        void DrawGizmo ( VkCommandBuffer commandBuffer ) noexcept;
        void OnGBufferResolutionChanged ( android_vulkan::Texture2D &idImage, uint32_t idResourceIdx ) noexcept;

        [[nodiscard]] bool IsSelectionRequested () const noexcept;
        [[nodiscard]] bool HasSelection () const noexcept;
        void Select ( Rect const &rect, bool invert ) noexcept;
        void ComputeSelect ( VkCommandBuffer commandBuffer ) noexcept;

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

        void ComputeTransformGBufferOnly ( GXProjectionClipPlanes const &frustum ) noexcept;
        void ComputeTransformGBufferWithID ( GXProjectionClipPlanes const &frustum ) noexcept;

        void FillGBufferOnly ( VkCommandBuffer commandBuffer ) noexcept;
        void FillGBufferWithID ( VkCommandBuffer commandBuffer ) noexcept;

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
