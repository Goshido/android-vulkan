#ifndef PBR_CSS_RULE_H
#define PBR_CSS_RULE_H


#include "property.h"

GX_DISABLE_COMMON_WARNINGS

#include <list>
#include <memory>
#include <string>
#include <unordered_map>

GX_RESTORE_WARNING_STATE


namespace pbr {

using CSSProps = std::list<std::unique_ptr<Property>>;
using CSSRules = std::unordered_map<std::u32string, CSSProps>;

} // namespace pbr


#endif // PBR_CSS_RULE_H
