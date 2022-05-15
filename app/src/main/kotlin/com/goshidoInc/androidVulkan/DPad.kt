package com.goshidoInc.androidVulkan


import android.view.MotionEvent


internal class DPad : Input2D ( "DPAD" )
{
    private data class State
    (
        var down : Boolean = false,
        var left : Boolean = false,
        var right : Boolean = false,
        var up : Boolean = false
    )

    private var oldState = State ()
    private var currentState = State ()

    override fun sync ( event : MotionEvent )
    {
        x = event.getAxisValue ( MotionEvent.AXIS_HAT_X )
        y = event.getAxisValue ( MotionEvent.AXIS_HAT_Y )

        yell ()
    }
}
