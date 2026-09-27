#include <precompiled_headers.hpp>
#include <resource_heap.hpp>


namespace editor {

ResourceHeap* ResourceHeap::_instance = nullptr;

ResourceHeap::ResourceHeap () noexcept
{
    _instance = this;
}

pbr::ResourceHeap &ResourceHeap::Instance () noexcept
{
    return _instance->_resourceHeap;
}

} // namespace editor
