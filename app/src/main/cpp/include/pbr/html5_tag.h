#ifndef PBR_HTML5_TAG_H
#define PBR_HTML5_TAG_H


#include <GXCommon/GXWarning.h>

GX_DISABLE_COMMON_WARNINGS

#include <optional>
#include <string_view>
#include <unordered_map>

GX_RESTORE_WARNING_STATE


namespace pbr {

class HTML5Tag final
{
    public:
        enum class eTag : uint32_t
        {
            Body,
            DIV,
            Doctype,
            HTML,
            Head,
            IMG,
            Link,
            Text
        };

    private:
        eTag                                                            _tag = eTag::Doctype;

        static std::unordered_map<std::string_view, HTML5Tag> const     _tags;
        static std::unordered_map<eTag, std::string_view> const         _names;

    public:
        HTML5Tag () = default;

        HTML5Tag ( HTML5Tag const & ) = default;
        HTML5Tag& operator = ( HTML5Tag const & ) = default;

        HTML5Tag ( HTML5Tag && ) = default;
        HTML5Tag& operator = ( HTML5Tag && ) = default;

        constexpr explicit HTML5Tag ( eTag tag ) noexcept:
            _tag ( tag )
        {
            // NOTHING
        }

        ~HTML5Tag () = default;

        [[nodiscard]] bool operator == ( eTag tag ) const noexcept;

        [[nodiscard]] char const* ToString () const noexcept;

        [[nodiscard]] static std::optional<HTML5Tag> Parse ( std::string_view string ) noexcept;
};

} // namespace pbr


#endif // PBR_HTML5_TAG_H
