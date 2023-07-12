#ifndef PBR_MATERIAL_POOL_HPP
#define PBR_MATERIAL_POOL_HPP


#include "default_texture_manager.h"
#include "geometry_pass_material.h"
#include "descriptor_set_layout.h"
#include "uniform_buffer_pool.h"


namespace pbr {

class MaterialPool final
{
    private:
        VkImageView                             _defaultDiffuse = VK_NULL_HANDLE;
        VkImageView                             _defaultEmission = VK_NULL_HANDLE;
        VkImageView                             _defaultMask = VK_NULL_HANDLE;
        VkImageView                             _defaultNormal = VK_NULL_HANDLE;
        VkImageView                             _defaultParam = VK_NULL_HANDLE;

        VkDescriptorPool                        _descriptorPool = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet>            _descriptorSets {};
        std::vector<VkDescriptorImageInfo>      _imageInfo {};

        size_t                                  _itemBaseIndex = 0U;
        size_t                                  _itemReadIndex = 0U;
        size_t                                  _itemWriteIndex = 0U;
        size_t                                  _itemWritten = 0U;

        std::vector<VkWriteDescriptorSet>       _writeSets {};

    public:
        MaterialPool () = default;

        MaterialPool ( MaterialPool const & ) = delete;
        MaterialPool &operator = ( MaterialPool const & ) = delete;

        MaterialPool ( MaterialPool && ) = delete;
        MaterialPool &operator = ( MaterialPool && ) = delete;

        ~MaterialPool () = default;

        [[nodiscard]] VkDescriptorSet Acquire () noexcept;
        void Commit () noexcept;

        [[nodiscard]] bool Init ( VkDevice device, DefaultTextureManager const &defaultTextureManager ) noexcept;
        void Destroy ( VkDevice device ) noexcept;

        void IssueSync ( VkDevice device ) const noexcept;
        void Push ( GeometryPassMaterial &material ) noexcept;
};

} // namespace pbr


#endif // PBR_MATERIAL_POOL_HPP
