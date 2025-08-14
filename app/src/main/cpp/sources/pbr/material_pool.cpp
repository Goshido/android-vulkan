#include <precompiled_headers.hpp>
#include <pbr/fif_count.hpp>
#include <pbr/geometry_pass_binds.inc>
#include <pbr/geometry_pass_texture_descriptor_set_layout.hpp>
#include <pbr/material_pool.hpp>
#include <vulkan_utils.hpp>


namespace pbr {

namespace {

constexpr size_t MAX_MATERIALS_PER_FRAME = 16384U;
constexpr size_t BIND_PER_SET = 5U;
constexpr size_t MATERIALS = FIF_COUNT * MAX_MATERIALS_PER_FRAME;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

VkDescriptorSet MaterialPool::Acquire () noexcept
{
    VkDescriptorSet set = _descriptorSets[ _itemReadIndex ];
    _itemReadIndex = ( _itemReadIndex + 1U ) % MATERIALS;
    return set;
}

void MaterialPool::Commit () noexcept
{
    _itemBaseIndex = _itemWriteIndex;
    _itemReadIndex = _itemWriteIndex;
    _itemWritten = 0U;
}

bool MaterialPool::Init ( VkDevice device, DefaultTextureManager const &defaultTextureManager ) noexcept
{
    _defaultDiffuse = defaultTextureManager.GetAlbedo ()->GetImageView ();
    _defaultEmission = defaultTextureManager.GetEmission ()->GetImageView ();
    _defaultMask = defaultTextureManager.GetMask ()->GetImageView ();
    _defaultNormal = defaultTextureManager.GetNormal ()->GetImageView ();
    _defaultParam = defaultTextureManager.GetParams ()->GetImageView ();

    constexpr size_t totalImages = MATERIALS * BIND_PER_SET;

    constexpr VkDescriptorPoolSize const poolSizes[] =
    {
        {
            .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .descriptorCount = static_cast<uint32_t> ( totalImages )
        }
    };

    VkDescriptorPoolCreateInfo const poolInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .maxSets = static_cast<uint32_t> ( MATERIALS ),
        .poolSizeCount = static_cast<uint32_t> ( std::size ( poolSizes ) ),
        .pPoolSizes = poolSizes
    };

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateDescriptorPool ( device, &poolInfo, nullptr, &_descriptorPool ),
        "pbr::MaterialPool::Init",
        "Can't create descriptor pool"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _descriptorPool, VK_OBJECT_TYPE_DESCRIPTOR_POOL, "Material pool" )

    _descriptorSets.resize ( MATERIALS );
    VkDescriptorSet* descriptorSets = _descriptorSets.data ();

    std::vector<VkDescriptorSetLayout> const layouts ( MATERIALS,
        GeometryPassTextureDescriptorSetLayout ().GetLayout ()
    );

    VkDescriptorSetAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = _descriptorPool,
        .descriptorSetCount = poolInfo.maxSets,
        .pSetLayouts = layouts.data ()
    };

    result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateDescriptorSets ( device, &allocateInfo, descriptorSets ),
        "pbr::MaterialPool::Init",
        "Can't allocate descriptor sets"
    );

    if ( !result ) [[unlikely]]
        return false;

#if defined ( AV_ENABLE_VVL ) || defined ( AV_ENABLE_RENDERDOC )

    for ( size_t i = 0U; i < MATERIALS; ++i )
        AV_SET_VULKAN_OBJECT_NAME ( device, descriptorSets[ i ], VK_OBJECT_TYPE_DESCRIPTOR_SET, "Material #%zu", i )

