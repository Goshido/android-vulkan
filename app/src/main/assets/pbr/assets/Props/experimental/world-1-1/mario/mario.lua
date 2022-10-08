require "av://engine/actor.lua"
require "av://engine/material.lua"
require "av://engine/script_component.lua"
require "av://engine/static_mesh_component.lua"
require "av://engine/rigid_body_component.lua"


-- Constants
local DEAD_ZONE = 0.2
local DEAD_ZONE_FACTOR = 1.0 / ( 1.0 - DEAD_ZONE )

local JUMP_SPEED = GXVec3 ()
JUMP_SPEED:Init ( 0.0, 9.0, 0.0 )

local MOVE_FORCE = GXVec3 ()
MOVE_FORCE:Init ( 0.0, 0.0, 500.0 )

local MOVE_SPEED = 5.0

-- Class declaration
local Mario = {}

-- Methods
local function GetOrigin ( self, actor, component )
    local t = GXMat4 ()
    actor:FindComponent ( component ):GetTransform ( t )
    local origin = GXVec3 ()
    t:GetW ( origin )
    return origin
end

local function GetSensorSize ( self, min, max )
    local result = GXVec3 ()
    result:Subtract ( max, min )
    result:MultiplyScalar ( result, g_scene:GetRendererToPhysicsScaleFactor () )
    return result
end

local function GetShapeSize ( self )
    return self._size
end

local function GetShapeTransform ( self )
    return self._transform
end

local function Hit ( self, velocityMultiplier )
    local body = self._rigidBody
    local v = GXVec3 ()
    body:GetVelocityLinear ( v )
    v:MultiplyVector ( v, velocityMultiplier )
    body:SetVelocityLinear ( v, true )
end

local function Jump ( self )
    self._isJump = true
end

local function Move ( self, inputEvent )
    local v = inputEvent._x

    if ( math.abs ( v ) < DEAD_ZONE ) then
        self._move = 0.0
        return
    end

    -- Ternary operator
    self._move = DEAD_ZONE_FACTOR * ( v >= 0.0 and ( v - DEAD_ZONE ) or ( v + DEAD_ZONE ) )
end

-- Engine event handlers
local function OnInputActive ( self, inputEvent )
    if inputEvent._type == eEventType.LeftStick then
        self:Move ( inputEvent )
        return
    end

    if inputEvent._type == eEventType.KeyDown and inputEvent._key == eKey.A then
        self:Jump ()
        return
    end

    if inputEvent._type == eEventType.KeyUp and inputEvent._key == eKey.Home then
        self:QuitGame ()
    end
end

local function OnInputIdle ( self, inputEvent )
    -- NOTHING
end

local function OnPostPhysicsActive ( self, deltaTime )
    self._rigidBody:GetTransform ( self._transform )

    if self._xActor and deltaTime < 0.5 then
        self._xTimer = self._xTimer - deltaTime

        if self._xTimer < 0.0 then
            self._xActor:Destroy ()
            self._xActor = nil
            self._xTimer = nil
        end
    end
end

local function OnPostPhysicsIdle ( self, deltaTime )
    -- NOTHING
end

local function OnPrePhysicsActive ( self, deltaTime )
    local body = self._rigidBody

    local velocityLinear = GXVec3 ()
    body:GetVelocityLinear ( velocityLinear )

    if self._isJump then
        velocityLinear:Sum ( velocityLinear, JUMP_SPEED )
        body:SetVelocityLinear ( velocityLinear, true )
        self._isJump = false
    end

    local m = self._move
    local y = velocityLinear:GetZ ()
    local isMove = ( ( m < 0.0 ) and ( y >= -MOVE_SPEED ) ) or ( ( m > 0.0 ) and ( y < MOVE_SPEED ) )

    if not isMove then
        return
    end

    local force = GXVec3 ()
    force:MultiplyScalar ( MOVE_FORCE, m )

    local location = GXVec3 ()
    body:GetLocation ( location )

    body:AddForce ( force, location, true )
end

local function OnPrePhysicsIdle ( self, deltaTime )
    -- NOTHING
end

local function OnActorConstructed ( self, actor )
    local body = actor:FindComponent ( "Collider" )
    body:GetTransform ( self._transform )
    self._rigidBody = body

    self.OnInput = OnInputActive
    self.OnPostPhysics = OnPostPhysicsActive
    self.OnPrePhysics = OnPrePhysicsActive

    local min = self:GetOrigin ( actor, "Min" )
    local max = self:GetOrigin ( actor, "Max" )
    self._size = self:GetSensorSize ( min, max )

    local xMesh = StaticMeshComponent ( "xMesh", "pbr/assets/System/Default.mesh2" )
    xMesh:SetMaterial ( Material ( "pbr/assets/Props/experimental/world-1-1/sensors/Sensor.mtl" ) )

    local offset = GXVec3 ()
    offset:Init ( 0.0, 3.0, 0.0 )

    local location = GXVec3 ()
    self._transform:GetW ( location )
    location:Sum ( location, offset )

    local xRigidBody = RigidBodyComponent ( "xRigidBody" )
    xRigidBody:SetLocation ( location )
    xRigidBody:SetShapeSphere ( 0.5, true )

    local xScript = ScriptComponent ( "xScript", "av://assets/Props/experimental/world-1-1/debris/debris.lua", nil )

    local xActor = Actor ( "xActor" )
    xActor:AppendComponent ( xMesh )
    xActor:AppendComponent ( xRigidBody )
    xActor:AppendComponent ( xScript )
    g_scene:AppendActor ( xActor )

    self._xActor = xActor
    self._xTimer = 5.0
end

local function QuitGame ( self )
    g_scene:Quit ()
end

-- Metamethods
local function Constructor ( self, handle, params )
    local obj = ScriptComponent ( handle )

    -- Data
    obj._isJump = false
    obj._move = 0.0
    obj._transform = GXMat4 ()

    -- Methods
    obj.GetOrigin = GetOrigin
    obj.GetSensorSize = GetSensorSize
    obj.GetShapeSize = GetShapeSize
    obj.GetShapeTransform = GetShapeTransform
    obj.Hit = Hit
    obj.Jump = Jump
    obj.Move = Move
    obj.QuitGame = QuitGame

    -- Engine events
    obj.OnActorConstructed = OnActorConstructed
    obj.OnInput = OnInputIdle
    obj.OnPostPhysics = OnPostPhysicsIdle
    obj.OnPrePhysics = OnPrePhysicsIdle

    return obj
end

setmetatable ( Mario, { __call = Constructor } )

-- Module function: fabric callable for Mario class.
return Mario
