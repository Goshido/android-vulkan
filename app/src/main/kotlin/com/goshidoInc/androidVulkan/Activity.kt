package com.goshidoInc.androidVulkan


import android.content.res.AssetManager
import android.os.Bundle
import android.view.KeyEvent
import android.view.MotionEvent
import android.view.Surface
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

    private val dPad = DPad ( this )

    private val leftStick = LeftStick ( this )
    private val rightStick = RightStick ( this )

    private val leftTrigger = LeftTrigger ( this )
    private val rightTrigger = RightTrigger ( this )

    private external fun doCreate ( assetManager : AssetManager )
    private external fun doDestroy ()
    private external fun doKeyDown ( keyCode : Int )
    private external fun doKeyUp ( keyCode : Int )
    private external fun doLeftStick ( x : Float, y : Float )
    private external fun doRightStick ( x : Float, y : Float )
    private external fun doLeftTrigger ( value : Float )
    private external fun doRightTrigger ( value : Float )
    private external fun doSurfaceCreated ( surface : Surface )
    private external fun doSurfaceDestroyed ()

    override fun onCreate ( savedInstanceState : Bundle? )
    {
        super.onCreate ( savedInstanceState )

        window.takeSurface ( this )

        // Using legacy API for easy auto entering to fullscreen mode after swipes.
        @Suppress ( "DEPRECATION" )
        window.decorView.systemUiVisibility = View.SYSTEM_UI_FLAG_FULLSCREEN or
            View.SYSTEM_UI_FLAG_HIDE_NAVIGATION or
            View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY

        doCreate ( assets )
    }

    override fun onDestroy ()
    {
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
        if ( event != null && event.repeatCount == 0 )
            doKeyDown ( keyCode )

        return true
    }

    override fun onKeyUp ( keyCode : Int, event : KeyEvent? ) : Boolean
    {
        if ( event != null && event.repeatCount == 0 )
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
        doSurfaceCreated ( holder.surface )
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
