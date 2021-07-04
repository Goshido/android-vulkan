#ifndef ANDROID_VULKAN_RIGID_BODY_H
#define ANDROID_VULKAN_RIGID_BODY_H


#include "shape.h"


namespace android_vulkan {

class RigidBody final
{
    private:
        float       _dampingAngular;
        float       _dampingLinear;

        // TODO move this to the shape properties.
        float       _friction;

        GXMat3      _inertiaTensorInverse;

        bool        _isAwake;
        bool        _isCanSleep;
        bool        _isKinematic;

        GXVec3      _location;
        GXVec3      _locationBefore;

        float       _mass;
        float       _massInverse;

        float       _restitution;

        GXQuat      _rotation;
        GXQuat      _rotationBefore;

        ShapeRef    _shape;
        float       _sleepTimeout;

        GXVec3      _totalForce;
        GXVec3      _totalTorque;

        GXMat4      _transform;

        GXVec3      _velocityAngular;
        GXVec3      _velocityLinear;

    public:
        RigidBody () noexcept;

        RigidBody ( RigidBody const & ) = delete;
        RigidBody& operator = ( RigidBody const & ) = delete;

        RigidBody ( RigidBody && ) = delete;
        RigidBody& operator = ( RigidBody && ) = delete;

        ~RigidBody () = default;

        [[maybe_unused]] void AddVelocityAngular ( GXVec3 const &velocity ) noexcept;
        [[maybe_unused, nodiscard]] GXVec3 const& GetVelocityAngular () const noexcept;
        [[maybe_unused]] void SetVelocityAngular ( GXVec3 const &velocity ) noexcept;
        void SetVelocityAngular ( float wx, float wy, float wz ) noexcept;

        [[maybe_unused]] void AddVelocityLinear ( GXVec3 const &velocity ) noexcept;
        [[maybe_unused, nodiscard]] GXVec3 const& GetVelocityLinear () const noexcept;
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

        [[nodiscard]] float GetFriction () const noexcept;
        [[maybe_unused]] void SetFriction ( float friction ) noexcept;

        [[maybe_unused, nodiscard]] GXMat3 const& GetInertiaTensorInverse () const noexcept;

        [[nodiscard]] GXVec3 const& GetLocation () const noexcept;
        [[maybe_unused]] void SetLocation ( GXVec3 const &location ) noexcept;
        [[maybe_unused]] void SetLocation ( float x, float y, float z ) noexcept;

        [[maybe_unused, nodiscard]] float GetMass () const noexcept;
        [[nodiscard]] float GetMassInverse () const noexcept;
        [[maybe_unused]] void SetMass ( float mass ) noexcept;

        [[nodiscard]] float GetRestitution () const noexcept;
        [[maybe_unused]] void SetRestitution ( float restitution ) noexcept;

        [[maybe_unused, nodiscard]] GXQuat const& GetRotation () const noexcept;
        [[maybe_unused]] void SetRotation ( GXQuat const &rotation ) noexcept;

        [[nodiscard]] Shape& GetShape () noexcept;
        [[nodiscard]] bool HasShape () const noexcept;
        [[maybe_unused]] void SetShape ( ShapeRef &shape ) noexcept;

        [[maybe_unused, nodiscard]] GXVec3 const& GetTotalForce () const noexcept;
        [[maybe_unused, nodiscard]] GXVec3 const& GetTotalTorque () const noexcept;

        [[maybe_unused, nodiscard]] GXMat4 const& GetTransform () const noexcept;

        void Integrate ( float deltaTime ) noexcept;
        [[maybe_unused, nodiscard]] bool IsAwake () const noexcept;
        void ResetAccumulators ();

    private:
        void IntegrateAsDynamic ( float deltaTime ) noexcept;
        void IntegrateAsKinematic ( float deltaTime ) noexcept;

        void RunSleepLogic ( float deltaTime ) noexcept;

        void SetAwake () noexcept;
        void SetSleep () noexcept;
        void UpdateCacheData () noexcept;
};

using RigidBodyRef = std::shared_ptr<RigidBody>;

} // namespace android_vulkan


#endif // ANDROID_VULKAN_RIGID_BODY_H
