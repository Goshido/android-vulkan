#ifndef EDITOR_POINT_LIGHT_INFO_HPP
#define EDITOR_POINT_LIGHT_INFO_HPP


#include <GXCommon/GXMath.hpp>


namespace editor {

struct PointLightInfo final
{
    GXMat4          _projection[ 6U ];
    GXAABB          _bounds {};
    GXVec3          _location {};
    float           _radius = 7.0F;
    float           _intensity = 1.0F;
    GXColorUNORM    _color { 255U, 255U, 255U, 255U };
};

} // namespace editor


#endif // EDITOR_POINT_LIGHT_INFO_HPP
