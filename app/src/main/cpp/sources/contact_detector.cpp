#include <contact_detector.h>
#include <gjk.h>
#include <logger.h>


namespace android_vulkan {

void ContactDetector::Check ( RigidBodyRef const &a,
    RigidBodyRef const &b,
    ContactManager &/*contactManager*/
) noexcept
{
    RigidBody& bodyA = *a;
    RigidBody& bodyB = *b;

    if ( bodyA.IsKinematic () && bodyB.IsKinematic () )
        return;

    GJK gjk {};

    if ( !gjk.Run ( bodyA.GetShape (), bodyB.GetShape () ) )
    {
        LogDebug ( "ContactDetector::Check - No contacts, "
           "GJK steps: %hhu, lines: %hhu, triangles: %hhu, tetrahedrons: %hhu",
           gjk.GetSteps (),
           gjk.GetTestLines (),
           gjk.GetTestTriangles (),
           gjk.GetTestTetrahedrons ()
        );

        return;
    }

    LogDebug ( "ContactDetector::Check - There is a contact, "
        "GJK steps: %hhu, lines: %hhu, triangles: %hhu, tetrahedrons: %hhu",
        gjk.GetSteps (),
        gjk.GetTestLines (),
        gjk.GetTestTriangles (),
        gjk.GetTestTetrahedrons ()
    );

    // TODO
}

} // namespace android_vulkan
