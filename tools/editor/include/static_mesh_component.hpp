#ifndef EDITOR_STATIC_MESH_COMPONENT_HPP
#define EDITOR_STATIC_MESH_COMPONENT_HPP


#include "component.hpp"
#include "mesh_geometry_ref.hpp"
#include "mesh_node.hpp"


namespace editor {

class StaticMeshComponent final : public Component
{
    private:
        MeshGeometryRef                         _mesh {};
        MeshNode                                _node {};
        PBRMaterial                             _material {};

    public:
        constexpr static std::string_view       TYPE = "StaticMesh";

    public:
        explicit StaticMeshComponent () noexcept;

        StaticMeshComponent ( StaticMeshComponent const & ) = delete;
        StaticMeshComponent &operator = ( StaticMeshComponent const & ) = delete;

        StaticMeshComponent ( StaticMeshComponent && ) = delete;
        StaticMeshComponent &operator = ( StaticMeshComponent && ) = delete;

        explicit StaticMeshComponent ( SaveState::Container const &info ) noexcept;
        explicit StaticMeshComponent ( std::string_view mesh, std::string_view emission ) noexcept;

        ~StaticMeshComponent () noexcept override;

    private:
        void Register ( Actor &actor ) noexcept override;
        void Unregister () noexcept override;
        void ActorTransformChanged () noexcept override;
        void Save ( SaveState::Container &root ) const noexcept override;

        void LoadResources ( std::string_view mesh, std::string_view emission ) noexcept;

        void JoinRendering () noexcept;
        void QuitRendering () noexcept;

        void UpdateTransform () noexcept;
};

} // namespace editor


#endif // EDITOR_STATIC_MESH_COMPONENT_HPP
