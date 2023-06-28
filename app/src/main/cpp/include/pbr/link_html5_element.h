#ifndef PBR_LINK_HTML5_ELEMENT_H
#define PBR_LINK_HTML5_ELEMENT_H


#include "stream.h"

GX_DISABLE_COMMON_WARNINGS

#include <optional>

GX_RESTORE_WARNING_STATE


namespace pbr {

class LinkHTML5Element final
{
    public:
        struct Result final
        {
            std::string     _assetRoot;
            std::string     _cssFile;
            Stream          _newStream;
        };

    public:
        LinkHTML5Element () = delete;

        LinkHTML5Element ( LinkHTML5Element const & ) = delete;
        LinkHTML5Element& operator = ( LinkHTML5Element const & ) = delete;

        LinkHTML5Element ( LinkHTML5Element && ) = delete;
        LinkHTML5Element& operator = ( LinkHTML5Element && ) = delete;

        ~LinkHTML5Element () = delete;

        [[nodiscard]] static std::optional<Result> Parse ( char const* html,
            Stream stream,
            const char* assetRoot
        ) noexcept;
};

} // namespace pbr


#endif // PBR_LINK_HTML5_ELEMENT_H
