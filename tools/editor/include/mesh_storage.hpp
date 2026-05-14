#ifndef EDITOR_MESH_STORAGE_HPP
#define EDITOR_MESH_STORAGE_HPP


#include "mesh_geometry_ref.hpp"
#include "message_queue.hpp"
#include <renderer.hpp>

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
        android_vulkan::Renderer                            &_renderer;
        std::unordered_map<std::string, MeshGeometryRef>    _storage {};

        static MeshStorage*                                 _instance;

    public:
        MeshStorage () = delete;

        MeshStorage ( MeshStorage const & ) = delete;
        MeshStorage &operator = ( MeshStorage const & ) = delete;

        MeshStorage ( MeshStorage && ) = delete;
        MeshStorage &operator = ( MeshStorage && ) = delete;

        explicit MeshStorage ( android_vulkan::Renderer &renderer ) noexcept;

        ~MeshStorage () = default;

        // Note LoadResult could be called from IO or RenderSession thread depending of success or fail.
        void Load ( std::string_view asset, LoadResult &&result ) noexcept;
        void CollectGarbage () noexcept;

        [[nodiscard]] static MeshStorage &Instance () noexcept;
};

} // namespace editor


#endif // EDITOR_MESH_STORAGE_HPP
