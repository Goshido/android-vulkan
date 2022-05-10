require "av://engine/script_component.lua"


-- Constants
local ZNEAR = 1.0e-1
local ZFAR = 1.0e+4
local FOVY = math.rad ( 60.0 )
local SPEED = 1.0

local OFFSET = GXVec3 ()
OFFSET:Init ( 0.0, 10.0, 0.0 )

local ROTATION_AXIS = GXVec3 ()
ROTATION_AXIS:Init ( 1.0, 0.0, 0.0 )

local LOCATION = GXVec3 ()
LOCATION:Init ( 489.852417, 197.398102, 83.105896 )

local Camera = {}

-- Engine events
local function OnActorConstructed ( self, actor )
    local camera = actor:FindComponent ( "Camera" )
    camera:SetProjection ( FOVY, g_scene:GetRenderTargetAspectRatio (), ZNEAR, ZFAR )
    self._cameraComponent = camera
    g_scene:SetActiveCamera ( camera )
end

local function OnPostPhysics ( self, deltaTime )
    if not self:FindTarget () then
        return
    end

    self._slider = self._slider + SPEED * deltaTime

    local rotation = self._rotation
    rotation:FromAxisAngle ( ROTATION_AXIS, self._slider )

    local tmp0 = self._tmp0
    rotation:TransformFast ( tmp0, OFFSET )

    local tmp1 = self._tmp1
    tmp1:Sum ( LOCATION, tmp0 )

    local transform = self._transform
    transform:SetW ( tmp1 )

    self._cameraComponent:SetLocal ( transform )
end

local function OnRenderTargetChanged ( self )
    self._cameraComponent:SetProjection ( FOVY, g_scene:GetRenderTargetAspectRatio (), ZNEAR, ZFAR )
end

-- Methods
local function FindTarget ( self )
    -- TODO
    return true
end

-- Metamethods
local function Constructor ( self, handle, params )
    local obj = ScriptComponent ( handle )

    local transform = GXMat4 ()
    transform:RotationY ( math.rad ( -90.0 ) )

    -- Data
    obj._cameraComponent = nil
    obj._rotation = GXQuat ()
    obj._slider = 0.0
    obj._target = nil
    obj._tmp0 = GXVec3 ()
    obj._tmp1 = GXVec3 ()
    obj._transform = transform

    -- Engine events
    obj.OnActorConstructed = OnActorConstructed
    obj.OnPostPhysics = OnPostPhysics
    obj.OnRenderTargetChanged = OnRenderTargetChanged

    -- Methods
    obj.FindTarget = FindTarget

    return obj
end

setmetatable ( Camera, { __call = Constructor } )

-- Module function: fabric callable for Camera class.
return Camera
