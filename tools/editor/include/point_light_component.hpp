#ifndef EDITOR_POINT_LIGHT_COMPONENT_HPP
#define EDITOR_POINT_LIGHT_COMPONENT_HPP


#include "component.hpp"


namespace editor {

class PointLightComponent final : public Component
{
    public:
        constexpr static std::string_view       TYPE = "PointLight";

    public:
        explicit PointLightComponent () noexcept;

        PointLightComponent ( PointLightComponent const & ) = delete;
        PointLightComponent &operator = ( PointLightComponent const & ) = delete;

        PointLightComponent ( PointLightComponent && ) = delete;
        PointLightComponent &operator = ( PointLightComponent && ) = delete;

        explicit PointLightComponent ( SaveState::Container const &info ) noexcept;

        ~PointLightComponent () = default;

    private:
        void Save ( SaveState::Container &root ) const noexcept override;
};

} // namespace editor


#endif // EDITOR_POINT_LIGHT_COMPONENT_HPP
