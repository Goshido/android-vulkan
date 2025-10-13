#ifndef PBR_RESOURCE_HEAP_DESCRIPTOR_SET_LAYOUT_HPP
#define PBR_RESOURCE_HEAP_DESCRIPTOR_SET_LAYOUT_HPP


#include <pbr/descriptor_set_layout.hpp>


namespace pbr {

class ResourceHeapDescriptorSetLayout final : public pbr::DescriptorSetLayout
{
    private:
        bool                _init = false;

        static uint32_t     _resourceCapacity;

    public:
        ResourceHeapDescriptorSetLayout () = default;

        ResourceHeapDescriptorSetLayout ( ResourceHeapDescriptorSetLayout const & ) = delete;
        ResourceHeapDescriptorSetLayout &operator = ( ResourceHeapDescriptorSetLayout const & ) = delete;

        ResourceHeapDescriptorSetLayout ( ResourceHeapDescriptorSetLayout && ) = delete;
        ResourceHeapDescriptorSetLayout &operator = ( ResourceHeapDescriptorSetLayout && ) = delete;

        ~ResourceHeapDescriptorSetLayout () override = default;

        void Destroy ( VkDevice device ) noexcept override;
        [[nodiscard]] bool Init ( VkDevice device ) noexcept override;

        [[nodiscard]] VkDescriptorSetLayout &GetLayout () const noexcept override;

        static void SetResourceCapacity ( uint32_t capacity ) noexcept;
};

} // namespace pbr


#endif // PBR_RESOURCE_HEAP_DESCRIPTOR_SET_LAYOUT_HPP
