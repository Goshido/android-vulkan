#ifndef PBR_MESH_MANAGER_HPP
#define PBR_MESH_MANAGER_HPP


#include <GXCommon/GXWarning.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <deque>
#include <mutex>
#include <unordered_map>

GX_RESTORE_WARNING_STATE

#include "types.hpp"


namespace pbr {

class MeshManager final
{
    private:
        std::unordered_map<std::string_view, MeshRef>       _meshStorage {};
        std::deque<android_vulkan::MeshGeometry*>           _toFreeTransferResource {};

        static MeshManager*                                 _instance;
        static std::mutex                                   _mutex;

    public:
        MeshManager ( MeshManager const & ) = delete;
        MeshManager &operator = ( MeshManager const & ) = delete;

        MeshManager ( MeshManager && ) = delete;
        MeshManager &operator = ( MeshManager && ) = delete;

        void FreeTransferResources ( android_vulkan::Renderer &renderer ) noexcept;

        [[nodiscard]] MeshRef LoadMesh ( android_vulkan::Renderer &renderer,
            size_t &commandBufferConsumed,
            char const* fileName,
            VkCommandBuffer commandBuffer,
            VkFence fence
        ) noexcept;

        [[nodiscard]] static MeshManager &GetInstance () noexcept;
        static void Destroy ( android_vulkan::Renderer &renderer ) noexcept;

        [[nodiscard]] constexpr static uint32_t MaxCommandBufferPerMesh () noexcept
        {
            return 1U;
        }

    protected:
        MeshManager () = default;
        ~MeshManager () = default;

        void DestroyInternal ( android_vulkan::Renderer &renderer ) noexcept;
};

} // namespace pbr


#endif // PBR_MESH_MANAGER_HPP
