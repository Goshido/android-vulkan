#include <file.h>
#include <pbr/html5_parser.h>

GX_DISABLE_COMMON_WARNINGS

#include <stdlib.h>

GX_RESTORE_WARNING_STATE


[[nodiscard]] int main ()
{
    constexpr char const html[] = "assets/index.html";
    android_vulkan::File htmlFile ( html );

    if ( !htmlFile.LoadContent () )
        return EXIT_FAILURE;

    std::vector<uint8_t>& data = htmlFile.GetContent ();

    bool const result = pbr::HTML5Parser ().Parse ( html,
        pbr::Stream ( pbr::Stream::Data ( data.data (), data.size () ), 1U ),
        "assets"
    );

    return result ? EXIT_SUCCESS : EXIT_FAILURE;
}
