package com.goshidoInc.androidVulkan


import android.util.Log
import android.view.MotionEvent


internal sealed class Input2D ( private val PREFIX : String )
{
    protected var x : Float = 0.0F
    protected var y : Float = 0.0F

    abstract fun sync ( event : MotionEvent )

    protected fun yell ()
    {
        Log.d ( Activity.TAG, "     $PREFIX: $x $y" )
    }
}
