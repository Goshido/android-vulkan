#ifndef PBR_CAMERA_COMPONENT_H
#define PBR_CAMERA_COMPONENT_H


#include "component.h"
#include "camera_component_desc.h"


namespace pbr {

class CameraComponent final : public Component
{
    private:
        GXMat4          _local {};
        GXMat4          _projection {};
        float           _zNear;
        float           _zFar;
        float           _fieldOfViewRadians;

        static int      _registerCameraComponentIndex;

    public:
        CameraComponent () noexcept;

        CameraComponent ( CameraComponent const & ) = delete;
        CameraComponent& operator = ( CameraComponent const & ) = delete;

        CameraComponent ( CameraComponent && ) = delete;
        CameraComponent& operator = ( CameraComponent && ) = delete;

        explicit CameraComponent ( CameraComponentDesc const &desc, uint8_t const* data ) noexcept;
        explicit CameraComponent ( std::string &&name ) noexcept;

        ~CameraComponent () override = default;

        [[nodiscard]] GXMat4 const& GetLocalMatrix () const noexcept;
        [[nodiscard]] GXMat4 const& GetProjectionMatrix () const noexcept;

        void SetAspectRatio ( float aspectRatio ) noexcept;
        void SetLocal ( GXMat4 const &local ) noexcept;
        void SetProjection ( float fieldOfViewRadians, float aspectRatio, float zNear, float zFar ) noexcept;

        [[nodiscard]] bool Register ( lua_State &vm ) noexcept;

        [[nodiscard]] static bool Init ( lua_State &vm ) noexcept;

    private:
        [[nodiscard]] ComponentRef& GetReference () noexcept override;

        [[nodiscard]] static int OnCreate ( lua_State* state );
        [[nodiscard]] static int OnSetAspectRatio ( lua_State* state );
        [[nodiscard]] static int OnSetLocal ( lua_State* state );
        [[nodiscard]] static int OnSetProjection ( lua_State* state );
};

} // namespace pbr


#endif // PBR_CAMERA_COMPONENT_H
