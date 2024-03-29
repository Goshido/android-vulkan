require "av://engine/object.lua"


Component = {}

-- Methods
local function GetName ( self )
    assert ( type ( self ) == "table" and IsComponent ( self._type ),
        [[Component:GetName - Calling not via ":" syntax.]]
    )

    return av_ComponentGetName ( self._handle )
end

-- Metamethods
local function Constructor ( self, objectType, handle )
    assert ( type ( handle ) == "userdata", [[Component:Constructor - "handle" is not native object.]] )
    local obj = Object ( objectType )

    -- Data
    obj._handle = handle

    -- Methods
    obj.GetName = GetName

    return obj
end

setmetatable ( Component, { __call = Constructor } )

-- Module contract
return nil
