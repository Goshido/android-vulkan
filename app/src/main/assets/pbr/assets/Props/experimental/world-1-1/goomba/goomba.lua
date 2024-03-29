require "av://engine/bit_field.lua"
require "av://engine/script_component.lua"
require "av://engine/sound_channel.lua"


-- Constants
local SPEED = 1.3889

local GRAVITY = GXVec3 ()
GRAVITY:Init ( 0.0, -9.81, 0.0 )

local LANDING_FACTOR = -1.0e-2
local SCORE = 100

local SQUASH_VELOCITY_FACTOR = GXVec3 ()
SQUASH_VELOCITY_FACTOR:Init ( 1.0, -1.0, 1.0 )

-- Class declaration
local Goomba = {}

-- Methods
local function AddScore ( self )
    g_scene:FindActor ( "Mastermind" ):FindComponent ( "LevelScript" ):AddScore ( SCORE )
end

local function CheckMoveSensor ( self, size, offset, localMatrix, origin, velocity )
    local alpha = GXVec3 ()
    localMatrix:MultiplyAsNormal ( alpha, offset )
    alpha:Sum ( alpha, origin )

    local m = GXMat4 ()
    m:Clone ( localMatrix )
    m:SetW ( alpha )

    local sweep = g_scene:SweepTestBox ( m, size, self._collisionGroups )
    local count = sweep._count

    if count == 0 then
        return
    end

    local react = false
    local bodies = sweep._bodies
    local kinematic = self._bodyKinematic

    for i = 1, count do
        if bodies[ i ] ~= kinematic then
            react = true
            break
        end
    end

    if react and velocity:DotProduct ( self._horizontalVelocity ) < -0.5 then
        self._horizontalVelocity:Clone ( velocity )
    end
end

local function CheckTopSensor ( self, localMatrix, origin )
    local mario = self._mario

    local p = GXVec3 ()
    p:Sum ( origin, self._sensorTopOffset )
    local m = GXMat4 ()
    m:Clone ( localMatrix )
    m:SetW ( p )

    local result = g_scene:OverlapTestBoxBox ( m, self._sensorTopSize,
        mario:GetShapeTransform (),
        mario:GetShapeSize ()
    )

    if not result then
        return
    end

    mario:Hit ( SQUASH_VELOCITY_FACTOR )
    self:AddScore ()
    self:SpawnSound ()

    self._actor:Destroy ()
end

local function GetCenter ( self, min, max )
    local diff = GXVec3 ()
    diff:Subtract ( max, min )

    local result = GXVec3 ()
    result:SumScaled ( min, 0.5, diff )
    return result
end

local function GetOrigin ( self, origin, actor, component )
    local t = GXMat4 ()
    actor:FindComponent ( component ):GetTransform ( t )
    t:GetW ( origin )
end

local function GetSensorOffset ( self, parentOrigin, actor, min, max )
    local result = GXVec3 ()
    result:Subtract ( self:GetCenter ( actor, min, max ), parentOrigin )
    result:MultiplyScalar ( result, g_scene:GetRendererToPhysicsScaleFactor () )
    return result
end

local function GetSensorSize ( self, min, max )
    local result = GXVec3 ()
    result:Subtract ( max, min )
    result:MultiplyScalar ( result, g_scene:GetRendererToPhysicsScaleFactor () )
    return result
end

local function SpawnSound ( self )
    local location = GXVec3 ()
    self._bodyKinematic:GetLocation ( location )

    local actor = Actor ( "GoombaSound" )

    actor:AppendComponent (
        ScriptComponent ( "Script",
            "av://assets/Props/experimental/world-1-1/scripts/single_shot_sound.lua",

            {
                _duration = 1.0,
                _distance = 45.0,
                _location = location,
                _soundAsset = "pbr/assets/Props/experimental/world-1-1/sounds/kick.wav",
                _soundChannel = eSoundChannel.SFX,
                _volume = 1.0
            }
        )
    )

    g_scene:AppendActor ( actor )
end

