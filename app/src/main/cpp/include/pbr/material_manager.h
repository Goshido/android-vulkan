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
        [[nodiscard]] MaterialRef LoadMaterial ( size_t &commandBufferConsumed,
            char const* fileName,
            android_vulkan::Renderer &renderer,
            VkCommandBuffer const* commandBuffers
        );

        [[nodiscard]] static MaterialManager& GetInstance ();
        static void Destroy ( android_vulkan::Renderer &renderer );

    private:
        MaterialManager() = default;
        ~MaterialManager () = default;

        void DestroyInternal ( android_vulkan::Renderer &renderer );
};

} // namespace pbr


#endif // PBR_MATERIAL_MANAGER_H