#endif // AV_ENABLE_VVL || AV_ENABLE_RENDERDOC

    // Initialize all immutable constant fields.

    constexpr VkDescriptorImageInfo image
    {
        .sampler = VK_NULL_HANDLE,
        .imageView = VK_NULL_HANDLE,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };

    _imageInfo.resize ( totalImages, image );
    VkDescriptorImageInfo* imageInfo = _imageInfo.data ();

    constexpr VkWriteDescriptorSet writeSet
    {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = nullptr,
        .dstSet = VK_NULL_HANDLE,
        .dstBinding = 0U,
        .dstArrayElement = 0U,
        .descriptorCount = 1U,
        .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        .pImageInfo = nullptr,
        .pBufferInfo = nullptr,
        .pTexelBufferView = nullptr
    };

    _writeSets.resize ( totalImages, writeSet );
    VkWriteDescriptorSet* writeSets = _writeSets.data ();
    size_t idx = 0U;

    for ( size_t i = 0U; i < MATERIALS; ++i )
    {
        VkDescriptorSet set = descriptorSets[ i ];

        VkWriteDescriptorSet &diffuse = writeSets[ idx ];
        diffuse.dstSet = set;
        diffuse.dstBinding = static_cast<uint32_t> ( BIND_DIFFUSE_TEXTURE );
        diffuse.pImageInfo = imageInfo + idx++;

        VkWriteDescriptorSet &emission = writeSets[ idx ];
        emission.dstSet = set;
        emission.dstBinding = static_cast<uint32_t> ( BIND_EMISSION_TEXTURE );
        emission.pImageInfo = imageInfo + idx++;

        VkWriteDescriptorSet &mask = writeSets[ idx ];
        mask.dstSet = set;
        mask.dstBinding = static_cast<uint32_t> ( BIND_MASK_TEXTURE );
        mask.pImageInfo = imageInfo + idx++;

        VkWriteDescriptorSet &normal = writeSets[ idx ];
        normal.dstSet = set;
        normal.dstBinding = static_cast<uint32_t> ( BIND_NORMAL_TEXTURE );
        normal.pImageInfo = imageInfo + idx++;

        VkWriteDescriptorSet &param = writeSets[ idx ];
        param.dstSet = set;
        param.dstBinding = static_cast<uint32_t> ( BIND_PARAMS_TEXTURE );
        param.pImageInfo = imageInfo + idx++;
    }

    // Now all what is needed to do is to init "_imageInfo::imageView".
    // Then to invoke vkUpdateDescriptorSets.

    _itemBaseIndex = 0U;
    _itemReadIndex = 0U;
    _itemWriteIndex = 0U;
    _itemWritten = 0U;

    return true;
}

void MaterialPool::Destroy ( VkDevice device ) noexcept
{
    if ( _descriptorPool != VK_NULL_HANDLE ) [[likely]]
    {
        vkDestroyDescriptorPool ( device, _descriptorPool, nullptr );
        _descriptorPool = VK_NULL_HANDLE;
    }

    constexpr auto clean = [] ( auto &vector ) noexcept {
        vector.clear ();
        vector.shrink_to_fit ();
    };

    clean ( _descriptorSets );
    clean ( _imageInfo );
    clean ( _writeSets );
}

void MaterialPool::IssueSync ( VkDevice device ) const noexcept
{
    size_t const idx = _itemBaseIndex + _itemWritten;
    size_t const cases[] = { 0U, idx - MATERIALS };
    size_t const more = cases[ static_cast<size_t> ( idx > MATERIALS ) ];
    size_t const available = _itemWritten - more;

    VkWriteDescriptorSet const* writeSets = _writeSets.data ();

    vkUpdateDescriptorSets ( device,
        static_cast<uint32_t> ( BIND_PER_SET * available ),
        writeSets + BIND_PER_SET * _itemBaseIndex,
        0U,
        nullptr
    );

    if ( more > 0U ) [[unlikely]]
    {
        vkUpdateDescriptorSets ( device, static_cast<uint32_t> ( BIND_PER_SET * more ), writeSets, 0U, nullptr );
    }
}

void MaterialPool::Push ( GeometryPassMaterial &material ) noexcept
{
    VkDescriptorImageInfo* images = _imageInfo.data ();

    Texture2DRef const &diffuse = material.GetAlbedo ();
    Texture2DRef const &emission = material.GetEmission ();
    Texture2DRef const &mask = material.GetMask ();
    Texture2DRef const &normal = material.GetNormal ();
    Texture2DRef const &param = material.GetParam ();

    size_t const base = _itemWriteIndex * BIND_PER_SET;
    images[ base + BIND_DIFFUSE_TEXTURE ].imageView = diffuse ? diffuse->GetImageView () : _defaultDiffuse;
    images[ base + BIND_EMISSION_TEXTURE ].imageView = emission ? emission->GetImageView () : _defaultEmission;
    images[ base + BIND_MASK_TEXTURE ].imageView = mask ? mask->GetImageView () : _defaultMask;
    images[ base + BIND_NORMAL_TEXTURE ].imageView = normal ? normal->GetImageView () : _defaultNormal;
    images[ base + BIND_PARAMS_TEXTURE ].imageView = param ? param->GetImageView () : _defaultParam;

    _itemWriteIndex = ( _itemWriteIndex + 1U ) % MATERIALS;
    ++_itemWritten;
}

} // namespace pbr
