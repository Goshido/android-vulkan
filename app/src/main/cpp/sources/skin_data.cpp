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

    uint8_t* skin = skinFile.GetContent ().data ();
    uint8_t const* skeleton = skeletonFile.GetContent ().data ();

    auto const &skinHeader = *reinterpret_cast<android_vulkan::SkinHeader const*> ( skin );
    auto const &skeletonHeader = *reinterpret_cast<android_vulkan::SkeletonHeader const*> ( skeleton );

    std::unordered_map<std::string_view, int32_t> skeletonBones {};
    auto boneCount = static_cast<int32_t> ( skeletonHeader._boneCount );

    auto const* nameOffset = reinterpret_cast<android_vulkan::UTF8Offset const*> (
        skeleton + skeletonHeader._nameInfoOffset
    );

    for ( int32_t i = 0; i < boneCount; ++i )
    {
        skeletonBones.emplace ( reinterpret_cast<char const*> ( skeleton + *nameOffset ), i );
        ++nameOffset;
    }

    std::unordered_map<uint32_t, uint32_t> boneMapper {};
    boneCount = static_cast<int32_t> ( skinHeader._boneCount );
    auto const* skinBone = reinterpret_cast<android_vulkan::SkinBone const*> ( skin + skinHeader._boneDataOffset );
    auto const end = skeletonBones.cend ();

    for ( int32_t i = 0; i < boneCount; ++i )
    {
        auto const* name = reinterpret_cast<char const*> ( skin + skinBone->_name );
        auto const findResult = skeletonBones.find ( name );

        if ( findResult != end )
        {
            boneMapper.emplace ( skinBone->_index, static_cast<uint32_t> ( findResult->second ) );
            ++skinBone;
            continue;
        }

        LogWarning ( "SkinData::LoadSkin - Can't find bone '%s'. Skin: '%s'. Skeleton: '%s'.",
            name,
            skinFile.GetPath ().c_str (),
            skeletonFile.GetPath ().c_str ()
        );

        return false;
    }

    uint32_t const skinVertexCount = skinHeader._skinVertexCount;
    auto* skinInfluence = reinterpret_cast<android_vulkan::SkinInfluence*> ( skin + skinHeader._skinVertexDataOffset );

    for ( uint32_t i = 0U; i < skinVertexCount; ++i )
    {
        skinInfluence->_boneIndex = boneMapper.find ( skinInfluence->_boneIndex )->second;
        ++skinInfluence;
    }

    std::memcpy ( &_bounds._min, skinHeader._bounds._min, sizeof ( _bounds._min ) );
    std::memcpy ( &_bounds._max, skinHeader._bounds._max, sizeof ( _bounds._max ) );
    _bounds._vertices = 2U;

    // TODO
    _fileName = std::move ( skinFile.GetPath () );
    return true;
}

void SkinData::FreeResourceInternal ( Renderer &/*renderer*/ ) noexcept
{
    // TODO
}

} // namespace android_vulkan
