#ifndef LIGHT_PASS_BASE_H
#define LIGHT_PASS_BASE_H


#include <renderer.h>


namespace pbr {

class LightPassNotifier
{
    public:
        LightPassNotifier ( LightPassNotifier const & ) = delete;
        LightPassNotifier& operator = ( LightPassNotifier const & ) = delete;

        LightPassNotifier ( LightPassNotifier && ) = delete;
        LightPassNotifier& operator = ( LightPassNotifier && ) = delete;

        virtual void OnBeginLightWithVolume ( VkCommandBuffer commandBuffer ) = 0;
        virtual void OnEndLightWithVolume ( VkCommandBuffer commandBuffer ) = 0;

    protected:
        LightPassNotifier () = default;
        ~LightPassNotifier () = default;
};

} // namespace pbr


#endif // LIGHT_PASS_BASE_H
