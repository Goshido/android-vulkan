#ifndef PBR_TRANSFORMABLE_HPP
#define PBR_TRANSFORMABLE_HPP


#include <GXCommon/GXMath.hpp>


namespace pbr {

class Transformable
{
    public:
        Transformable () = default;

        Transformable ( Transformable const & ) = delete;
        Transformable &operator = ( Transformable const & ) = delete;

        Transformable ( Transformable && ) = delete;
        Transformable &operator = ( Transformable && ) = delete;

        ~Transformable () = default;

        virtual void OnTransform ( GXMat4 const &transformWorld ) noexcept = 0;
};

} // namespace pbr


#endif // PBR_TRANSFORMABLE_HPP
