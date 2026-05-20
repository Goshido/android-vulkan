#include <precompiled_headers.hpp>
#include <texture2D_upload_info.hpp>


namespace editor {

Texture2DUploadInfo::Texture2DUploadInfo ( Texture2DRef &&texture,
    Texture2DLoadResult &&result
) noexcept:
    _texture ( std::move ( texture ) ),
    _result ( std::move ( result ) )
{
    // NOTHING
}

} // namespace editor
