package com.goshidoInc.androidVulkan


import android.os.Bundle
import android.view.KeyEvent
import android.view.MotionEvent
import android.view.SurfaceHolder
import android.view.View


internal class Activity : android.app.Activity (), SurfaceHolder.Callback2, AnalogControlListener
{
    private companion object
    {
        init
        {
            System.loadLibrary ( "android-vulkan" )
        }
    }

    private var isWindowCreated = false

    private val dPad = DPad ( this )

    private val leftStick = LeftStick ( this )
    private val rightStick = RightStick ( this )

    private val leftTrigger = LeftTrigger ( this )
    private val rightTrigger = RightTrigger ( this )

    private external fun doCreate ()
    private external fun doDestroy ()
    private external fun doKeyDown ( keyCode : Int )
    private external fun doKeyUp ( keyCode : Int )
    private external fun doLeftStick ( x : Float, y : Float )
    private external fun doRightStick ( x : Float, y : Float )
    private external fun doLeftTrigger ( value : Float )
    private external fun doRightTrigger ( value : Float )
    private external fun doSurfaceCreated ()
    private external fun doSurfaceDestroyed ()
    private external fun doWindowCreated ()

    override fun onCreate ( savedInstanceState : Bundle? )
    {
        super.onCreate ( savedInstanceState )

        window.takeSurface ( this )

        val view = View ( this )
        setContentView ( view )
        view.requestFocus ()

        doCreate ()
    }

    override fun onDestroy ()
    {
        isWindowCreated = false
        doDestroy ()
        super.onDestroy ()
    }

    override fun onGenericMotionEvent ( event : MotionEvent? ) : Boolean
    {
        if ( event == null )
            return false

        leftStick.sync ( event )
        rightStick.sync ( event )

        leftTrigger.sync ( event )
        rightTrigger.sync ( event )

        dPad.sync ( event )
        return true
    }

    override fun onKeyDown ( keyCode : Int, event : KeyEvent? ) : Boolean
    {
        doKeyDown ( keyCode )
        return true
    }

    override fun onKeyUp ( keyCode : Int, event : KeyEvent? ) : Boolean
    {
        doKeyUp ( keyCode )
        return true
    }

    override fun onLeftStick ( x : Float, y : Float )
    {
        doLeftStick ( x, y )
    }

    override fun onRightStick ( x : Float, y : Float )
    {
        doRightStick ( x, y )
    }

    override fun onLeftTrigger ( value : Float )
    {
        doLeftTrigger ( value )
    }

    override fun onRightTrigger ( value : Float )
    {
        doRightTrigger ( value )
    }

    override fun surfaceChanged ( holder : SurfaceHolder, format : Int, width : Int, height : Int )
    {
        // NOTHING
    }

    override fun surfaceCreated ( holder : SurfaceHolder )
    {
        if ( !isWindowCreated )
        {
            isWindowCreated = true
            doWindowCreated ()
        }

        doSurfaceCreated ()
    }

    override fun surfaceDestroyed ( holder : SurfaceHolder )
    {
        doSurfaceDestroyed ()
    }

    override fun surfaceRedrawNeeded ( holder : SurfaceHolder )
    {
        // NOTHING
    }
}
