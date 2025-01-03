#include <precompiled_headers.hpp>
#include <pbr/renderable_component.hpp>


namespace pbr {

void RenderableComponent::FreeTransferResources ( android_vulkan::Renderer &/*renderer*/ ) noexcept
{
    // NOTHING
}

RenderableComponent::RenderableComponent ( ClassID classID ) noexcept:
    Component ( classID )
{
    // NOTHING
}

RenderableComponent::RenderableComponent ( ClassID classID, std::string &&name ) noexcept:
    Component ( classID, std::move ( name ) )
{
    // NOTHING
}

} // namespace pbr
