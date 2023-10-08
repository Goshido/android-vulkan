require "av://engine/object.lua"


UIElement = {}

-- Util
local function IsCorrectType ( self )
    if type ( self ) ~= "table" then
        return false
    end

    local t = self._type
    return t == eObjectType.DIVUIElement or t == eObjectType.TextUIElement or t == eObjectType.ImageUIElement
end

-- Methods
local function Hide ( self )
    assert ( IsCorrectType ( self ), [[UIElement:Hide - Calling not via ":" syntax.]] )
    av_UIElementHide ( self._handle )
end

local function IsVisible ( self )
    assert ( IsCorrectType ( self ), [[UIElement:IsVisible - Calling not via ":" syntax.]] )
    return av_UIElementIsVisible ( self._handle )
end

local function Show ( self )
    assert ( IsCorrectType ( self ), [[UIElement:Show - Calling not via ":" syntax.]] )
    av_UIElementShow ( self._handle )
end

-- Metamethods
local function Constructor ( self, elementType )
    local obj = Object ( elementType )

    -- Methods
    obj.Hide = Hide
    obj.IsVisible = IsVisible
    obj.Show = Show

    return obj
end

setmetatable ( UIElement, { __call = Constructor } )

-- Module contract
return nil
