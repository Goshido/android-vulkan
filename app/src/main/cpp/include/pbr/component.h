#ifndef PBR_COMPONENT_H
#define PBR_COMPONENT_H


#include "component_desc.h"
#include "render_session.h"
#include "script_engine.h"
#include "types.h"


namespace pbr {

class Component
{
    private:
        ClassID                 _classID;
        std::string const       _name;

    public:
        Component () = delete;

        Component ( Component const & ) = delete;
        Component& operator = ( Component const & ) = delete;

        Component ( Component && ) = delete;
        Component& operator = ( Component && ) = delete;

        virtual ~Component () = default;

        virtual void FreeTransferResources ( VkDevice device ) noexcept;
        virtual void Submit ( RenderSession &renderSession ) noexcept;

        [[nodiscard]] virtual bool RegisterScript ( ScriptEngine &scriptEngine ) noexcept;

        [[nodiscard, maybe_unused]] ClassID GetClassID () const noexcept;
        [[nodiscard]] std::string const& GetName () const noexcept;

        [[nodiscard]] static ComponentRef Create ( android_vulkan::Renderer &renderer,
            size_t &commandBufferConsumed,
            size_t &dataRead,
            ComponentDesc const &desc,
            uint8_t const* data,
            VkCommandBuffer const* commandBuffers
        ) noexcept;

    protected:
        explicit Component ( ClassID classID ) noexcept;
        explicit Component ( ClassID classID, std::string &&name ) noexcept;
};

} // namespace pbr


#endif // PBR_COMPONENT_H
