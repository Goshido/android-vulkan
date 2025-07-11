#ifndef EDITOR_REFLECTION_COMPONENT_HPP
#define EDITOR_REFLECTION_COMPONENT_HPP


#include "component.hpp"


namespace editor {

class ReflectionComponent final : public Component
{
    private:
        static Factory<ReflectionComponent> const       _factory;

    public:
        explicit ReflectionComponent () noexcept;

        ReflectionComponent ( ReflectionComponent const & ) = delete;
        ReflectionComponent &operator = ( ReflectionComponent const & ) = delete;

        ReflectionComponent ( ReflectionComponent && ) = delete;
        ReflectionComponent &operator = ( ReflectionComponent && ) = delete;

        explicit ReflectionComponent ( SaveState::Container const &info ) noexcept;

        ~ReflectionComponent () = default;

    private:
        void Save ( SaveState::Container &root ) const noexcept override;
};

} // namespace editor


#endif // EDITOR_REFLECTION_COMPONENT_HPP
