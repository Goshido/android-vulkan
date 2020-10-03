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

        // Note commandBuffers must point to at least 4 free command buffers.
        [[maybe_unused]] [[nodiscard]] MeshRef LoadMesh ( std::string_view const &fileName,
            android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer
        );

        [[maybe_unused]] [[nodiscard]] MeshRef LoadMesh ( char const* fileName,
            android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer
        );

        [[maybe_unused]] [[nodiscard]] MeshManager& GetInstance ();
        static void Destroy ( android_vulkan::Renderer &renderer );

    protected:
        MeshManager () = default;
        ~MeshManager () = default;

        void DestroyInternal ( android_vulkan::Renderer &renderer );

        [[nodiscard]] MeshRef LoadMesh ( std::string &&fileName,
            android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer
        );
};

} // namespace pbr


#endif // PBR_MESH_MANAGER_H
