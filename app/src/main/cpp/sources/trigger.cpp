#include <trigger.h>


namespace android_vulkan {

Trigger::Trigger ():
    _context ( nullptr ),
    _handler ( nullptr ),
    _pushCurrent ( 0.0F ),
    _pushOld ( 0.0F )
{
    // TODO
}

void Trigger::Bind ( void* context, TriggerHandler handler )
{
    _context = context;
    _handler = handler;
    _pushCurrent = _pushOld = 0.0F;
}

void Trigger::Unbind ()
{
    _context = nullptr;
    _handler = nullptr;
    _pushCurrent = _pushOld;
}

void Trigger::Execute ()
{
    if ( !_handler || _pushCurrent == _pushOld )
        return;

    _handler ( _context, _pushCurrent );
    _pushOld = _pushCurrent;
}

void Trigger::Update ( AInputEvent* event, int32_t axis )
{
    _pushCurrent = AMotionEvent_getAxisValue ( event, axis, 0U );
}

} // namespace android_vulkan
