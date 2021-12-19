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
    public:
        using Context = void*;

    private:
        Context                 _context;

        float                   _dampingAngular;
        float                   _dampingLinear;

        bool                    _forceAwake;
        GXMat3                  _inertiaTensorInverse;

        bool                    _isAwake;
        bool                    _isCanSleep;
        bool                    _isKinematic;

        GXVec3                  _location;

        float                   _mass;
        float                   _massInverse;

        Physics*                _physics;
        GXQuat                  _rotation;

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

        [[maybe_unused]] void AddVelocityAngular ( GXVec3 const &velocity, bool forceAwake ) noexcept;
        [[nodiscard]] GXVec3 const& GetVelocityAngular () const noexcept;
        [[maybe_unused]] void SetVelocityAngular ( GXVec3 const &velocity, bool forceAwake ) noexcept;
        [[maybe_unused]] void SetVelocityAngular ( float wx, float wy, float wz, bool forceAwake ) noexcept;

        [[maybe_unused]] void AddVelocityLinear ( GXVec3 const &velocity, bool forceAwake ) noexcept;
        [[nodiscard]] GXVec3 const& GetVelocityLinear () const noexcept;
        void SetVelocityLinear ( GXVec3 const &velocity, bool forceAwake ) noexcept;
        [[maybe_unused]] void SetVelocityLinear ( float x, float y, float z, bool forceAwake ) noexcept;

        // Methods operates linear and angular velocities in one transaction. The layout:
        //      GXVec6::_data[ 0U ] - x component of the linear velocity
        //      GXVec6::_data[ 1U ] - y component of the linear velocity
        //      GXVec6::_data[ 2U ] - z component of the linear velocity
        //      GXVec6::_data[ 3U ] - x component of the angular velocity
        //      GXVec6::_data[ 4U ] - y component of the angular velocity
        //      GXVec6::_data[ 5U ] - z component of the angular velocity
        void SetVelocities ( GXVec6 const &velocities ) noexcept;
        [[nodiscard]] GXVec6 GetVelocities () const noexcept;

        void AddForce ( GXVec3 const &force, GXVec3 const &point, bool forceAwake ) noexcept;
        [[maybe_unused]] void AddImpulse ( GXVec3 const &impulse, GXVec3 const &point, bool forceAwake ) noexcept;

        void DisableKinematic ( bool forceAwake ) noexcept;
        void EnableKinematic () noexcept;
        [[nodiscard]] bool IsKinematic () const noexcept;

        [[maybe_unused]] void DisableSleep () noexcept;
        void EnableSleep () noexcept;
        [[maybe_unused, nodiscard]] bool IsCanSleep () const noexcept;

        [[nodiscard]] Context GetContext () const noexcept;
        void SetContext ( Context context ) noexcept;

        [[maybe_unused, nodiscard]] float GetDampingAngular () const noexcept;
        void SetDampingAngular ( float damping ) noexcept;

        [[maybe_unused, nodiscard]] float GetDampingLinear () const noexcept;
        void SetDampingLinear ( float damping ) noexcept;

        [[nodiscard]] GXMat3 const& GetInertiaTensorInverse () const noexcept;

        [[nodiscard]] GXVec3 const& GetLocation () const noexcept;
        void SetLocation ( GXVec3 const &location, bool forceAwake ) noexcept;
        void SetLocation ( float x, float y, float z, bool forceAwake ) noexcept;

        [[nodiscard]] float GetMass () const noexcept;
        [[nodiscard]] float GetMassInverse () const noexcept;
        void SetMass ( float mass, bool forceAwake ) noexcept;

        [[nodiscard]] GXQuat const& GetRotation () const noexcept;
        void SetRotation ( GXQuat const &rotation, bool forceAwake ) noexcept;

        [[nodiscard]] Shape& GetShape () noexcept;
        [[nodiscard]] bool HasShape () const noexcept;
        void SetShape ( ShapeRef &shape, bool forceAwake ) noexcept;

        // This feature is primary for debugging purposes.
        [[maybe_unused, nodiscard]] std::string const& GetTag () const noexcept;
        void SetTag ( std::string &&tag ) noexcept;

        [[maybe_unused, nodiscard]] GXVec3 const& GetTotalForce () const noexcept;
        [[maybe_unused, nodiscard]] GXVec3 const& GetTotalTorque () const noexcept;

        [[nodiscard]] GXMat4 const& GetTransform () const noexcept;

        void Integrate ( float deltaTime ) noexcept;

        [[maybe_unused, nodiscard]] bool IsAwake () const noexcept;
        void SetAwake () noexcept;

        void OnRegister ( Physics &physics ) noexcept;
        void OnUnregister () noexcept;

        void UpdatePositionAndRotation ( float deltaTime ) noexcept;

    private:
        void ResetAccumulators () noexcept;
        void RunSleepLogic ( float deltaTime ) noexcept;
        void UpdateCacheData () noexcept;
};

using RigidBodyRef = std::shared_ptr<RigidBody>;

} // namespace android_vulkan


#endif // ANDROID_VULKAN_RIGID_BODY_H
