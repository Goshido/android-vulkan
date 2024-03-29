#ifndef ANDROID_VULKAN_CONTACT_DETECTOR_HPP
#define ANDROID_VULKAN_CONTACT_DETECTOR_HPP


#include "contact_manager.hpp"
#include "cyrus_beck.hpp"
#include "epa.hpp"
#include "gjk.hpp"
#include "sutherland_hodgman.hpp"


namespace android_vulkan {

class ContactDetector final
{
    private:
        using FirstContactData = std::pair<ContactManifold*, Contact*>;

    private:
        CyrusBeck               _cyrusBeck;
        EPA                     _epa;
        GJK                     _gjk;
        Vertices                _rays;
        Vertices                _shapeAPoints;
        Vertices                _shapeBPoints;
        SutherlandHodgman       _sutherlandHodgman;

    public:
        ContactDetector () noexcept;

        ContactDetector ( ContactDetector const & ) = delete;
        ContactDetector &operator = ( ContactDetector const & ) = delete;

        ContactDetector ( ContactDetector && ) = delete;
        ContactDetector &operator = ( ContactDetector && ) = delete;

        ~ContactDetector () = default;

        void Check ( ContactManager &contactManager, RigidBodyRef const &a, RigidBodyRef const &b ) noexcept;

    private:
        [[nodiscard]] FirstContactData AllocateFirstContact ( ContactManager &contactManager,
            RigidBodyRef const &a,
            RigidBodyRef const &b,
            float friction,
            float restitution
        ) const noexcept;

        void CollectBackwardExtremePoints ( Vertices &vertices, Shape const &shape, GXMat3 const &tbn ) noexcept;
        void CollectForwardExtremePoints ( Vertices &vertices, Shape const &shape, GXMat3 const &tbn ) noexcept;
        void GenerateRays () noexcept;

        void ManifoldEdgeEdge ( ContactManager &contactManager,
            RigidBodyRef const &a,
            RigidBodyRef const &b,
            GXMat3 const &tbn,
            float friction,
            float restitution
        ) noexcept;

        void ManifoldEdgeFace ( ContactManager &contactManager,
            RigidBodyRef const &a,
            RigidBodyRef const &b,
            float friction,
            float restitution
        ) noexcept;

        void ManifoldFaceFace ( ContactManager &contactManager,
            RigidBodyRef const &a,
            RigidBodyRef const &b,
            float friction,
            float restitution
        ) noexcept;

        void ManifoldPoint ( ContactManager &contactManager,
            RigidBodyRef const &a,
            RigidBodyRef const &b,
            GXMat3 const &tbn,
            float friction,
            float restitution,
            GXVec3 const &vertex
        ) noexcept;

        [[maybe_unused]] void NotifyEPAFail () noexcept;

        static void AppendExtremePoint ( Vertices &vertices,
            Shape const &shape,
            GXMat3 const &tbn,
            GXVec3 const &ray
        ) noexcept;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_CONTACT_DETECTOR_HPP
