#ifndef PBR_MATERIAL_MANAGER_H
#define PBR_MATERIAL_MANAGER_H


#include <GXCommon/GXWarning.h>

GX_DISABLE_COMMON_WARNINGS

#include <shared_mutex>

GX_RESTORE_WARNING_STATE


#include "material.h"
#include "types.h"


namespace pbr {

class MaterialManager final
{
    private:
        static MaterialManager*             _instance;
        static std::shared_timed_mutex      _mutex;

    public:
        MaterialManager ( MaterialManager const &other ) = delete;
        MaterialManager& operator = ( MaterialManager const &other ) = delete;

        MaterialManager ( MaterialManager &&other ) = delete;
        MaterialManager& operator = ( MaterialManager &&other ) = delete;

        // Note commandBuffers must point to at least 4 free command buffers.
        [[maybe_unused]] [[nodiscard]] MaterialRef LoadMaterial ( std::string_view const &fileName,
            android_vulkan::Renderer &renderer,
            VkCommandBuffer const* commandBuffers
        );

        [[nodiscard]] static MaterialManager& GetInstance ();
        static void Destroy ();

    private:
        MaterialManager() = default;
        ~MaterialManager () = default;
};

} // namespace pbr


#endif // PBR_MATERIAL_MANAGER_H
