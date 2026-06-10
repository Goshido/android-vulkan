#ifndef EDITOR_SKELETAL_MESH_COMPONENT_HPP
#define EDITOR_SKELETAL_MESH_COMPONENT_HPP


#include "component.hpp"


namespace editor {

class SkeletalMeshComponent final : public Component
{
    public:
        constexpr static std::string_view       TYPE = "SkeletalMesh";

    public:
        explicit SkeletalMeshComponent () noexcept;

        SkeletalMeshComponent ( SkeletalMeshComponent const & ) = delete;
        SkeletalMeshComponent &operator = ( SkeletalMeshComponent const & ) = delete;

        SkeletalMeshComponent ( SkeletalMeshComponent && ) = delete;
        SkeletalMeshComponent &operator = ( SkeletalMeshComponent && ) = delete;

        explicit SkeletalMeshComponent ( SaveState::Container const &info ) noexcept;

        ~SkeletalMeshComponent () = default;

    private:
        void Register ( Actor &actor ) noexcept override;
        void Unregister () noexcept override;
        void Save ( SaveState::Container &root ) const noexcept override;
};

} // namespace editor


#endif // EDITOR_SKELETAL_MESH_COMPONENT_HPP
