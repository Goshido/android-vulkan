#ifndef EDITOR_BLIT_DESCRIPTOR_SET_LAYOUT_HPP
#define EDITOR_BLIT_DESCRIPTOR_SET_LAYOUT_HPP


#include <pbr/descriptor_set_layout.hpp>


namespace editor {

class BlitDescriptorSetLayout final : public pbr::DescriptorSetLayout
{
    public:
        BlitDescriptorSetLayout () = default;

        BlitDescriptorSetLayout ( BlitDescriptorSetLayout const & ) = delete;
        BlitDescriptorSetLayout &operator = ( BlitDescriptorSetLayout const & ) = delete;

        BlitDescriptorSetLayout ( BlitDescriptorSetLayout && ) = delete;
        BlitDescriptorSetLayout &operator = ( BlitDescriptorSetLayout && ) = delete;

        ~BlitDescriptorSetLayout () override = default;

        void Destroy ( VkDevice device ) noexcept override;
        [[nodiscard]] bool Init ( VkDevice device ) noexcept override;

        [[nodiscard]] VkDescriptorSetLayout &GetLayout () const noexcept override;
};

} // namespace editor


#endif // EDITOR_BLIT_DESCRIPTOR_SET_LAYOUT_HPP
