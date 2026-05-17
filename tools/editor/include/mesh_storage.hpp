#ifndef EDITOR_MESH_STORAGE_HPP
#define EDITOR_MESH_STORAGE_HPP


#include "mesh_upload_info.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <unordered_map>

GX_RESTORE_WARNING_STATE


namespace editor {

class MeshStorage final
{
    private:
        std::unordered_map<std::string, MeshGeometryRef>    _storage {};

        static MeshStorage*                                 _instance;

    public:
        explicit MeshStorage () noexcept;

        MeshStorage ( MeshStorage const & ) = delete;
        MeshStorage &operator = ( MeshStorage const & ) = delete;

        MeshStorage ( MeshStorage && ) = delete;
        MeshStorage &operator = ( MeshStorage && ) = delete;

        ~MeshStorage () = default;

        // Note LoadResult could be called from IO or RenderSession thread depending of success or fail.
        void Load ( std::string_view asset, MeshLoadResult &&result ) noexcept;
        void Unload ( MeshGeometryRef &&mesh ) noexcept;

        [[nodiscard]] static MeshStorage &Instance () noexcept;
};

} // namespace editor


#endif // EDITOR_MESH_STORAGE_HPP
