#ifndef PBR_MATERIAL_MANAGER_H
#define PBR_MATERIAL_MANAGER_H


#include <GXCommon/GXWarning.h>

GX_DISABLE_COMMON_WARNINGS

#include <shared_mutex>
#include <unordered_map>

GX_RESTORE_WARNING_STATE

#include <file.h>
#include "types.h"


namespace pbr {

class MaterialManager final
{
    private:
        std::unordered_map<std::string_view, Texture2DRef>      _textureStorage;

        static MaterialManager*                                 _instance;
        static std::shared_timed_mutex                          _mutex;

    public:
        MaterialManager ( MaterialManager const &other ) = delete;
        MaterialManager& operator = ( MaterialManager const &other ) = delete;

        MaterialManager ( MaterialManager &&other ) = delete;
        MaterialManager& operator = ( MaterialManager &&other ) = delete;

        // Note commandBuffers must point to at least 4 free command buffers.
        [[nodiscard]] MaterialRef LoadMaterial ( android_vulkan::Renderer &renderer,
            size_t &commandBufferConsumed,
            char const* fileName,
            VkCommandBuffer const* commandBuffers
        ) noexcept;

        [[nodiscard]] static MaterialManager& GetInstance () noexcept;
        static void Destroy ( VkDevice device ) noexcept;

        [[nodiscard]] constexpr static uint32_t MaxCommandBufferPerMaterial () noexcept
        {
            return 4U;
        }

    private:
        MaterialManager() = default;
        ~MaterialManager () = default;

        void DestroyInternal ( VkDevice device ) noexcept;
};

} // namespace pbr


#endif // PBR_MATERIAL_MANAGER_H
