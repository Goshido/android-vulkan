#include <contact_detector.h>
#include <logger.h>


namespace android_vulkan {

ContactDetector::ContactDetector () noexcept:
    _epa {},
    _gjk {}
{
    // NOTHING
}

void ContactDetector::Check ( RigidBodyRef const &a,
    RigidBodyRef const &b,
    ContactManager &/*contactManager*/
) noexcept
{
    RigidBody& bodyA = *a;
    RigidBody& bodyB = *b;

    if ( bodyA.IsKinematic () && bodyB.IsKinematic () )
        return;

    Shape const& shapeA = bodyA.GetShape ();
    Shape const& shapeB = bodyB.GetShape ();

    _gjk.Reset ();

    if ( !_gjk.Run ( shapeA, shapeB ) )
    {
        LogDebug ( "ContactDetector::Check - No contacts, "
           "GJK steps: %hhu, lines: %hhu, triangles: %hhu, tetrahedrons: %hhu",
           _gjk.GetSteps (),
           _gjk.GetTestLines (),
           _gjk.GetTestTriangles (),
           _gjk.GetTestTetrahedrons ()
        );

        return;
    }

    LogDebug ( "ContactDetector::Check - There is a contact, "
        "GJK steps: %hhu, lines: %hhu, triangles: %hhu, tetrahedrons: %hhu",
        _gjk.GetSteps (),
        _gjk.GetTestLines (),
        _gjk.GetTestTriangles (),
        _gjk.GetTestTetrahedrons ()
    );

    _epa.Reset ();

    if ( !_epa.Run ( _gjk.GetSimplex (), shapeA, shapeB ) )
    {
        LogWarning ( "ContactDetector::Check - Can't find penetration depth and separation normal." );
        return;
    }

    LogDebug ( "ContactDetector::Check - Penetration depth and separation normal have been found" );
    // TODO
}

} // namespace android_vulkan
