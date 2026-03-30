#include <precompiled_headers.hpp>
#include <crash_dump.hpp>
#include <editor.hpp>

// FUCK
#include <tools/hlsl.hpp>


namespace hlsl {

void Test () noexcept
{
    constexpr uint32_t width = 777U;
    constexpr uint32_t height = 553U;

    constexpr auto roundUp = [] ( uint32_t v, uint32_t factor ) consteval -> uint32_t
    {
        v += factor - 1U;
        return v / factor;
    };

    constexpr uint32_t tileWidth = 8U;
    constexpr uint32_t tileHeight = 8U;
    constexpr uint32_t tileLayers = 8U;

    constexpr uint32_t samplesPerTileLayer = tileWidth * tileHeight;
    constexpr uint32_t samplesPerTile = samplesPerTileLayer * tileLayers;

    constexpr uint32_t tilesWidthCount = roundUp ( width, tileWidth );
    constexpr uint32_t tilesHeightCount = roundUp ( height, tileHeight );

    constexpr auto size = static_cast<size_t> ( tilesWidthCount * tilesHeightCount * samplesPerTile );

    constexpr uint32_t fake = std::numeric_limits<uint32_t>::max ();
    std::vector<uint32_t> storage ( size, fake );
    uint32_t* data = storage.data ();

    // See <repo>/docs/gizmo-rendering.md#samples
    constexpr auto framentIdx = [] ( uint32_t x, uint32_t y, uint32_t layer, uint32_t tilesPerRow ) -> size_t {
        uint32_t4 const alpha = 
            uint32_t4 ( x, y, x, y ) & uint32_t4 ( 0x0000'0007U, 0x0000'0007U, 0x0000'0001U, 0x0000'0001U );

        uint32_t2 const beta = (uint32_t2)layer & uint32_t2 ( 0xFFFF'FFFEU, 0x0000'0001U );
        uint32_t4 const yota = uint32_t4 ( x, y, alpha.xy ) >> uint32_t4 ( 3U, 3U, 1U, 1U );

        uint32_t4 zeta = uint32_t4 ( yota.y * tilesPerRow + yota.x,
            beta.x,
            ( uint32_t2 ( yota.w, alpha.w ) << uint32_t2 ( 2U, 1U ) ) + uint32_t2 ( yota.z, alpha.z )
        );

        zeta <<= uint32_t4 ( 9U, 6U, 3U, 1U );
        return zeta.x + zeta.y + zeta.z + zeta.w + beta.y;
    };

    auto const fill = [ data ] ( uint32_t v, auto indexer ) noexcept {
        for ( uint32_t x = 0U; x < width; ++x )
        {
            for ( uint32_t y = 0U; y < height; ++y )
            {
                for ( uint32_t layer = 0U; layer < tileLayers; ++layer )
                {
                    data[ indexer ( x, y, layer, tilesWidthCount ) ] = v;
                }
            }
        }
    };

    auto const incrementFragment = [ data ] () noexcept {
        for ( uint32_t x = 0U; x < width; ++x )
        {
            for ( uint32_t y = 0U; y < height; ++y )
            {
                for ( uint32_t layer = 0U; layer < tileLayers; ++layer )
                {
                    data[ framentIdx ( x, y, layer, tilesWidthCount ) ] += 1U;
                }
            }
        }
    };

    auto const incrementCompute = [ data ] () noexcept {
        constexpr uint32_t const cases[] = { 1U, 127U };

        for ( uint32_t x = 0U; x < width; ++x )
        {
            for ( uint32_t y = 0U; y < height; ++y )
            {
                uint32_t4 const alpha =
                    uint32_t4 ( x, x, y, y ) & uint32_t4 ( 0x0000'0007U, 0x0000'0001U, 0x0000'0007U, 0x0000'0001U );

                uint32_t4 const beta = uint32_t4 ( alpha.xz, x, y ) >> uint32_t4 ( 1U, 1U, 3U, 3U );

                uint32_t3 zeta = uint32_t3 ( beta.w * tilesWidthCount,
                    uint32_t2 ( beta.y, alpha.w ) << uint32_t2 ( 2U, 1U )
                );

                zeta += uint32_t3 ( beta.zx, alpha.y );
                zeta <<= uint32_t3 ( 9U, 3U, 1U );
                uint32_t offset = zeta.x + zeta.y + zeta.z;

                for ( uint32_t layer = 0U; layer < tileLayers; ++layer )
                {
                    data[ offset ] += 1U;
                    offset += cases[ static_cast<size_t> ( layer & 0x0000'0001U ) ];
                }
            }
        }
    };

    auto const check = [ &storage ] () noexcept {
        size_t passed = 0U;

        for ( uint32_t const v : storage )
        {
            if ( v == fake ) [[unlikely]]
                continue;

            if ( v != 1U )
            {
                std::printf ( "fuck!" );
                continue;
            }

            ++passed;
        }

        constexpr auto expected = static_cast<size_t> ( width * height * tileLayers );

        if ( passed != expected ) [[unlikely]]
        {
            std::printf ( "fuck!" );
        }
    };

    fill ( 0, framentIdx );
    incrementFragment ();
    check ();

    std::memset ( data, 0xFF, size * sizeof ( fake ) );
    fill ( 0, framentIdx );
    incrementCompute ();
    check ();

    GXVec2 const stop {};
}

} // hlsl

[[nodiscard]] int main ( int argc, char** argv )
{
    hlsl::Test ();

    if ( !editor::CrashDump::Install () )
        return EXIT_FAILURE;

    constexpr int skipExePath = 1;

    std::unique_ptr<editor::Editor> editor = std::make_unique<editor::Editor> (
        editor::CommandLine ( argv + skipExePath, static_cast<size_t> ( argc - skipExePath ) )
    );

    return editor->Run () ? EXIT_SUCCESS : EXIT_FAILURE;
}
