require "av://engine/ui_element.lua"


local DIVUIElement = {}

-- Methods
local function GetTextElement ( self )
    return self._text
end

-- Engine method
local function AppendChildElement ( self, element )
    if not self._children then
        self._children = {}
    end

    table.insert ( self._children, element )

    if not self._text and element._type == eObjectType.TextUIElement then
        self._text = element
    end
end

-- Metamethods
local mt = {
    __gc = function ( self )
        av_UIElementCollectGarbage ( self._handle )
    end
}

-- This function is exported to C++ side.
function RegisterDIVUIElement ( handle )
    local obj = UIElement ( eObjectType.DIVUIElement )

    -- Data
    obj._handle = handle

    -- Methods
    obj.GetTextElement = GetTextElement

    -- Engine method
    obj.AppendChildElement = AppendChildElement

    return setmetatable ( obj, mt )
end

-- Module contract
return nil
