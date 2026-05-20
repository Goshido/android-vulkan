#ifndef EDITOR_MESH_STORAGE_HPP
#define EDITOR_MESH_STORAGE_HPP


#include "mesh_upload_info.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <deque>
#include <unordered_map>

GX_RESTORE_WARNING_STATE


namespace editor {

class MeshStorage final
{
    private:
        struct Item final
        {
            MeshGeometryRef                                             _mesh {};
            size_t                                                      _references = 0U;
        };

    private:
        std::unordered_map<std::string, Item>                           _storage {};
        std::unordered_map<std::string, std::deque<MeshLoadResult>>     _tasks {};

        static MeshStorage*                                             _instance;

    public:
        explicit MeshStorage () noexcept;

        MeshStorage ( MeshStorage const & ) = delete;
        MeshStorage &operator = ( MeshStorage const & ) = delete;

        MeshStorage ( MeshStorage && ) = delete;
        MeshStorage &operator = ( MeshStorage && ) = delete;

        ~MeshStorage () = default;

        // Note load result will be called from IO thread.
        void Load ( std::string_view asset, MeshLoadResult &&result ) noexcept;
        void Unload ( MeshGeometryRef &&mesh ) noexcept;

        [[nodiscard]] static MeshStorage &Instance () noexcept;
};

} // namespace editor


#endif // EDITOR_MESH_STORAGE_HPP
