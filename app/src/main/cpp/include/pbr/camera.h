#ifndef PBR_CAMERA_H
#define PBR_CAMERA_H


#include <GXCommon/GXMath.h>


namespace pbr {

class Camera final
{
    private:
        GXVec3      _location;
        float       _moveBoost;
        float       _pitch;
        float       _yaw;

        // move space is perpendicular to the UP viewer direction.
        // x is forward
        // y is strafe
        // z is altitude
        GXVec3      _moveSpeed;

        // x is yaw
        // y is pitch
        GXVec2      _angularSpeed;

        GXMat4      _projectionMatrix;
        GXMat4      _viewMatrix;

    public:
        Camera ();

        Camera ( Camera const &other ) = delete;
        Camera& operator = ( Camera const &other ) = delete;

        Camera ( Camera &&other ) = delete;
        Camera& operator = ( Camera &&other ) = delete;

        ~Camera () = default;

        void SetProjection ( float fieldOfViewRadians, float aspectRatio, float zNear, float zFar );
        [[maybe_unused]] void SetLocation ( float x, float y, float z );
        void SetLocation ( GXVec3 const &location );

        void Update ( float deltaTime );

        [[nodiscard]] GXMat4 const& GetProjectionMatrix () const;
        [[nodiscard]] GXMat4 const& GetViewMatrix () const;

        void CaptureInput ();
        static void ReleaseInput ();

    private:
        static void OnADown ( void* context );
        static void OnAUp ( void* context );
        static void OnXDown ( void* context );
        static void OnXUp ( void* context );
        static void OnLeftStick ( void* context, float horizontal, float vertical );
        static void OnRightStick ( void* context, float horizontal, float vertical );
        static void OnRightTrigger ( void* context, float push );
};

} // namespace pbr


#endif // PBR_CAMERA_H
