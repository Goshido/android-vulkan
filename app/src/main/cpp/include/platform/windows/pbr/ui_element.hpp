#ifndef PBR_UI_ELEMENT_HPP
#define PBR_UI_ELEMENT_HPP


#include <pbr/ui_element_base.hpp>
#include "ui_pass.hpp"
#include "ui_vertex_info.hpp"


namespace pbr {

using UIElement = UIElementBase<FontStorage, UIPass, UIBufferStreams>;

} // namespace pbr


#endif // PBR_UI_ELEMENT_HPP
