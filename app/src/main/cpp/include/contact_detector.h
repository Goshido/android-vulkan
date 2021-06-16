#ifndef ANDROID_VULKAN_CONTACT_DETECTOR_H
#define ANDROID_VULKAN_CONTACT_DETECTOR_H


#include "contact_manager.h"
#include "epa.h"
#include "gjk.h"


namespace android_vulkan {

class ContactDetector final
{
    private:
        EPA     _epa;
        GJK     _gjk;

    public:
        ContactDetector () noexcept;

        ContactDetector ( ContactDetector const & ) = delete;
        ContactDetector& operator = ( ContactDetector const & ) = delete;

        ContactDetector ( ContactDetector && ) = delete;
        ContactDetector& operator = ( ContactDetector && ) = delete;

        ~ContactDetector () = default;

        void Check ( RigidBodyRef const &a, RigidBodyRef const &b, ContactManager &contactManager ) noexcept;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_CONTACT_DETECTOR_H
