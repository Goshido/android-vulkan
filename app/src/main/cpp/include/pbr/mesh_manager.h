#ifndef PBR_MESH_MANAGER_H
#define PBR_MESH_MANAGER_H


#include <GXCommon/GXWarning.h>

GX_DISABLE_COMMON_WARNINGS

#include <shared_mutex>
#include <unordered_map>

GX_RESTORE_WARNING_STATE

#include <renderer.h>
#include "types.h"


namespace pbr {

class MeshManager final
{
    private:
        std::unordered_map<std::string_view, MeshRef>       _meshStorage;

        static MeshManager*                                 _instance;
        static std::shared_timed_mutex                      _mutex;

    public:
        MeshManager ( MeshManager const &other ) = delete;
        MeshManager& operator = ( MeshManager const &other ) = delete;

        MeshManager ( MeshManager &&other ) = delete;
        MeshManager& operator = ( MeshManager &&other ) = delete;

        [[nodiscard]] MeshRef LoadMesh ( android_vulkan::Renderer &renderer,
            size_t &commandBufferConsumed,
            char const* fileName,
            VkCommandBuffer commandBuffer
        );

        [[nodiscard]] static MeshManager& GetInstance ();
        static void Destroy ( VkDevice device );

    protected:
        MeshManager () = default;
        ~MeshManager () = default;

        void DestroyInternal ( VkDevice device );
};

} // namespace pbr


#endif // PBR_MESH_MANAGER_H
