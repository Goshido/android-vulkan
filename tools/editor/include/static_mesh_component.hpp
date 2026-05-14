#ifndef EDITOR_STATIC_MESH_COMPONENT_HPP
#define EDITOR_STATIC_MESH_COMPONENT_HPP


#include "component.hpp"
#include "mesh_geometry_ref.hpp"


namespace editor {

class StaticMeshComponent final : public Component
{
    private:
        MeshGeometryRef                         _mesh {};

    public:
        constexpr static std::string_view       TYPE = "StaticMesh";

    public:
        explicit StaticMeshComponent () noexcept;

        StaticMeshComponent ( StaticMeshComponent const & ) = delete;
        StaticMeshComponent &operator = ( StaticMeshComponent const & ) = delete;

        StaticMeshComponent ( StaticMeshComponent && ) = delete;
        StaticMeshComponent &operator = ( StaticMeshComponent && ) = delete;

        explicit StaticMeshComponent ( MessageQueue &messageQueue, SaveState::Container const &info ) noexcept;
        explicit StaticMeshComponent ( MessageQueue &messageQueue, std::string_view mesh ) noexcept;

        ~StaticMeshComponent () = default;

    private:
        void Register () noexcept override;
        void Unregister () noexcept override;
        void Save ( SaveState::Container &root ) const noexcept override;

        void LoadMesh ( std::string_view mesh ) noexcept;
};

} // namespace editor


#endif // EDITOR_STATIC_MESH_COMPONENT_HPP
