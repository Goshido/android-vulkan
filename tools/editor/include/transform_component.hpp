#ifndef EDITOR_TRANSFORM_COMPONENT_HPP
#define EDITOR_TRANSFORM_COMPONENT_HPP


#include "component.hpp"


namespace editor {

class TransformComponent final : public Component
{
    public:
        constexpr static std::string_view       TYPE = "Transform";

    public:
        explicit TransformComponent () noexcept;

        TransformComponent ( TransformComponent const & ) = delete;
        TransformComponent &operator = ( TransformComponent const & ) = delete;

        TransformComponent ( TransformComponent && ) = delete;
        TransformComponent &operator = ( TransformComponent && ) = delete;

        explicit TransformComponent ( SaveState::Container const &info ) noexcept;

        ~TransformComponent () = default;

    private:
        void Save ( SaveState::Container &root ) const noexcept override;
};

} // namespace editor


#endif // EDITOR_TRANSFORM_COMPONENT_HPP
