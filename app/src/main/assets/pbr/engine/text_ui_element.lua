require "av://engine/ui_element.lua"


local TextUIElement = {}

-- Methods
local function SetColorHSV ( self, h, s, v, a )
    assert ( type ( self ) == "table" and self._type == eObjectType.TextUIElement,
        [[TextUIElement:SetColorHSV - Calling not via ":" syntax.]]
    )

    assert ( type ( h ) == "number", [[TextUIElement:SetColorHSV - "h" is not a number.]] )
    assert ( type ( s ) == "number", [[TextUIElement:SetColorHSV - "s" is not a number.]] )
    assert ( type ( v ) == "number", [[TextUIElement:SetColorHSV - "v" is not a number.]] )
    assert ( type ( a ) == "number", [[TextUIElement:SetColorHSV - "a" is not a number.]] )

    av_TextUIElementSetColorHSV ( self._handle, h, s, v, a )
end

local function SetColorRGB ( self, r, g, b, a )
    assert ( type ( self ) == "table" and self._type == eObjectType.TextUIElement,
        [[TextUIElement:SetColorRGB - Calling not via ":" syntax.]]
    )

    assert ( type ( r ) == "number", [[TextUIElement:SetColorRGB - "r" is not a number.]] )
    assert ( type ( g ) == "number", [[TextUIElement:SetColorRGB - "g" is not a number.]] )
    assert ( type ( b ) == "number", [[TextUIElement:SetColorRGB - "b" is not a number.]] )
    assert ( type ( a ) == "number", [[TextUIElement:SetColorRGB - "a" is not a number.]] )

    av_TextUIElementSetColorRGB ( self._handle, r, g, b, a )
end

local function SetText ( self, text )
    assert ( type ( self ) == "table" and self._type == eObjectType.TextUIElement,
        [[TextUIElement:SetText - Calling not via ":" syntax.]]
    )

    assert ( type ( text ) == "string", [[TextUIElement:SetText - "text" is not a string.]] )
    av_TextUIElementSetText ( self._handle, text )
end

-- Metamethods
local mt = {
    __gc = function ( self )
        av_UIElementCollectGarbage ( self._handle )
    end
}

-- This function is exported to C++ side.
function RegisterTextUIElement ( handle )
    local obj = UIElement ( eObjectType.TextUIElement )

    -- Data
    obj._handle = handle

    -- Methods
    obj.SetColorHSV = SetColorHSV
    obj.SetColorRGB = SetColorRGB
    obj.SetText = SetText

    return setmetatable ( obj, mt )
end

-- Module contract
return nil
