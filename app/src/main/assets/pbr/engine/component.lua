require "av://engine/object.lua"


Component = {}

-- Utils
local function IsComponent ( objectType )
    return objectType == eObjectType.CameraComponent or
        objectType == eObjectType.RigidBodyComponent or
        objectType == eObjectType.ScriptComponent or
        objectType == eObjectType.StaticMeshComponent or
        objectType == eObjectType.TransformComponent or
        objectType == eObjectType.SoundEmitterGlobalComponent
end

-- Methods
local function GetName ( self )
    assert ( type ( self ) == "table" and IsComponent ( self._type ),
        [[Component:GetName - Calling not via ":" syntax.]]
    )

    return av_ComponentGetName ( self._handle )
end

-- Metamethods
local function Constructor ( self, objectType, handle )
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
