require "av://engine/object.lua"


Component = {}

-- utils
local function IsComponent ( objectType )
    return objectType == eObjectType.ScriptComponent
end

-- methods
local function GetName ( self )
    assert ( type ( self ) == "table" and IsComponent ( self._type ),
        [[Component:GetName - Calling not via ":" syntax.]]
    )

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
