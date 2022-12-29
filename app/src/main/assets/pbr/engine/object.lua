-- IIFE pattern
local MakeUniqueID = ( function ()
    local c = -1

    return function ()
        c = c + 1
        return c
    end
end ) ()

eObjectType = {
    Actor = MakeUniqueID (),
    CameraComponent = MakeUniqueID (),
    GXMat3 = MakeUniqueID (),
    GXMat4 = MakeUniqueID (),
    GXQuat = MakeUniqueID (),
    GXVec3 = MakeUniqueID (),
    GXVec4 = MakeUniqueID (),
    Material = MakeUniqueID (),
    RigidBodyComponent = MakeUniqueID (),
    Scene = MakeUniqueID (),
    ScriptComponent = MakeUniqueID (),
    StaticMeshComponent = MakeUniqueID (),
    TransformComponent = MakeUniqueID (),
    SoundEmitterGlobalComponent = MakeUniqueID (),
    Unknown = MakeUniqueID ()
}

------------------------------------------------------------------------------------------------------------------------

Object = {
    _type = eObjectType.Unknown
}

-- Metamethods
local function Constructor ( self, objectType )
    return {
        _type = objectType
    }
end

setmetatable ( Object, { __call = Constructor } )

-- Module contract
return nil
