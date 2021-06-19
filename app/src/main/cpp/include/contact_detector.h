#ifndef ANDROID_VULKAN_CONTACT_DETECTOR_H
#define ANDROID_VULKAN_CONTACT_DETECTOR_H


#include "contact_manager.h"
#include "epa.h"
#include "gjk.h"


namespace android_vulkan {

class ContactDetector final
{
    private:
        EPA         _epa;
        GJK         _gjk;
        Vertices    _rays;
        Vertices    _shapeAPoints;
        Vertices    _shapeBPoints;

    public:
        ContactDetector () noexcept;

        ContactDetector ( ContactDetector const & ) = delete;
        ContactDetector& operator = ( ContactDetector const & ) = delete;

        ContactDetector ( ContactDetector && ) = delete;
        ContactDetector& operator = ( ContactDetector && ) = delete;

        ~ContactDetector () = default;

        void Check ( RigidBodyRef const &a, RigidBodyRef const &b, ContactManager &contactManager ) noexcept;

    private:
        void CollectExtremePoints ( Vertices &vertices, Shape const &shape, GXMat3 const &tbn ) noexcept;
        void GenerateRays () noexcept;

        void ManifoldEdgeEdge ( ContactManager &contactManager,
            RigidBodyRef const &a,
            RigidBodyRef const &b,
            GXMat3 const &tbn
        ) noexcept;

        [[maybe_unused]] void ManifoldEdgeFace ( ContactManager &contactManager,
            RigidBodyRef const &a,
            RigidBodyRef const &b,
            GXMat3 const &tbn,
            Vertices &aVertices,
            Vertices &bVertices
        ) noexcept;

        [[maybe_unused]] void ManifoldFaceFace ( ContactManager &contactManager,
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
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_CONTACT_DETECTOR_H
