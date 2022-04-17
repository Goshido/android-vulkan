require "av://engine/object.lua"


Component = {}

-- methods
local function GetName ( self )
    return av_ComponentGetName ( self._handle )
end

-- metamethods
local function Constructor ( self, objectType, handle )
    local obj = Object ( objectType )

    -- data
    obj._handle = handle

    -- methods
    obj.GetName = GetName

    return obj
end

setmetatable ( Component, { __call = Constructor } )

-- module contract
return nil
