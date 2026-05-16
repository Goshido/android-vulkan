#ifndef EDITOR_MESH_STORAGE_HPP
#define EDITOR_MESH_STORAGE_HPP


#include "mesh_geometry_ref.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <functional>
#include <optional>
#include <string_view>
#include <unordered_map>

GX_RESTORE_WARNING_STATE


namespace editor {

class MeshStorage final
{
    public:
        using LoadResult = std::function<void ( std::optional<MeshGeometryRef> && )>;

    private:
        std::unordered_map<std::string, MeshGeometryRef>    _storage {};

        static MeshStorage*                                 _instance;

    public:
        explicit MeshStorage () = default;

        MeshStorage ( MeshStorage const & ) = delete;
        MeshStorage &operator = ( MeshStorage const & ) = delete;

        MeshStorage ( MeshStorage && ) = delete;
        MeshStorage &operator = ( MeshStorage && ) = delete;

        ~MeshStorage () = default;

        // Note LoadResult could be called from IO or RenderSession thread depending of success or fail.
        void Load ( std::string_view asset, LoadResult &&result ) noexcept;
        void CollectGarbage () noexcept;

        [[nodiscard]] static MeshStorage &Instance () noexcept;
};

} // namespace editor


#endif // EDITOR_MESH_STORAGE_HPP
