#ifndef PBR_CAMERA_HPP
#define PBR_CAMERA_HPP


#include <GXCommon/GXMath.hpp>


namespace pbr {

class Camera final
{
    private:
        float       _moveBoost;
        float       _movingSpeed;

        // range [-pi / 2, pi / 2]
        float       _pitch;

        // range [-pi, pi]
        float       _yaw;

        // move space is perpendicular to the UP viewer direction.
        // x is forward
        // y is strafe
        // z is altitude
        GXVec3      _moveSpeed;

        // x is yaw
        // y is pitch
        GXVec2      _angularSpeed;

        GXMat4      _local;
        GXMat4      _projection;

    public:
        Camera () noexcept;

        Camera ( Camera const &other ) = delete;
        Camera &operator = ( Camera const &other ) = delete;

        Camera ( Camera &&other ) = delete;
        Camera &operator = ( Camera &&other ) = delete;

        ~Camera () = default;

        void CaptureInput () noexcept;
        void ReleaseInput () noexcept;

        [[nodiscard]] GXMat4 const &GetLocalMatrix () const noexcept;
        [[nodiscard]] GXMat4 const &GetProjectionMatrix () const noexcept;

        void SetLocation ( GXVec3 const &location ) noexcept;

        // Render units per second.
        void SetMovingSpeed ( float speed ) noexcept;

        void SetProjection ( float fieldOfViewRadians, float aspectRatio, float zNear, float zFar ) noexcept;

        // Note "pitch" and "yaw" must be in radians.
        void SetRotation ( float pitch, float yaw ) noexcept;

        void Update ( float deltaTime ) noexcept;

    private:
        static void OnADown ( void* context ) noexcept;
        static void OnAUp ( void* context ) noexcept;
        static void OnXDown ( void* context ) noexcept;
        static void OnXUp ( void* context ) noexcept;
        static void OnLeftStick ( void* context, float horizontal, float vertical ) noexcept;
        static void OnRightStick ( void* context, float horizontal, float vertical ) noexcept;
        static void OnRightTrigger ( void* context, float push ) noexcept;
};

} // namespace pbr


#endif // PBR_CAMERA_HPP
