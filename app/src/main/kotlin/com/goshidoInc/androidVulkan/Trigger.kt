package com.goshidoInc.androidVulkan


import android.view.MotionEvent


internal sealed class Trigger ( protected val listener : AnalogControlListener )
{
    protected var oldValue : Float = 0.0F

    abstract fun sync ( event : MotionEvent )
}

internal class LeftTrigger ( listener : AnalogControlListener ) : Trigger ( listener )
{
    override fun sync ( event : MotionEvent )
    {
        val value = event.getAxisValue ( MotionEvent.AXIS_LTRIGGER )

        if ( value == oldValue )
            return

        oldValue = value
        listener.onLeftTrigger ( value )
    }
}

internal class RightTrigger ( listener : AnalogControlListener ) : Trigger ( listener )
{
    override fun sync ( event : MotionEvent )
    {
        val value = event.getAxisValue ( MotionEvent.AXIS_RTRIGGER )

        if ( value == oldValue )
            return

        oldValue = value
        listener.onRightTrigger ( value )
    }
}
