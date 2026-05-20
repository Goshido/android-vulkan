#ifndef EDITOR_TEXTURE_2D_UPLOAD_INFO_HPP
#define EDITOR_TEXTURE_2D_UPLOAD_INFO_HPP


#include "texture2D_ref.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <functional>
#include <optional>

GX_RESTORE_WARNING_STATE


namespace editor {

using Texture2DLoadResult = std::function<void ( std::optional<Texture2DRef> && )>;

class Texture2DUploadInfo final
{
    public:
        Texture2DRef            _texture {};

        // FUCK
        //android_vulkan::MeshGeometry::Info      _info {};
        Texture2DLoadResult     _result = nullptr;

    public:
        Texture2DUploadInfo () = delete;

        Texture2DUploadInfo ( Texture2DUploadInfo const & ) = default;
        Texture2DUploadInfo &operator = ( Texture2DUploadInfo const & ) = delete;

        Texture2DUploadInfo ( Texture2DUploadInfo && ) = default;
        Texture2DUploadInfo &operator = ( Texture2DUploadInfo && ) = default;

        explicit Texture2DUploadInfo ( Texture2DRef &&texture,

            // FUCK
            //android_vulkan::MeshGeometry::Info &&info,

            Texture2DLoadResult &&result
        ) noexcept;

        ~Texture2DUploadInfo () = default;
};

} // namespace editor


#endif // EDITOR_TEXTURE_2D_UPLOAD_INFO_HPP
