#ifndef EDITOR_RESOURCE_HEAP_HPP
#define EDITOR_RESOURCE_HEAP_HPP


#include <platform/windows/pbr/resource_heap.hpp>


namespace editor {

class ResourceHeap final
{
    private:
        pbr::ResourceHeap       _resourceHeap {};
        static ResourceHeap*    _instance;

    public:
        explicit ResourceHeap () noexcept;

        ResourceHeap ( ResourceHeap const & ) = delete;
        ResourceHeap &operator = ( ResourceHeap const & ) = delete;

        ResourceHeap ( ResourceHeap && ) = delete;
        ResourceHeap &operator = ( ResourceHeap && ) = delete;

        ~ResourceHeap () = default;

        [[nodiscard]] static pbr::ResourceHeap &Instance () noexcept;
};

} // namespace editor


#endif // EDITOR_RESOURCE_HEAP_HPP
