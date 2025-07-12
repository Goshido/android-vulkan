#ifndef EDITOR_CAMERA_COMPONENT_HPP
#define EDITOR_CAMERA_COMPONENT_HPP


#include "component.hpp"


namespace editor {

class CameraComponent final : public Component
{
    public:
        constexpr static std::string_view       TYPE = "Camera";

    public:
        explicit CameraComponent () noexcept;

        CameraComponent ( CameraComponent const & ) = delete;
        CameraComponent &operator = ( CameraComponent const & ) = delete;

        CameraComponent ( CameraComponent && ) = delete;
        CameraComponent &operator = ( CameraComponent && ) = delete;

        explicit CameraComponent ( SaveState::Container const &info ) noexcept;

        ~CameraComponent () = default;

    private:
        void Save ( SaveState::Container &root ) const noexcept override;
};

} // namespace editor


#endif // EDITOR_CAMERA_COMPONENT_HPP
