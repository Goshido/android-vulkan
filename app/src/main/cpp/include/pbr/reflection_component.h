#ifndef PBR_REFLECTION_COMPONENT_H
#define PBR_REFLECTION_COMPONENT_H


#include "component.h"
#include "reflection_component_desc.h"


namespace pbr {

class ReflectionComponent final : public Component
{
    private:
        [[maybe_unused]] bool               _isGlobal;
        [[maybe_unused]] float              _size;

        [[maybe_unused]] GXVec3             _location;
        [[maybe_unused]] TextureCubeRef     _prefilter;

    public:
        ReflectionComponent () = delete;

        ReflectionComponent ( ReflectionComponent const & ) = delete;
        ReflectionComponent& operator = ( ReflectionComponent const & ) = delete;

        ReflectionComponent ( ReflectionComponent && ) = delete;
        ReflectionComponent& operator = ( ReflectionComponent && ) = delete;

        explicit ReflectionComponent ( ReflectionComponentDesc const &desc, uint8_t const *data ) noexcept;

        ~ReflectionComponent () override = default;

    private:
        void FreeTransferResources ( android_vulkan::Renderer &renderer ) override;
        void Submit ( RenderSession &renderSession ) override;
};

} // namespace pbr


#endif // PBR_REFLECTION_COMPONENT_H
