#ifndef PBR_RESOURCE_HEAP_DESCRIPTOR_SET_LAYOUT_EXT_HPP
#define PBR_RESOURCE_HEAP_DESCRIPTOR_SET_LAYOUT_EXT_HPP


#include <pbr/descriptor_set_layout.hpp>


namespace pbr {

class ResourceHeapDescriptorSetLayoutEXT final : public pbr::DescriptorSetLayout
{
    private:
        bool                _init = false;

        static uint32_t     _resourceCapacity;

    public:
        ResourceHeapDescriptorSetLayoutEXT () = default;

        ResourceHeapDescriptorSetLayoutEXT ( ResourceHeapDescriptorSetLayoutEXT const & ) = delete;
        ResourceHeapDescriptorSetLayoutEXT &operator = ( ResourceHeapDescriptorSetLayoutEXT const & ) = delete;

        ResourceHeapDescriptorSetLayoutEXT ( ResourceHeapDescriptorSetLayoutEXT && ) = delete;
        ResourceHeapDescriptorSetLayoutEXT &operator = ( ResourceHeapDescriptorSetLayoutEXT && ) = delete;

        ~ResourceHeapDescriptorSetLayoutEXT () override = default;

        void Destroy ( VkDevice device ) noexcept override;
        [[nodiscard]] bool Init ( VkDevice device ) noexcept override;

        [[nodiscard]] VkDescriptorSetLayout &GetLayout () const noexcept override;

        static void SetResourceCapacity ( uint32_t capacity ) noexcept;
};

} // namespace pbr


#endif // PBR_RESOURCE_HEAP_DESCRIPTOR_SET_LAYOUT_EXT_HPP
