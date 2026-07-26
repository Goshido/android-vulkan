#ifndef EDITOR_TRANSFORM_HPP
#define EDITOR_TRANSFORM_HPP


#include "model.hpp"


namespace editor {

AV_DX_ALIGNMENT_BEGIN

struct Transform final
{
    Model       _model;
    uint64_t    _normal;
};

AV_DX_ALIGNMENT_END

} // namespace editor


#endif // EDITOR_TRANSFORM_HPP
