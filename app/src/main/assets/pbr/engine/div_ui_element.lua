require "av://engine/ui_element.lua"


local DIVUIElement = {}

-- Engine method
local function AppendChildElement ( self, element )
    if not self._childs then
        self._childs = {}
    end

    table.insert ( self._childs, element )

    if not self._text and element._type == eObjectType.TextUIElement then
        self._text = element
    end
end

-- Metamethods
local mt = {
    __gc = function ( self )
        av_DIVUIElementCollectGarbage ( self._handle )
    end
}

-- This function is exported to C++ side.
function RegisterDIVUIElement ( handle )
    local obj = UIElement ( eObjectType.DIVUIElement )

    -- Data
    obj._handle = handle

    -- Engine method
    obj.AppendChildElement = AppendChildElement

    return setmetatable ( obj, mt )
end

-- Module contract
return nil
