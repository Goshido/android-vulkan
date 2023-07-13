#ifndef PBR_PARSE_RESULT_HPP
#define PBR_PARSE_RESULT_HPP


#include "stream.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <optional>

GX_RESTORE_WARNING_STATE


namespace pbr {

using ParseResult = std::optional<Stream>;

} // namespace pbr


#endif // PBR_PARSE_RESULT_HPP
