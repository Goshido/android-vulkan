#ifndef PBR_SVG_WRITER_HPP
#define PBR_SVG_WRITER_HPP


#include <GXCommon/GXMath.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <string_view>

GX_RESTORE_WARNING_STATE


namespace pbr {

class SVGWriter final
{
    private:
        std::string                                 _content {};
        std::string const                           _directory {};
        std::unordered_map<std::string, size_t>     _names {};
        uint32_t                                    _color = 0U;

    public:
        SVGWriter () = delete;

        SVGWriter ( SVGWriter const & ) = delete;
        SVGWriter &operator = ( SVGWriter const & ) = delete;

        SVGWriter ( SVGWriter && ) = delete;
        SVGWriter &operator = ( SVGWriter && ) = delete;

        explicit SVGWriter ( std::string &&directory, uint32_t width, uint32_t height ) noexcept;

        ~SVGWriter () noexcept;

        void AddRect ( std::string_view name, GXVec2 const &topLeft, GXVec2 const &bottomRight ) noexcept;
};

} // namespace pbr


#endif // PBR_SVG_WRITER_HPP
