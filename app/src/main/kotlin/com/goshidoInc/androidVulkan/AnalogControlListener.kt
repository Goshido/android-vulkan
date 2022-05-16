package com.goshidoInc.androidVulkan


internal sealed interface AnalogControlListener
{
    fun onLeftStick ( x : Float, y : Float )
    fun onRightStick ( x : Float, y : Float )

    fun onLeftTrigger ( value : Float )
    fun onRightTrigger ( value : Float )
}
