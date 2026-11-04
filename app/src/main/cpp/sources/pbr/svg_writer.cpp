#include <precompiled_headers.hpp>
#include <pbr/svg_writer.hpp>


namespace pbr {

namespace {

constexpr std::array COLORS =
{
    std::string_view ( "#0000ff" ),
    std::string_view ( "#00ff00" ),
    std::string_view ( "#00ffff" ),
    std::string_view ( "#ff0000" ),
    std::string_view ( "#ff00ff" ),
    std::string_view ( "#ffff00" ),
    std::string_view ( "#ffffff" )
};

} // end of anonymous namespace

SVGWriter::SVGWriter ( std::string &&directory, uint32_t width, uint32_t height ) noexcept:
    _directory ( std::move ( directory ) )
{
    auto symbols = static_cast<size_t> (
        std::snprintf ( nullptr,
            0UZ,
#include <pbr/svg_prefix.in>
            ,
            width,
            height,
            width,
            height
        )
    );

    _content.resize ( symbols++ );

    std::snprintf ( _content.data (),
        symbols,
#include <pbr/svg_prefix.in>
        ,
        width,
        height,
        width,
        height
    );
}

SVGWriter::~SVGWriter () noexcept
{
    constexpr std::string_view end = R"(</svg>)";
    _content += end;

    auto const timeStamp = std::chrono::system_clock::to_time_t ( std::chrono::system_clock::now () );
    std::tm const &timeInfo = *std::localtime ( &timeStamp );

    char path[ 1024U ];

    std::snprintf ( path,
        std::size ( path ),
        R"__(%s/layout %d-%02d-%02d %02d-%02d-%02d.svg)__",
        _directory.c_str (),
        1900 + timeInfo.tm_year,
        timeInfo.tm_mon,
        timeInfo.tm_mday,
        timeInfo.tm_hour,
        timeInfo.tm_min,
        timeInfo.tm_sec
    );

    std::ofstream svg ( path, std::ios::binary | std::ios::trunc | std::ios::out );
    svg.write ( _content.data (), static_cast<std::streamsize> ( _content.size () ) );
}

void SVGWriter::AddRect ( std::string_view name, GXVec2 const &topLeft, GXVec2 const &bottomRight ) noexcept
{
    constexpr auto colorCount = static_cast<uint32_t> ( COLORS.size () );
    char const* color = COLORS[ std::exchange ( _color, ( _color + 1U ) % colorCount ) ].data ();
    char const* n = name.data ();

    GXVec2 size {};
    size.Subtract ( bottomRight, topLeft );

    constexpr GXVec2 compensateBorderThickness ( -1.0F, -1.0F );
    size.Sum ( size, compensateBorderThickness );

    constexpr GXVec2 compensateBorderOffset ( 0.5F, 0.5F );
    GXVec2 location {};
    location.Sum ( topLeft, compensateBorderOffset );

    auto symbols = static_cast<size_t> (
        std::snprintf ( nullptr,
            0UZ,
#include <pbr/svg_rect.in>
            ,
            color,
            n,
            size._data[ 0U ],
            size._data[ 1U ],
            location._data[ 0U ],
            location._data[ 1U ]
        )
    );

    size_t offset = _content.size ();
    _content.resize ( offset + symbols++ );

    std::snprintf ( _content.data () + offset,
        symbols,
#include <pbr/svg_rect.in>
        ,
        color,
        n,
        size._data[ 0U ],
        size._data[ 1U ],
        location._data[ 0U ],
        location._data[ 1U ]
    );
}

} // namespace pbr
