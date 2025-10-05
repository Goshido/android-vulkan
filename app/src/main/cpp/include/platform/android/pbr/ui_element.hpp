#ifndef PBR_UI_ELEMENT_HPP
#define PBR_UI_ELEMENT_HPP


#include "ui_vertex_info.hpp"
#include "ui_pass.hpp"
#include <pbr/ui_element_base.hpp>


namespace pbr {

using UIElement = UIElementBase<FontStorage, UIPass, UIBufferStreams>;

} // namespace pbr


#endif // PBR_UI_ELEMENT_HPP
