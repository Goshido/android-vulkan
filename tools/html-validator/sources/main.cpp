#include <precompiled_headers.hpp>
#include <file.hpp>
#include <logger.hpp>
#include <pbr/html5_parser.hpp>


namespace {

constexpr auto RETRY_TIMEOUT = std::chrono::seconds ( 2U );

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

[[nodiscard]] int main ( int argc, char* argv[] )
{
    // Make sure that string to number parsers will be consistent.
    std::locale::global ( std::locale::classic () );

    if ( argc < 2 )
    {
        android_vulkan::LogError ( "Specify HTML file to check. Abort." );
        system ( "pause" );
        return EXIT_FAILURE;
    }

    char const* html = argv[ 1U ];
    auto const assetRoot = std::filesystem::absolute ( html ).parent_path ();

    for ( ; ; )
    {
        std::system ( "cls" );
        android_vulkan::File htmlFile ( html );

        if ( !htmlFile.LoadContent () )
            return EXIT_FAILURE;

        std::vector<uint8_t> &data = htmlFile.GetContent ();

        bool const result = pbr::HTML5Parser ().Parse ( html,
            pbr::Stream ( pbr::Stream::Data ( data.data (), data.size () ), 1U ),
            assetRoot.string ().c_str ()
        );

        if ( result )
            android_vulkan::LogError ( "%s is valid.", html );

        std::this_thread::sleep_for ( RETRY_TIMEOUT );
    }

    return EXIT_SUCCESS;
}
