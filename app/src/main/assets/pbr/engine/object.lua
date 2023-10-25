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
    AnimationGraph = MakeUniqueID (),
    AnimationPlayerNode = MakeUniqueID (),
    BitField = MakeUniqueID (),
    CameraComponent = MakeUniqueID (),
    DIVUIElement = MakeUniqueID (),
    GXMat3 = MakeUniqueID (),
    GXMat4 = MakeUniqueID (),
    GXQuat = MakeUniqueID (),
    GXVec3 = MakeUniqueID (),
    GXVec4 = MakeUniqueID (),
    ImageUIElement = MakeUniqueID (),
    Material = MakeUniqueID (),
    RigidBodyComponent = MakeUniqueID (),
    Scene = MakeUniqueID (),
    ScriptComponent = MakeUniqueID (),
    SkeletalMeshComponent = MakeUniqueID (),
    SoundEmitterGlobalComponent = MakeUniqueID (),
    SoundEmitterSpatialComponent = MakeUniqueID (),
    StaticMeshComponent = MakeUniqueID (),
    TextUIElement = MakeUniqueID (),
    TransformComponent = MakeUniqueID (),
    UILayer = MakeUniqueID (),
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

------------------------------------------------------------------------------------------------------------------------

function IsComponent ( objectType )
    return objectType == eObjectType.CameraComponent or
        objectType == eObjectType.RigidBodyComponent or
        objectType == eObjectType.ScriptComponent or
        objectType == eObjectType.StaticMeshComponent or
        objectType == eObjectType.TransformComponent or
        objectType == eObjectType.SoundEmitterGlobalComponent or
        objectType == eObjectType.SoundEmitterSpatialComponent or
        objectType == eObjectType.SkeletalMeshComponent
end

------------------------------------------------------------------------------------------------------------------------

-- Module contract
return nil
