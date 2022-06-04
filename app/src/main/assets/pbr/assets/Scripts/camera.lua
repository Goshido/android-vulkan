require "av://engine/script_component.lua"


-- Constants
local FOVY = math.rad ( 60.0 )
local ZNEAR = 1.0e-1
local ZFAR = 1.0e+4

local OFFSET = GXVec3 ()
OFFSET:Init ( 512.0, 128.0, 0.0 )

local SPEED = 2.0e-2
local TOLERANCE = 1.0e-4

-- Class declaration
local Camera = {}

-- Engine event handlers
local function OnActorConstructed ( self, actor )
    local camera = actor:FindComponent ( "Camera" )
    camera:SetProjection ( FOVY, g_scene:GetRenderTargetAspectRatio (), ZNEAR, ZFAR )
    self._cameraComponent = camera
    g_scene:SetActiveCamera ( camera )
end

local function OnRenderTargetChanged ( self )
    self._cameraComponent:SetProjection ( FOVY, g_scene:GetRenderTargetAspectRatio (), ZNEAR, ZFAR )
end

local function Tracking ( self, deltaTime )
    local target = GXVec3 ()
    self._tracker:GetLocation ( target )

    local ideal = GXVec3 ()
    ideal:SumScaled ( OFFSET, g_scene:GetPhysicsToRendererScaleFactor (), target )

    local localMatrix = self._localMatrix
    local loc = GXVec3 ()
    localMatrix:GetW ( loc )

    local delta = GXVec3 ()
    delta:Subtract ( ideal, loc )

    local length = delta:Length ()

    if length < TOLERANCE then
        return
    end

    loc:SumScaled ( loc, length * SPEED * deltaTime, delta )
    localMatrix:SetW ( loc )

    self._cameraComponent:SetLocal ( localMatrix )
end

local function FindTracker ( self, deltaTime )
    local mario = g_scene:FindActor ( "Mario" )

    if not mario then
        return
    end

    local tracker = mario:FindComponent ( "Collider" )

    local target = GXVec3 ()
    tracker:GetLocation ( target )
    target:MultiplyScalar ( target, g_scene:GetPhysicsToRendererScaleFactor () )
    self._localMatrix:SetW ( target )

    self._tracker = tracker

    -- Switching from finding state to tracking state.
    self.OnPostPhysics = Tracking
end

-- Metamethods
local function Constructor ( self, handle, params )
    local obj = ScriptComponent ( handle )

    -- Data
    obj._localMatrix = GXMat4 ()
    obj._localMatrix:RotationY ( math.rad ( -90.0 ) )

    -- Engine events
    obj.OnActorConstructed = OnActorConstructed
    obj.OnPostPhysics = FindTracker
    obj.OnRenderTargetChanged = OnRenderTargetChanged

    return obj
end

setmetatable ( Camera, { __call = Constructor } )

-- Module function: fabric callable for Camera class.
return Camera
