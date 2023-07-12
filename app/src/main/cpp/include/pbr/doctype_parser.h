#ifndef PBR_DOCTYPE_PARSER_H
#define PBR_DOCTYPE_PARSER_H


#include "parse_result.h"

GX_DISABLE_COMMON_WARNINGS

#include <optional>

GX_RESTORE_WARNING_STATE


namespace pbr {

class DoctypeParser final
{
    public:
        DoctypeParser () = default;

        DoctypeParser ( DoctypeParser const & ) = delete;
        DoctypeParser &operator = ( DoctypeParser const & ) = delete;

        DoctypeParser ( DoctypeParser && ) = delete;
        DoctypeParser &operator = ( DoctypeParser && ) = delete;

        ~DoctypeParser () = default;

        [[nodiscard]] static ParseResult Parse ( char const* html, Stream stream ) noexcept;
};

} // namespace pbr


#endif // PBR_DOCTYPE_PARSER_H
