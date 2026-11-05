#include <precompiled_headers.hpp>
#include <pbr/svg_writer.hpp>


namespace pbr {

namespace {

constexpr std::array COLORS =
{
    std::string_view ( "#722e1a" ),
    std::string_view ( "#1a5772" ),
    std::string_view ( "#44721a" ),
    std::string_view ( "#631a72" ),
    std::string_view ( "#72571a" ),
    std::string_view ( "#2a1a72" ),
    std::string_view ( "#1a7244" ),
    std::string_view ( "#721a3d" ),
    std::string_view ( "#ffae93" ),
    std::string_view ( "#93e3ff" ),
    std::string_view ( "#ccff93" ),
    std::string_view ( "#f193ff" ),
    std::string_view ( "#ffe393" ),
    std::string_view ( "#a893ff" ),
    std::string_view ( "#93ffcc" ),
    std::string_view ( "#ff93cc" ),
    std::string_view ( "#7f2d13" ),
    std::string_view ( "#135f7f" ),
    std::string_view ( "#447f13" ),
    std::string_view ( "#6d137f" ),
    std::string_view ( "#7f5f13" ),
    std::string_view ( "#27137f" ),
    std::string_view ( "#137f46" ),
    std::string_view ( "#7f1341" ),
    std::string_view ( "#ff9066" ),
    std::string_view ( "#66dfff" ),
    std::string_view ( "#c8ff66" ),
    std::string_view ( "#ee66ff" ),
    std::string_view ( "#ffdf66" ),
    std::string_view ( "#9666ff" ),
    std::string_view ( "#66ffc8" ),
    std::string_view ( "#ff66b4" ),
    std::string_view ( "#a5573d" ),
    std::string_view ( "#3d85a5" ),
    std::string_view ( "#6da53d" ),
    std::string_view ( "#943da5" ),
    std::string_view ( "#a5853d" ),
    std::string_view ( "#513da5" ),
    std::string_view ( "#3da56d" ),
    std::string_view ( "#a53d68" ),
    std::string_view ( "#d8896d" ),
    std::string_view ( "#6dbad8" ),
    std::string_view ( "#9fd86d" ),
    std::string_view ( "#c66dd8" ),
    std::string_view ( "#d8ba6d" ),
    std::string_view ( "#836dd8" ),
    std::string_view ( "#6dd89f" ),
    std::string_view ( "#d86d9b" ),
    std::string_view ( "#b24a28" ),
    std::string_view ( "#288bb2" ),
    std::string_view ( "#6bb228" ),
    std::string_view ( "#9e28b2" ),
    std::string_view ( "#b28b28" ),
    std::string_view ( "#4728b2" ),
    std::string_view ( "#28b26b" ),
    std::string_view ( "#b22858" ),
    std::string_view ( "#e5734a" ),
    std::string_view ( "#4ab7e5" ),
    std::string_view ( "#9be54a" ),
    std::string_view ( "#d04ae5" ),
    std::string_view ( "#e5b74a" ),
    std::string_view ( "#724ae5" ),
    std::string_view ( "#4ae59b" ),
    std::string_view ( "#e54a82" )
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

    GXVec2 size {};
    size.Subtract ( bottomRight, topLeft );

    constexpr GXVec2 compensateBorderThickness ( -1.0F, -1.0F );
    size.Sum ( size, compensateBorderThickness );

    constexpr GXVec2 compensateBorderOffset ( 0.5F, 0.5F );
    GXVec2 location {};
    location.Sum ( topLeft, compensateBorderOffset );

    char const* n = name.data ();
    char const* suffix = "";
    char buffer[ 64UZ ];
    std::string key ( name );

    if ( auto findResult = _names.find ( key ); findResult == _names.cend () ) [[likely]]
    {
        _names.insert ( std::pair ( std::move ( key ), 1UZ ) );
    }
    else
    {
        std::snprintf ( buffer, std::size ( buffer ), " #%03zu", ++findResult->second );
        suffix = buffer;
    }

    auto symbols = static_cast<size_t> (
        std::snprintf ( nullptr,
            0UZ,
#include <pbr/svg_rect.in>
            ,
            color,
            n,
            suffix,
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
        suffix,
        size._data[ 0U ],
        size._data[ 1U ],
        location._data[ 0U ],
        location._data[ 1U ]
    );
}

} // namespace pbr
