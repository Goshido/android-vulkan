package com.goshidoInc.androidVulkan


import android.view.MotionEvent


internal sealed class Stick ( protected val listener : AnalogControlListener )
{
    protected data class State
    (
        val x : Float = 0.0F,
        val y : Float = 0.0F
    )

    protected var oldState = State ()

    abstract fun sync ( event : MotionEvent )
}

internal class LeftStick ( listener : AnalogControlListener ) : Stick ( listener )
{
    override fun sync ( event : MotionEvent )
    {
        val currentState = State ( event.getAxisValue ( MotionEvent.AXIS_X ),
            -event.getAxisValue ( MotionEvent.AXIS_Y )
        )

        if ( oldState == currentState )
            return

        oldState = currentState
        listener.onLeftStick ( currentState.x, currentState.y )
    }
}

internal class RightStick ( listener : AnalogControlListener ) : Stick ( listener )
{
    override fun sync ( event : MotionEvent )
    {
        val currentState = State ( event.getAxisValue ( MotionEvent.AXIS_Z ),
            -event.getAxisValue ( MotionEvent.AXIS_RZ )
        )

        if ( oldState == currentState )
            return

        oldState = currentState
        listener.onRightStick ( currentState.x, currentState.y )
    }
}
