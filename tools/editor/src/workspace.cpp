#include <precompiled_headers.hpp>
#include <trace.hpp>
#include <workspace.hpp>


namespace editor {

Workspace::Workspace ( MessageQueue& messageQueue ) noexcept:
    _messageQueue ( messageQueue )
{
    // NOTHING
}

void Workspace::Init () noexcept
{
    AV_TRACE ( "Workspace init" )
    // FUCK
}

void Workspace::Destroy () noexcept
{
    AV_TRACE ( "Workspace destroy" )
    // FUCK
}

void Workspace::Draw ( VkCommandBuffer commandBuffer ) noexcept
{
    AV_TRACE ( "Workspace draw" )
    AV_VULKAN_GROUP ( commandBuffer, "Workspace draw" )

    DrawOpaque ( commandBuffer );
    DrawGizmo ( commandBuffer );
    DrawUI ( commandBuffer );
}

void Workspace::Pick ( int32_t /*x*/, int32_t /*y*/, GXMat4 const &/*viewer*/, GXMat4 const &/*projection*/ ) noexcept
{
    AV_TRACE ( "Workspace pick" )
    // FUCK
}

void Workspace::Pick ( Rect const &/*rect*/, GXMat4 const &/*viewer*/, GXMat4 const &/*projection*/ ) noexcept
{
    AV_TRACE ( "Workspace pick" )
    // FUCK
}

OpaqueMeshNode Workspace::RegisterOpaqueMesh ( MeshGeometryRef &/*mesh*/ ) noexcept
{
    AV_TRACE ( "Workspace register opaque mesh" )
    // FUCK
    return {};
}

void Workspace::Unregister ( OpaqueMeshNode &/*node*/ ) noexcept
{
    AV_TRACE ( "Workspace unregister opaque mesh" )
    // FUCK
}

void Workspace::DrawOpaque ( VkCommandBuffer commandBuffer )
{
    AV_TRACE ( "Opaque" )
    AV_VULKAN_GROUP ( commandBuffer, "Opaque" )
    // FUCK
}

void Workspace::DrawGizmo ( VkCommandBuffer commandBuffer )
{
    AV_TRACE ( "Gizmo" )
    AV_VULKAN_GROUP ( commandBuffer, "Gizmo" )
    // FUCK
}

void Workspace::DrawUI ( VkCommandBuffer commandBuffer )
{
    AV_TRACE ( "UI" )
    AV_VULKAN_GROUP ( commandBuffer, "UI" )
    // FUCK
}

} // namespace editor
