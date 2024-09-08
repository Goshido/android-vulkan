#ifndef EDITOR_HELLO_TRIANGLE_VERTEX_HPP
#define EDITOR_HELLO_TRIANGLE_VERTEX_HPP


#include <GXCommon/GXMath.hpp>


namespace editor {

#pragma pack ( push, 1 )

struct HelloTriangleVertex final
{
    GXVec2      _vertex {};
    GXVec3      _color {};
};

#pragma pack ( pop )

} // namespace editor


#endif // EDITOR_HELLO_TRIANGLE_VERTEX_HPP
