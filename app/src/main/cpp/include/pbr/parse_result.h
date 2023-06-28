#ifndef PBR_PARSE_RESULT_H
#define PBR_PARSE_RESULT_H


#include "stream.h"

GX_DISABLE_COMMON_WARNINGS

#include <optional>

GX_RESTORE_WARNING_STATE


namespace pbr {

using ParseResult = std::optional<Stream>;

} // namespace pbr


#endif // PBR_PARSE_RESULT_H
