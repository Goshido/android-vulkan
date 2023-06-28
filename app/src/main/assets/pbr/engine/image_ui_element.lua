require "av://engine/ui_element.lua"


local ImageUIElement = {}

-- Metamethods
local mt = {
    __gc = function ( self )
        av_UIElementCollectGarbage ( self._handle )
    end
}

-- This function is exported to C++ side.
function RegisterImageUIElement ( handle )
    local obj = UIElement ( eObjectType.ImageUIElement )

    -- Data
    obj._handle = handle

    return setmetatable ( obj, mt )
end

-- Module contract
return nil
