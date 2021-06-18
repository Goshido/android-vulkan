#include <epa.h>
#include <logger.h>


namespace android_vulkan {

Face::Face () noexcept:
    _a ( 0U ),
    _b ( 0U ),
    _c ( 0U ),
    _normal ( 0.0F, 0.0F, 0.0F )
{
    // NOTHING
}

[[maybe_unused]] Face::Face ( size_t a, size_t b, size_t c, Vertices const &vertices ) noexcept:
    _a ( a ),
    _b ( b ),
    _c ( c ),
    _normal {}
{
    GXVec3 ab {};
    ab.Subtract ( vertices[ b ], vertices[ a ] );

    GXVec3 ac {};
    ac.Subtract ( vertices[ c ], vertices[ a ] );

    _normal.CrossProduct ( ab, ac );
    _normal.Normalize ();
}

[[maybe_unused]] void Face::Flip () noexcept
{
    _normal.Reverse ();

    size_t const tmp = _b;
    _b = _c;
    _c = tmp;
}

//----------------------------------------------------------------------------------------------------------------------

EPA::EPA () noexcept:
    _depth ( 0.0F ),
    _normal ( 1.0F, 0.0F, 0.0F ),
    _faces {},
    _vertices {}
{
    // NOTHING
}

[[maybe_unused]] float EPA::GetDepth () const noexcept
{
    return _depth;
}

[[maybe_unused]] GXVec3 const& EPA::GetNormal () const noexcept
{
    return _normal;
}

void EPA::Reset () noexcept
{
    _vertices.clear ();
    _faces.clear ();
}

bool EPA::Run ( Simplex const &simplex, Shape const &/*shapeA*/, Shape const &/*shapeB*/ ) noexcept
{
    CreatePolytope ( simplex );
    // TODO
    return false;
}

void EPA::CreatePolytope ( Simplex const &/*simplex*/ ) noexcept
{
    LogDebug ( "android_vulkan::EPA::CreatePolytope - From %p", this );
    // TODO
}

} // namespace android_vulkan
