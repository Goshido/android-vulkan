#include <pbr/renderable_component.h>


namespace pbr {

void RenderableComponent::FreeTransferResources ( VkDevice /*device*/ ) noexcept
{
    // NOTHING
}

RenderableComponent::RenderableComponent ( ClassID classID, std::string &&name ) noexcept:
    Component ( classID, std::move ( name ) )
{
    // NOTHING
}

} // namespace pbr