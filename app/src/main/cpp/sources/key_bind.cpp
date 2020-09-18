#include <key_bind.h>


namespace android_vulkan {

KeyBind::KeyBind ():
    _context ( nullptr ),
    _handler ( nullptr )
{
    // NOTHING
}

void KeyBind::Init ( void* context, KeyHandler handler )
{
    _context = context;
    _handler = handler;
}

void KeyBind::Reset ()
{
    _context = nullptr;
    _handler = nullptr;
}

} // namespace android_vulkan
