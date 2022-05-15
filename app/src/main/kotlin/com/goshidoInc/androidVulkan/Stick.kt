package com.goshidoInc.androidVulkan


import android.view.MotionEvent


internal class LeftStick : Input2D ( "LS" )
{
    override fun sync ( event : MotionEvent )
    {
        x = event.getAxisValue ( MotionEvent.AXIS_X )
        y = -event.getAxisValue ( MotionEvent.AXIS_Y )

        yell ()
    }
}

internal class RightStick : Input2D ( "RS" )
{
    override fun sync ( event : MotionEvent )
    {
        x = event.getAxisValue ( MotionEvent.AXIS_Z )
        y = -event.getAxisValue ( MotionEvent.AXIS_RZ )

        yell ()
    }
}
