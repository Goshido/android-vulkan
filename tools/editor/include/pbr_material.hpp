#ifndef EDITOR_PBR_MATERIAL_HPP
#define EDITOR_PBR_MATERIAL_HPP


#include "texture2D_ref.hpp"


namespace editor {

struct PBRMaterial final
{
    Texture2DRef    _albedo {};
    Texture2DRef    _emission {};
    Texture2DRef    _mask {};
    Texture2DRef    _normal {};
    Texture2DRef    _param {};
};

} // namespace editor


#endif // EDITOR_PBR_MATERIAL_HPP
