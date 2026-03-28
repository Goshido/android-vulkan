#include <precompiled_headers.hpp>
#include <crash_dump.hpp>
#include <editor.hpp>


namespace {

void Fuck () noexcept
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
        uint32_t const tileX = x >> 3U;
        uint32_t const tileY = y >> 3U;
        uint32_t const tileOffset = ( tileY * tilesPerRow + tileX ) << 9U;

        uint32_t const metaLayerOffset = ( layer & 0xFFFF'FFFEU ) << 6U;

        uint32_t const tileLocalX = x & 0x0000'0007U;
        uint32_t const tileLocalY = y & 0x0000'0007U;
        uint32_t const cubeX = tileLocalX >> 1U;
        uint32_t const cubeY = tileLocalY >> 1U;
        uint32_t const cubeOffset = ( ( cubeY << 2U ) + cubeX ) << 3U;

        x &= 0x0000'0001U;
        y &= 0x0000'0001U;
        uint32_t const sampleOffset = ( ( y << 1U ) + x ) << 1U;

        uint32_t const nextOffset = layer & 0x0000'0001U;

        auto const idx = static_cast<size_t> ( tileOffset + metaLayerOffset + cubeOffset + sampleOffset + nextOffset );
        return idx;
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
            uint32_t const tileX = x >> 3U;
            uint32_t const cubeX = ( x & 0x0000'0007U ) >> 1U;
            uint32_t const cubeLocalX = x & 0x0000'0001U;

            for ( uint32_t y = 0U; y < height; ++y )
            {
                uint32_t const tileY = y >> 3U;
                uint32_t const tileOffset = ( tileY * tilesWidthCount + tileX ) << 9U;

                uint32_t const cubeY = ( y & 0x0000'0007U ) >> 1U;
                uint32_t const cubeOffset = ( ( cubeY << 2U ) + cubeX ) << 3U;

                uint32_t const cubeLocalY = y & 0x0000'0001U;
                uint32_t const sampleOffset = ( ( cubeLocalY << 1U ) + cubeLocalX ) << 1U;

                uint32_t offset = tileOffset + cubeOffset + sampleOffset;

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

} // end of anonymous namespace

[[nodiscard]] int main ( int argc, char** argv )
{
    Fuck ();

    if ( !editor::CrashDump::Install () )
        return EXIT_FAILURE;

    constexpr int skipExePath = 1;

    std::unique_ptr<editor::Editor> editor = std::make_unique<editor::Editor> (
        editor::CommandLine ( argv + skipExePath, static_cast<size_t> ( argc - skipExePath ) )
    );

    return editor->Run () ? EXIT_SUCCESS : EXIT_FAILURE;
}
