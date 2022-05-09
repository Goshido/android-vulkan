#ifndef PBR_TRANSFORMABLE_H
#define PBR_TRANSFORMABLE_H


#include <GXCommon/GXMath.h>


namespace pbr {

class Transformable
{
    public:
        Transformable () = default;

        Transformable ( Transformable const & ) = delete;
        Transformable& operator = ( Transformable const & ) = delete;

        Transformable ( Transformable && ) = delete;
        Transformable& operator = ( Transformable && ) = delete;

        ~Transformable () = default;

        virtual void OnTransform ( GXMat4 const &transformWorld ) noexcept = 0;
};

} // namespace pbr


#endif // PBR_TRANSFORMABLE_H
