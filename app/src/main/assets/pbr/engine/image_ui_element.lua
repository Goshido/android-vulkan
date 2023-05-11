require "av://engine/object.lua"


local ImageUIElement = {}

-- Methods
local function Hide ( self )
    assert ( type ( self ) == "table" and self._type == eObjectType.ImageUIElement,
        [[ImageUIElement:Hide - Calling not via ":" syntax.]]
    )

    av_ImageUIElementHide ( self._handle )
end

local function Show ( self )
    assert ( type ( self ) == "table" and self._type == eObjectType.ImageUIElement,
        [[ImageUIElement:Show - Calling not via ":" syntax.]]
    )

    av_ImageUIElementShow ( self._handle )
end

-- Metamethods
local mt = {
    __gc = function ( self )
        av_ImageUIElementCollectGarbage ( self._handle )
    end
}

-- This function is exported to C++ side.
function RegisterImageUIElement ( handle )
    local obj = Object ( eObjectType.ImageUIElement )

    -- Data
    obj._handle = handle

    -- Methods
    obj.Hide = Hide
    obj.Show = Show

    return setmetatable ( obj, mt )
end

-- Module contract
return nil
