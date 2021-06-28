#ifndef ANDROID_VULKAN_CONTACT_DETECTOR_H
#define ANDROID_VULKAN_CONTACT_DETECTOR_H


#include "contact_manager.h"
#include "cyrus_beck.h"
#include "epa.h"
#include "gjk.h"
#include "sutherland_hodgman.h"


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
        ContactDetector& operator = ( ContactDetector const & ) = delete;

        ContactDetector ( ContactDetector && ) = delete;
        ContactDetector& operator = ( ContactDetector && ) = delete;

        ~ContactDetector () = default;

        void Check ( ContactManager &contactManager, RigidBodyRef const &a, RigidBodyRef const &b ) noexcept;

    private:
        [[nodiscard]] FirstContactData AllocateFirstContact ( ContactManager &contactManager,
            RigidBodyRef const &a,
            RigidBodyRef const &b,
            GXMat3 const &tbn
        ) const noexcept;

        void CollectExtremePoints ( Vertices &vertices, Shape const &shape, GXMat3 const &tbn ) noexcept;
        void GenerateRays () noexcept;

        void ManifoldEdgeEdge ( ContactManager &contactManager,
            RigidBodyRef const &a,
            RigidBodyRef const &b,
            GXMat3 const &tbn
        ) noexcept;

        void ManifoldEdgeFace ( ContactManager &contactManager,
            RigidBodyRef const &a,
            RigidBodyRef const &b,
            GXMat3 const &tbn
        ) noexcept;

        void ManifoldFaceFace ( ContactManager &contactManager,
            RigidBodyRef const &a,
            RigidBodyRef const &b,
            GXMat3 const &tbn
        ) noexcept;

        void ManifoldPoint ( ContactManager &contactManager,
            RigidBodyRef const &a,
            RigidBodyRef const &b,
            GXMat3 const &tbn,
            GXVec3 const &vertex
        ) noexcept;

        void NotifyEPAFail () noexcept;

        [[nodiscard]] static Contact& AllocateAnotherContact ( ContactManager &contactManager,
            FirstContactData &firstContactData
        ) noexcept;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_CONTACT_DETECTOR_H
