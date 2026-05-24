#ifndef EDITOR_TEXTURE_2D_REF_HPP
#define EDITOR_TEXTURE_2D_REF_HPP


#include <texture2D.hpp>


namespace editor {

class Texture2D final
{
    public:
        android_vulkan::Texture2D       _resource {};
        uint32_t                        _heapIndex = 0U;

    public:
        Texture2D () = default;

        Texture2D ( Texture2D const & ) = delete;
        Texture2D &operator = ( Texture2D const & ) = delete;

        Texture2D ( Texture2D && ) = default;
        Texture2D &operator = ( Texture2D && ) = default;

        ~Texture2D () = default;
};

using Texture2DRef = std::shared_ptr<Texture2D>;

} // namespace editor


#endif // EDITOR_TEXTURE_2D_REF_HPP