-- Engine event handlers
local function OnActorConstructed ( self, actor )
    self._actor = actor

    local min = GXVec3 ()
    local max = GXVec3 ()

    self:GetOrigin ( min, actor, "BodyMin" )
    self:GetOrigin ( max, actor, "BodyMax" )
    self._bodySize = self:GetSensorSize ( min, max )
    self._bodyKinematic = actor:FindComponent ( "BodyKinematic" )

    local origin = self:GetCenter ( min, max )

    self._localMatrix = GXMat4 ()
    self._localMatrix:Identity ()
    self._localMatrix:SetW ( origin )

    self._mesh = actor:FindComponent ( "Mesh" )

    self:GetOrigin ( min, actor, "SensorTopMin" )
    self:GetOrigin ( max, actor, "SensorTopMax" )
    self._sensorTopSize = self:GetSensorSize ( min, max )
    self._sensorTopOffset = self:GetSensorOffset ( origin, min, max )

    self:GetOrigin ( min, actor, "SensorLeftMin" )
    self:GetOrigin ( max, actor, "SensorLeftMax" )
    self._sensorLeftSize = self:GetSensorSize ( min, max )
    self._sensorLeftOffset = self:GetSensorOffset ( origin, min, max )

    self:GetOrigin ( min, actor, "SensorRightMin" )
    self:GetOrigin ( max, actor, "SensorRightMax" )
    self._sensorRightSize = self:GetSensorSize ( min, max )
    self._sensorRightOffset = self:GetSensorOffset ( origin, min, max )

    self._horizontalVelocity:Init ( 0.0, 0.0, SPEED )
    self._verticalVelocity:Init ( 0.0, 0.0, 0.0 )

    local collisionGroups = BitField ()
    collisionGroups:SetAllBits ()
    self._collisionGroups = collisionGroups
end

local function OnPostPhysicsAct ( self, deltaTime )
    local origin = GXVec3 ()
    local kinematic = self._bodyKinematic
    kinematic:GetLocation ( origin )

    local m = GXMat4 ()
    m:Clone ( self._localMatrix )
    m:SetW ( origin )

    local p = g_scene:GetPenetrationBox ( m, self._bodySize, self._collisionGroups )
    local count = p._count
    local pns = p._penetrations

    for i = 1, count do
        local pn = pns[ i ];

        if pn._body ~= kinematic then
            origin:SumScaled ( origin, pn._depth, pn._normal )

            if GRAVITY:DotProduct ( pn._normal ) < LANDING_FACTOR then
                self._verticalVelocity:Init ( 0.0, 0.0, 0.0 )
            end
        end
    end

    kinematic:SetLocation ( origin )

    origin:MultiplyScalar ( origin, g_scene:GetPhysicsToRendererScaleFactor () )
    m:SetW ( origin )
    self._mesh:SetLocal ( m )
end

local function OnPostPhysicsIdle ( self, deltaTime )
    -- NOTHING
end

local function OnPrePhysicsAct ( self, deltaTime )
    local localMatrix = self._localMatrix
    kinematic = self._bodyKinematic

    local origin = GXVec3 ()
    kinematic:GetLocation ( origin )

    local velocity = GXVec3 ()
    velocity:Init ( 0.0, 0.0, -SPEED )
    self:CheckMoveSensor ( self._sensorRightSize, self._sensorRightOffset, localMatrix, origin, velocity )

    velocity:Reverse ()
    self:CheckMoveSensor ( self._sensorLeftSize, self._sensorLeftOffset, localMatrix, origin, velocity )

    self:CheckTopSensor ( localMatrix, origin )

    local vertical = self._verticalVelocity
    vertical:SumScaled ( vertical, deltaTime, GRAVITY )

    local velocity = GXVec3 ()
    velocity:Sum ( self._horizontalVelocity, vertical )

    kinematic:SetVelocityLinear ( velocity, true )
end

local function OnPrePhysicsSeek ( self, deltaTime )
    local mario = g_scene:FindActor ( "Mario" )

    if mario == nil then
        return
    end

    self._mario = mario:FindComponent ( "Script" )
    self.OnPostPhysics = OnPostPhysicsAct
    self.OnPrePhysics = OnPrePhysicsAct
end

-- Metamethods
local function Constructor ( self, handle, params )
    local obj = ScriptComponent ( handle )

    -- Data
    obj._horizontalVelocity = GXVec3 ()
    obj._verticalVelocity = GXVec3 ()

    -- Methods
    obj.AddScore = AddScore
    obj.CheckMoveSensor = CheckMoveSensor
    obj.CheckTopSensor = CheckTopSensor
    obj.GetCenter = GetCenter
    obj.GetOrigin = GetOrigin
    obj.GetSensorOffset = GetSensorOffset
    obj.GetSensorSize = GetSensorSize
    obj.SpawnSound = SpawnSound

    -- Engine events
    obj.OnActorConstructed = OnActorConstructed
    obj.OnPostPhysics = OnPostPhysicsIdle
    obj.OnPrePhysics = OnPrePhysicsSeek

    return obj
end

setmetatable ( Goomba, { __call = Constructor } )

-- Module function: fabric callable for Goomba class
return Goomba
