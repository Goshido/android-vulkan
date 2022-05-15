package com.goshidoInc.androidVulkan


import android.util.Log
import android.view.MotionEvent


internal sealed class Trigger ( private val AXIS : Int, private val PREFIX : String )
{
    protected var value : Float = 0.0F

    fun sync ( event : MotionEvent )
    {
        value = event.getAxisValue ( AXIS )

        Log.d ( Activity.TAG, "     $PREFIX: $value" )
    }
}

internal class LeftTrigger : Trigger ( MotionEvent.AXIS_LTRIGGER, "Left Trigger" )
internal class RightTrigger : Trigger ( MotionEvent.AXIS_RTRIGGER, "Right Trigger" )
