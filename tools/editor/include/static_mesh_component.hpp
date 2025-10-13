#ifndef EDITOR_STATIC_MESH_COMPONENT_HPP
#define EDITOR_STATIC_MESH_COMPONENT_HPP


#include "component.hpp"


namespace editor {

class StaticMeshComponent final : public Component
{
    public:
        constexpr static std::string_view       TYPE = "StaticMesh";

    public:
        explicit StaticMeshComponent () noexcept;

        StaticMeshComponent ( StaticMeshComponent const & ) = delete;
        StaticMeshComponent &operator = ( StaticMeshComponent const & ) = delete;

        StaticMeshComponent ( StaticMeshComponent && ) = delete;
        StaticMeshComponent &operator = ( StaticMeshComponent && ) = delete;

        explicit StaticMeshComponent ( SaveState::Container const &info ) noexcept;

        ~StaticMeshComponent () = default;

    private:
        void Save ( SaveState::Container &root ) const noexcept override;
};

} // namespace editor


#endif // EDITOR_STATIC_MESH_COMPONENT_HPP
