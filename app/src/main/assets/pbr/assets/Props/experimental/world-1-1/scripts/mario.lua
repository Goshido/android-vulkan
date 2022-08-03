require "av://engine/script_component.lua"


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
    local t = GXMat4 ()
    self._rigidBody:GetTransform ( t )
    return t
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
    self._rigidBody = actor:FindComponent ( "Collider" )
    self.OnInput = OnInputActive
    self.OnPrePhysics = OnPrePhysicsActive

    local min = self:GetOrigin ( actor, "Min" )
    local max = self:GetOrigin ( actor, "Max" )
    self._size = self:GetSensorSize ( min, max )
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

    -- Methods
    obj.GetOrigin = GetOrigin
    obj.GetSensorSize = GetSensorSize
    obj.GetShapeSize = GetShapeSize
    obj.GetShapeTransform = GetShapeTransform
    obj.Jump = Jump
    obj.Move = Move
    obj.QuitGame = QuitGame

    -- Engine events
    obj.OnActorConstructed = OnActorConstructed
    obj.OnInput = OnInputIdle
    obj.OnPrePhysics = OnPrePhysicsIdle

    return obj
end

setmetatable ( Mario, { __call = Constructor } )

-- Module function: fabric callable for Mario class.
return Mario
