#include <key_bind.h>


namespace android_vulkan {

KeyBind::KeyBind () noexcept:
    _context ( nullptr ),
    _handler ( nullptr )
{
    // NOTHING
}

void KeyBind::Init ( void* context, KeyHandler handler ) noexcept
{
    _context = context;
    _handler = handler;
}

void KeyBind::Reset () noexcept
{
    _context = nullptr;
    _handler = nullptr;
}

} // namespace android_vulkan
