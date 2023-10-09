#include <file.hpp>
#include <logger.hpp>
#include <skin_data.hpp>
#include <android_vulkan_sdk/skeleton.hpp>
#include <android_vulkan_sdk/skin.hpp>


namespace android_vulkan {

void SkinData::FreeResources ( Renderer &/*renderer*/ ) noexcept
{
    // TODO
}

void SkinData::FreeTransferResources ( Renderer &/*renderer*/ ) noexcept
{
    // TODO
}

GXAABB const &SkinData::GetBounds () const noexcept
{
    return _bounds;
}

VkBuffer const &SkinData::GetBuffer () const noexcept
{
    return _skin._buffer;
}

std::string const &SkinData::GetName () const noexcept
{
    return _fileName;
}

bool SkinData::LoadSkin ( std::string &&skinFilename,
    std::string &&skeletonFilename,
    Renderer &renderer,
    VkCommandBuffer /*commandBuffer*/,
    VkFence /*fence*/
) noexcept
{
    if ( skinFilename.empty () )
    {
        LogError ( "SkinData::LoadSkin - Can't upload data. Skin filename is empty." );
        return false;
    }

    if ( skeletonFilename.empty () )
    {
        LogError ( "SkinData::LoadSkin - Can't upload data. Skeleton filename is empty." );
        return false;
    }

    FreeResourceInternal ( renderer );

    File skinFile ( std::move ( skinFilename ) );
    File skeletonFile ( std::move ( skeletonFilename ) );

    if ( !skinFile.LoadContent () || !skeletonFile.LoadContent () )
        return false;

    uint8_t const* skin = skinFile.GetContent ().data ();
    uint8_t const* skeleton = skeletonFile.GetContent ().data ();

    auto const &skinHeader = reinterpret_cast<android_vulkan::SkinHeader const*> ( skin );
    auto const &skeletonHeader = reinterpret_cast<android_vulkan::SkeletonHeader const*> ( skeleton );
    (void)skinHeader;
    (void)skeletonHeader;

    // TODO
    _fileName = std::move ( skinFile.GetPath () );
    return true;
}

void SkinData::FreeResourceInternal ( Renderer &/*renderer*/ ) noexcept
{
    // TODO
}

} // namespace android_vulkan
