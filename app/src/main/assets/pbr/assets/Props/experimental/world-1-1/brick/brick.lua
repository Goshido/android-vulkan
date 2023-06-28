require "av://engine/material.lua"
require "av://engine/scene.lua"
require "av://engine/sound_channel.lua"


-- Constants
local DEBRIS_RADIUS = 0.2
local DEBRIS_VELOCITY_FACTOR = 2.5e-1

local SCORE = 200

local VELOCITY_MULTIPLIER = GXVec3 ()
VELOCITY_MULTIPLIER:Init ( 1.0, -0.75, 1.0 )


-- Class declaration
local BrickExt = {}

-- Methods
local function AddScore ( self )
    g_scene:FindActor ( "Mastermind" ):FindComponent ( "LevelScript" ):AddScore ( SCORE )
end

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

local function GetSensorTransform ( self, min, max )
    local diff = GXVec3 ()
    diff:Subtract ( max, min )

    local origin = GXVec3 ()
    origin:SumScaled ( min, 0.5, diff )
    origin:MultiplyScalar ( origin, g_scene:GetRendererToPhysicsScaleFactor () )

    local result = GXMat4 ()
    result:Identity ()
    result:SetW ( origin )
    return result
end

local function SpawnDebris ( self, debrisBegin, debrisEnd )
    local brickActor = self._actor

    local mesh = StaticMeshComponent ( "Mesh", "pbr/assets/Props/experimental/world-1-1/debris/debris.mesh2" )
    mesh:SetMaterial ( Material ( "pbr/assets/Props/experimental/world-1-1/brick/brick.mtl" ) )

    local rigidBody = RigidBodyComponent ( "RigidBody" )

    local location = self:GetOrigin ( brickActor, debrisBegin )
    local velocity = GXVec3 ()
    velocity:Subtract ( self:GetOrigin ( brickActor, debrisEnd ), location )
    velocity:MultiplyScalar ( velocity, DEBRIS_VELOCITY_FACTOR )
    rigidBody:SetVelocityLinear ( velocity, true )

    location:MultiplyScalar ( location, g_scene:GetRendererToPhysicsScaleFactor () )
    rigidBody:SetLocation ( location )

    rigidBody:SetShapeSphere ( DEBRIS_RADIUS, true )

    local debrisActor = Actor ( "Debris" )
    debrisActor:AppendComponent ( mesh )
    debrisActor:AppendComponent ( rigidBody )

    debrisActor:AppendComponent (
        ScriptComponent ( "Script", "av://assets/Props/experimental/world-1-1/debris/debris.lua", nil )
    )

    g_scene:AppendActor ( debrisActor )
end

local function SpawnSound ( self )
    local location = self:GetOrigin ( self._actor, "DebrisBegin001" )
    location:MultiplyScalar ( location, g_scene:GetRendererToPhysicsScaleFactor () )

    local actor = Actor ( "BrickSound" )

    actor:AppendComponent (
        ScriptComponent ( "Script",
            "av://assets/Props/experimental/world-1-1/scripts/single_shot_sound.lua",

            {
                _duration = 1.0,
                _distance = 45.0,
                _location = location,
                _soundAsset = "pbr/assets/Props/experimental/world-1-1/debris/break-block.wav",
                _soundChannel = eSoundChannel.SFX,
                _volume = 1.0
            }
        )
    )

    g_scene:AppendActor ( actor )
end

-- Engine event handlers
local function OnActorConstructed ( self, actor )
    local min = self:GetOrigin ( actor, "SensorMin" )
    local max = self:GetOrigin ( actor, "SensorMax" )
    self._sensorSize = self:GetSensorSize ( min, max )
    self._sensorTransform = self:GetSensorTransform ( min, max )
    self._actor = actor
end

local function OnPrePhysicsMonitor ( self, deltaTime )
    local mario = self._mario

    local result = g_scene:OverlapTestBoxBox ( self._sensorTransform,
        self._sensorSize,
        mario:GetShapeTransform (),
        mario:GetShapeSize ()
    )

    if not result then
        return
    end

    self:SpawnSound ()

    self:SpawnDebris ( "DebrisBegin001", "DebrisEnd001" )
    self:SpawnDebris ( "DebrisBegin002", "DebrisEnd002" )
    self:SpawnDebris ( "DebrisBegin003", "DebrisEnd003" )
    self:SpawnDebris ( "DebrisBegin004", "DebrisEnd004" )

    mario:Hit ( VELOCITY_MULTIPLIER )
    self:AddScore ()

    self._actor:Destroy ()
end

local function OnPrePhysicsSeek ( self, deltaTime )
    local mario = g_scene:FindActor ( "Mario" )

    if mario == nil then
        return
    end

    self._mario = mario:FindComponent ( "Script" )
    self.OnPrePhysics = OnPrePhysicsMonitor
end

-- Metamethods
local function Constructor ( self, handle, params )
    local obj = ScriptComponent ( handle )

    -- Methods
    obj.AddScore = AddScore
    obj.GetOrigin = GetOrigin
    obj.GetSensorSize = GetSensorSize
    obj.GetSensorTransform = GetSensorTransform
    obj.SpawnSound = SpawnSound
    obj.SpawnDebris = SpawnDebris

    -- Engine events
    obj.OnActorConstructed = OnActorConstructed
    obj.OnPrePhysics = OnPrePhysicsSeek

    return obj
end

setmetatable ( BrickExt, { __call = Constructor } )

-- Module function: fabric callable for BrickExt class
return BrickExt
