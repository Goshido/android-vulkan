#ifndef ANDROID_VULKAN_RIGID_BODY_H
#define ANDROID_VULKAN_RIGID_BODY_H


#include "shape.h"

GX_DISABLE_COMMON_WARNINGS

#include <mutex>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

class Physics;

class RigidBody final
{
    private:
        float                   _dampingAngular;
        float                   _dampingLinear;

        GXMat3                  _inertiaTensorInverse;

        bool                    _isAwake;
        bool                    _isCanSleep;
        bool                    _isKinematic;

        GXVec3                  _location;
        GXVec3                  _locationBefore;

        float                   _mass;
        float                   _massInverse;

        Physics*                _physics;

        GXQuat                  _rotation;
        GXQuat                  _rotationBefore;

        ShapeRef                _shape;
        float                   _sleepTimeout;

        GXVec3                  _totalForce;
        GXVec3                  _totalTorque;

        // This feature is primary for debugging purposes.
        std::string             _tag;

        GXMat4                  _transform;

        GXVec3                  _velocityAngular;
        GXVec3                  _velocityLinear;

        static std::mutex       _mutex;

    public:
        RigidBody () noexcept;

        RigidBody ( RigidBody const & ) = delete;
        RigidBody& operator = ( RigidBody const & ) = delete;

        RigidBody ( RigidBody && ) = delete;
        RigidBody& operator = ( RigidBody && ) = delete;

        ~RigidBody () = default;

        [[maybe_unused]] void AddVelocityAngular ( GXVec3 const &velocity ) noexcept;
        [[nodiscard]] GXVec3 const& GetVelocityAngular () const noexcept;
        [[maybe_unused]] void SetVelocityAngular ( GXVec3 const &velocity ) noexcept;
        void SetVelocityAngular ( float wx, float wy, float wz ) noexcept;

        [[maybe_unused]] void AddVelocityLinear ( GXVec3 const &velocity ) noexcept;
        [[nodiscard]] GXVec3 const& GetVelocityLinear () const noexcept;
        [[maybe_unused]] void SetVelocityLinear ( GXVec3 const &velocity ) noexcept;
        void SetVelocityLinear ( float x, float y, float z ) noexcept;

        [[maybe_unused]] void AddForce ( GXVec3 const &force, GXVec3 const &point ) noexcept;
        [[maybe_unused]] void AddImpulse ( GXVec3 const &impulse, GXVec3 const &point ) noexcept;

        [[maybe_unused]] void DisableKinematic () noexcept;
        [[maybe_unused]] void EnableKinematic () noexcept;
        [[maybe_unused, nodiscard]] bool IsKinematic () const noexcept;

        [[maybe_unused]] void DisableSleep () noexcept;
        [[maybe_unused]] void EnableSleep () noexcept;
        [[maybe_unused, nodiscard]] bool IsCanSleep () const noexcept;

        [[maybe_unused, nodiscard]] float GetDampingAngular () const noexcept;
        [[maybe_unused]] void SetDampingAngular ( float damping ) noexcept;

        [[maybe_unused, nodiscard]] float GetDampingLinear () const noexcept;
        [[maybe_unused]] void SetDampingLinear ( float damping ) noexcept;

        [[nodiscard]] GXMat3 const& GetInertiaTensorInverse () const noexcept;

        [[nodiscard]] GXVec3 const& GetLocation () const noexcept;
        [[maybe_unused]] void SetLocation ( GXVec3 const &location ) noexcept;
        [[maybe_unused]] void SetLocation ( float x, float y, float z ) noexcept;

        [[maybe_unused, nodiscard]] float GetMass () const noexcept;
        [[nodiscard]] float GetMassInverse () const noexcept;
        [[maybe_unused]] void SetMass ( float mass ) noexcept;

        [[maybe_unused, nodiscard]] GXQuat const& GetRotation () const noexcept;
        [[maybe_unused]] void SetRotation ( GXQuat const &rotation ) noexcept;

        [[nodiscard]] Shape& GetShape () noexcept;
        [[nodiscard]] bool HasShape () const noexcept;
        [[maybe_unused]] void SetShape ( ShapeRef &shape ) noexcept;

        // This feature is primary for debugging purposes.
        [[maybe_unused, nodiscard]] std::string const& GetTag () const noexcept;
        [[maybe_unused]] void SetTag ( std::string &&tag ) noexcept;

        [[maybe_unused, nodiscard]] GXVec3 const& GetTotalForce () const noexcept;
        [[maybe_unused, nodiscard]] GXVec3 const& GetTotalTorque () const noexcept;

        [[maybe_unused, nodiscard]] GXMat4 const& GetTransform () const noexcept;

        void Integrate ( float deltaTime ) noexcept;
        [[maybe_unused, nodiscard]] bool IsAwake () const noexcept;

        void OnRegister ( Physics &physics ) noexcept;
        void OnUnregister () noexcept;

        void ResetAccumulators () noexcept;
        void UpdatePositionAndRotation ( float deltaTime ) noexcept;

    private:
        void RunSleepLogic ( float deltaTime ) noexcept;

        void SetAwake () noexcept;
        void SetSleep () noexcept;

        void UpdateCacheData () noexcept;
};

using RigidBodyRef = std::shared_ptr<RigidBody>;

} // namespace android_vulkan


#endif // ANDROID_VULKAN_RIGID_BODY_H
