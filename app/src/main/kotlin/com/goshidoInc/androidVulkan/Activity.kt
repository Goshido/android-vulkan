package com.goshidoInc.androidVulkan


import android.os.Bundle
import android.util.Log
import android.view.KeyEvent
import android.view.MotionEvent
import android.view.SurfaceHolder
import android.view.View


internal class Activity : android.app.Activity (), SurfaceHolder.Callback2
{
    companion object
    {
        const val TAG = "android_vulkan::C++"
    }

    private var isWindowCreated = false

    private var dPad = DPad ()

    private val leftStick = LeftStick ()
    private val rightStick = RightStick ()

    private val leftTrigger = LeftTrigger ()
    private val rightTrigger = RightTrigger ()

    override fun onCreate ( savedInstanceState : Bundle? )
    {
        super.onCreate ( savedInstanceState )

        Log.d ( TAG, "~~~ onCreate" )

        window.takeSurface ( this )

        val view = View ( this )
        setContentView ( view )
        view.requestFocus ()
    }

    override fun onDestroy ()
    {
        isWindowCreated = false

        Log.d ( TAG, "~~~ onDestroy" )
        // TODO call native.

        super.onDestroy ()
    }

    override fun onGenericMotionEvent ( event : MotionEvent? ) : Boolean
    {
        if ( event == null )
            return false

        Log.d ( TAG, "~~~ onGenericMotionEvent" )

        leftStick.sync ( event )
        rightStick.sync ( event )

        leftTrigger.sync ( event )
        rightTrigger.sync ( event )

        dPad.sync ( event )

        // TODO call native.
        return true
    }

    override fun onKeyDown ( keyCode : Int, event : KeyEvent? ) : Boolean
    {
        Log.d ( TAG, "~~~ onKeyDown $keyCode" )
        // TODO call native.
        return true
    }

    override fun onKeyUp ( keyCode : Int, event : KeyEvent? ) : Boolean
    {
        Log.d ( TAG, "~~~ onKeyUp $keyCode" )
        // TODO call native.
        return true
    }

    override fun surfaceChanged ( holder : SurfaceHolder, format : Int, width : Int, height : Int )
    {
        // NOTHING
        Log.d ( TAG, "~~~ surfaceChanged" )
    }

    override fun surfaceCreated ( holder : SurfaceHolder )
    {
        if ( !isWindowCreated )
        {
            // TODO tell native about window.
            Log.d ( TAG, "~~~ window created" )
            isWindowCreated = true
        }

        Log.d ( TAG, "~~~ surfaceCreated" )
        // TODO tell native about surface.
    }

    override fun surfaceDestroyed ( holder : SurfaceHolder )
    {
        Log.d ( TAG, "~~~ surfaceDestroyed" )
        // TODO call native.
    }

    override fun surfaceRedrawNeeded ( holder : SurfaceHolder )
    {
        Log.d ( TAG, "~~~ surfaceRedrawNeeded" )
        // NOTHING
    }
}
