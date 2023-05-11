require "av://engine/object.lua"


local DIVUIElement = {}

-- Methods
local function Hide ( self )
    assert ( type ( self ) == "table" and self._type == eObjectType.DIVUIElement,
        [[DIVUIElement:Hide - Calling not via ":" syntax.]]
    )

    av_DIVUIElementHide ( self._handle )
end

local function Show ( self )
    assert ( type ( self ) == "table" and self._type == eObjectType.DIVUIElement,
        [[DIVUIElement:Show - Calling not via ":" syntax.]]
    )

    av_DIVUIElementShow ( self._handle )
end

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
    local obj = Object ( eObjectType.DIVUIElement )

    -- Data
    obj._handle = handle

    -- Engine method
    obj.AppendChildElement = AppendChildElement

    -- Methods
    obj.Hide = Hide
    obj.Show = Show

    return setmetatable ( obj, mt )
end

-- Module contract
return nil
