package com.goshidoInc.androidVulkan


import android.util.Log
import android.view.KeyEvent
import android.view.MotionEvent


internal class DPad ( private val notify : KeyEvent.Callback )
{
    private data class State
    (
        var down : Boolean = false,
        var left : Boolean = false,
        var right : Boolean = false,
        var up : Boolean = false
    )

    private var currentState = State ()
    private var oldState = State ()

    fun sync ( event : MotionEvent )
    {
        val x = event.getAxisValue ( MotionEvent.AXIS_HAT_X )
        val y = event.getAxisValue ( MotionEvent.AXIS_HAT_Y )

        with ( currentState )
        {
            if ( x < 0.0F )
            {
                left = true
                right = false
            }
            else
            {
                left = false
                right = x > 0.0F
            }

            if ( y < 0.0F )
            {
                up = true
                down = false
            }
            else
            {
                up = false
                down = y > 0.0F
            }
        }

        if ( oldState == currentState )
            return

        val resolve = fun ( old : Boolean, new : Boolean, key : Int ) {
            if ( old == new )
                return

            if ( new )
            {
                notify.onKeyDown ( key, null )
                return
            }

            notify.onKeyUp ( key, null )
        }

        resolve ( oldState.down, currentState.down, KeyEvent.KEYCODE_DPAD_DOWN )
        resolve ( oldState.left, currentState.left, KeyEvent.KEYCODE_DPAD_LEFT )
        resolve ( oldState.right, currentState.right, KeyEvent.KEYCODE_DPAD_RIGHT )
        resolve ( oldState.up, currentState.up, KeyEvent.KEYCODE_DPAD_UP )

        oldState = currentState.copy ()
    }
}
