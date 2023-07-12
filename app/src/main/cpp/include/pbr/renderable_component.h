#ifndef PBR_RENDERABLE_COMPONENT_H
#define PBR_RENDERABLE_COMPONENT_H


#include "component.h"
#include "render_session.h"


namespace pbr {

class RenderableComponent : public Component
{
    public:
        RenderableComponent () = delete;

        RenderableComponent ( RenderableComponent const & ) = delete;
        RenderableComponent &operator = ( RenderableComponent const & ) = delete;

        RenderableComponent ( RenderableComponent && ) = delete;
        RenderableComponent &operator = ( RenderableComponent && ) = delete;

        ~RenderableComponent () override = default;

        virtual void FreeTransferResources ( android_vulkan::Renderer &renderer ) noexcept;
        virtual void Submit ( RenderSession &renderSession ) noexcept = 0;

    protected:
        explicit RenderableComponent ( ClassID classID ) noexcept;
        explicit RenderableComponent ( ClassID classID, std::string &&name ) noexcept;
};

} // namespace pbr


#endif // PBR_RENDERABLE_COMPONENT_H
