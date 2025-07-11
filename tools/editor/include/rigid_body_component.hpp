#ifndef EDITOR_RIGID_BODY_COMPONENT_HPP
#define EDITOR_RIGID_BODY_COMPONENT_HPP


#include "component.hpp"


namespace editor {

class RigidBodyComponent final : public Component
{
    private:
        static Factory<RigidBodyComponent> const    _factory;

    public:
        explicit RigidBodyComponent () noexcept;

        RigidBodyComponent ( RigidBodyComponent const & ) = delete;
        RigidBodyComponent &operator = ( RigidBodyComponent const & ) = delete;

        RigidBodyComponent ( RigidBodyComponent && ) = delete;
        RigidBodyComponent &operator = ( RigidBodyComponent && ) = delete;

        explicit RigidBodyComponent ( SaveState::Container const &info ) noexcept;

        ~RigidBodyComponent () = default;

    private:
        void Save ( SaveState::Container &root ) const noexcept override;
};

} // namespace editor


#endif // EDITOR_RIGID_BODY_COMPONENT_HPP
