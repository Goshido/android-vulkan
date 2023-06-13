#ifndef PBR_UI_VERTEX_INFO_H
#define PBR_UI_VERTEX_INFO_H


#include <GXCommon/GXMath.h>


namespace pbr {

#pragma pack ( push, 1 )

struct UIVertexInfo final
{
    GXVec2          _vertex;
    GXColorRGB      _color;
    GXVec3          _atlas;
    GXVec2          _imageUV;

    UIVertexInfo () = default;

    constexpr UIVertexInfo ( GXVec3 const &vertex,
        GXColorRGB const &color,
        GXVec3 const &atlas,
        GXVec2 const &imageUV
    ) noexcept:
        _vertex ( vertex._data[ 0U ], vertex._data[ 1U ] ),
        _color ( color._data[ 0U ], color._data[ 1U ], color._data[ 2U ], color._data[ 3U ] ),
        _atlas ( atlas._data[ 0U ], atlas._data[ 1U ], atlas._data[ 2U ] ),
        _imageUV ( imageUV._data[ 0U ], imageUV._data[ 1U ] )
    {
        // NOTHING
    }

    ~UIVertexInfo () = default;
};

#pragma pack ( pop )

} // namespace pbr


#endif // PBR_UI_VERTEX_INFO_H
