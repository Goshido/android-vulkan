#ifndef PBR_SRC_PROPERTY_CHECKER_H
#define PBR_SRC_PROPERTY_CHECKER_H


#include "property_checker.h"

GX_DISABLE_COMMON_WARNINGS

#include <string>

GX_RESTORE_WARNING_STATE


namespace pbr {

class SRCPropertyChecker final : public PropertyChecker
{
    private:
        struct ParseInfo final
        {
            std::u32string_view     _resource;
            std::u32string_view     _tail;
        };

        using ParseResult = std::optional<ParseInfo>;

    private:
        std::string const&          _assetRoot;
        std::string&                _target;

    public:
        SRCPropertyChecker () = delete;

        SRCPropertyChecker ( SRCPropertyChecker const & ) = delete;
        SRCPropertyChecker& operator = ( SRCPropertyChecker const & ) = delete;

        SRCPropertyChecker ( SRCPropertyChecker && ) = delete;
        SRCPropertyChecker& operator = ( SRCPropertyChecker && ) = delete;

        explicit SRCPropertyChecker ( char const* css, std::string &target, std::string const &assetRoot ) noexcept;

        ~SRCPropertyChecker () override = default;

        [[nodiscard]] Result Process ( PropertyParser::Result &result ) noexcept override;

    private:
        [[nodiscard]] ParseResult ParseQuoted ( size_t line, std::u32string_view value, char32_t quote ) noexcept;
        [[nodiscard]] ParseResult ParseUnquoted ( size_t line, std::u32string_view value ) noexcept;
};

} // namespace pbr


#endif // PBR_SRC_PROPERTY_CHECKER_H
