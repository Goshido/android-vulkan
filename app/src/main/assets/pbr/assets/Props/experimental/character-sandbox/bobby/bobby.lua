require "av://engine/actor.lua"
require "av://engine/animation_graph.lua"
require "av://engine/animation_player_node.lua"
require "av://engine/bit_field.lua"
require "av://engine/gx_mat4.lua"
require "av://engine/gx_quat.lua"
require "av://engine/gx_vec3.lua"
require "av://engine/input_event.lua"
require "av://engine/material.lua"
require "av://engine/script_component.lua"
require "av://engine/skeletal_mesh_component.lua"
require "av://engine/sound_emitter_spatial_component.lua"


local Bobby = {}

-- Constants
local COLLISION_HEIGHT = 1.8

local COLLISION_SIZE = GXVec3 ()
COLLISION_SIZE:Init ( 0.6, COLLISION_HEIGHT, 0.6 )

local COLLISION_OFFSET = GXVec3 ()
COLLISION_OFFSET:Init ( 0.0, COLLISION_HEIGHT * 0.5, 0.0 )

local COLLISION_GROUPS = BitField ()
COLLISION_GROUPS:SetAllBits ()

local SCALE = 32.0

local MOVE_DEAD_ZONE = 0.25
local MOVE_SPEED = 5.0

local ROTATE_SPEED = 5.0
local RUN_ANIMATION_SPEED = 4.0

local UP = GXVec3 ()
UP:Init ( 0.0, 1.0, 0.0 )

local FOOTSTEP_DISTANCE = 5.0
local FOOTSTEP_VOLUME = 0.25
local EVENT_RIGHT_FOOTSTEP = 10
local EVENT_LEFT_FOOTSTEP = 48

-- Forward declaration
local OnInput
local OnPostPhysics

-- Animation events
local function OnAnimationEvent ( context, frameIdx )
    context._isFootstep = true
end

-- Methods
local function Activate ( self, cameraRight, gravity )
    self._cameraRight = cameraRight
    self._gravity = gravity
    self._velocity:Init ( 0.0, 0.0, 0.0 )

    self._animationGraph:Awake ()

    self.OnInput = OnInput
    self.OnPostPhysics = OnPostPhysics
end

local function GetForward ( self )
    return self._forward
end

local function GetLocation ( self )
    return self._location
end

local function ProcessFootstep ( self )
    if not self._isFootstep then
        return
    end

    local s = self._footstep
    s:SetLocation ( self._location )
    s:Stop ()
    s:Play ()

    self._isFootstep = false
end

local function ResolveCollisions ( self )
    local location = self._location
    local v = GXVec3 ()
    v:Sum ( location, COLLISION_OFFSET )

    local t = GXMat4 ()
    t:Identity ()
    t:SetW ( v )

    local res = g_scene:GetPenetrationBox ( t, COLLISION_SIZE, COLLISION_GROUPS )
    local count = res._count

    if count == 0 then
        return
    end

    v:Init ( 0.0, 0.0, 0.0 )
    local penetrations = res._penetrations

    for i = 1, count do
        local pn = penetrations[ i ];
        v:SumScaled ( v, pn._depth, pn._normal )
    end

    location:Sum ( location, v )

    if v:DotProduct ( self._gravity ) < 0.0 then
        self._velocity:Init ( 0.0, 0.0, 0.0 )
    end
end

local function UpdateVisual ( self )
    local t = GXMat4 ()
    t:Identity ()

    -- Model must be rotated by 180 degrees around Y axis.

    local v = GXVec3 ()
    local forward = self._forward
    v:CrossProduct ( UP, forward )
    v:MultiplyScalar ( v, SCALE )
    v:Reverse ()
    t:SetX ( v )

    v:MultiplyScalar ( UP, SCALE )
    t:SetY ( v )

    v:MultiplyScalar ( forward, SCALE )
    v:Reverse ()
    t:SetZ ( v )

    v:MultiplyScalar ( self._location, g_scene:GetPhysicsToRendererScaleFactor () )
    t:SetW ( v )

    self._mesh:SetLocal ( t )
end

local function UpdateLocation ( self, deltaTime )
    local velocity = self._velocity
    velocity:SumScaled ( velocity, deltaTime, self._gravity )

    local location = self._location
    location:SumScaled ( location, deltaTime, velocity )

    if not self._isMoving then
        return
    end

    local cameraRight = self._cameraRight

    local moveDirection = GXVec3 ()
    moveDirection:CrossProduct ( cameraRight, UP )
    moveDirection:MultiplyScalar ( moveDirection, self._moveY )
    moveDirection:SumScaled ( moveDirection, self._moveX, cameraRight )

    local forward = self._forward
    local length = moveDirection:Length ()
    location:SumScaled ( location, length * deltaTime * MOVE_SPEED, forward )

    local v = GXVec3 ()
    v:CrossProduct ( moveDirection, forward )
    local isCCW = v:GetY () > 0.0

    local r = GXQuat ()
    r:FromAxisAngle ( UP, deltaTime * ( isCCW and -ROTATE_SPEED or ROTATE_SPEED ) )

    local f = GXVec3 ()
    r:TransformFast ( f, forward )

    v:CrossProduct ( moveDirection, f )

    if v:GetY () > 0.0 == isCCW then
        -- No overshooting case.
        forward:Clone ( f )
        return
    end

    -- Overshooting case. It was too much rotation.
    forward:MultiplyScalar ( moveDirection, 1.0 / length )
end

-- Engine events
local function OnActorConstructed ( self, actor )
    local visual = Actor ( "Bobby Visual" )
    self._visual = visual

    local skeletonFile = "pbr/assets/Props/experimental/character-sandbox/bobby/bobby.skeleton"

    local mesh = SkeletalMeshComponent ( "Mesh",
        "pbr/assets/Props/experimental/character-sandbox/bobby/bobby.mesh2",
        "pbr/assets/Props/experimental/character-sandbox/bobby/bobby.skin",
        skeletonFile
    )

    self._mesh = mesh
    mesh:SetMaterial ( Material ( "pbr/assets/System/Default.mtl" ) )

    local t = GXMat4 ()
    actor:FindComponent ( "Origin" ):GetTransform ( t )

    local location = self._location
    t:GetW ( location )
    location:MultiplyScalar ( location, g_scene:GetRendererToPhysicsScaleFactor () )

    t:GetZ ( self._forward )

    local animationGraph = AnimationGraph ( skeletonFile )
    self._animationGraph = animationGraph

    local runAnimationPlayer = AnimationPlayerNode ()
    self._runAnimationPlayer = runAnimationPlayer
    runAnimationPlayer:LoadAnimation ( "pbr/assets/Props/experimental/character-sandbox/bobby/run.animation" )
    runAnimationPlayer:SetPlaybackSpeed ( RUN_ANIMATION_SPEED )
    runAnimationPlayer:SetEvent ( self, EVENT_RIGHT_FOOTSTEP, OnAnimationEvent )
    runAnimationPlayer:SetEvent ( self, EVENT_LEFT_FOOTSTEP, OnAnimationEvent )

    animationGraph:SetInput ( runAnimationPlayer )
    mesh:SetAnimationGraph ( animationGraph )

    local footstep = SoundEmitterSpatialComponent ( "Footstep", eSoundChannel.SFX )
    visual:AppendComponent ( footstep )
    footstep:SetSoundAsset ( "pbr/assets/Props/experimental/character-sandbox/bobby/step.ogg", false )
    footstep:SetDistance ( FOOTSTEP_DISTANCE )
    footstep:SetLocation ( location )
    footstep:SetVolume ( FOOTSTEP_VOLUME )
    self._footstep = footstep

    visual:AppendComponent ( mesh )
    g_scene:AppendActor ( visual )
end

OnInput = function ( self, inputEvent )
    if inputEvent._type ~= eEventType.LeftStick then
        return
    end

    local x = inputEvent._x
    local y = inputEvent._y

    if math.abs ( x ) + math.abs ( y ) < MOVE_DEAD_ZONE then
        self._isMoving = false
        return
    end

    self._moveX = x
    self._moveY = y
    self._isMoving = true
end

local function OnInputIdle ( self, inputEvent )
    -- NOTHING
end

OnPostPhysics = function ( self, deltaTime )
    self:UpdateLocation ( deltaTime )
    self:ResolveCollisions ()
    self:UpdateVisual ()
    self:ProcessFootstep ()
end

local function OnPostPhysicsIdle ( self, deltaTime )
    -- NOTHING
end

-- Metamethods
local function Constructor ( self, handle, params )
    local obj = ScriptComponent ( handle )

    -- Data
    obj._forward = GXVec3 ()
    obj._location = GXVec3 ()
    obj._velocity = GXVec3 ()

    obj._isMoving = false
    obj._moveX = 0.0
    obj._moveY = 0.0

    obj._isFootstep = false

    -- Methods
    obj.Activate = Activate
    obj.GetForward = GetForward
    obj.GetLocation = GetLocation
    obj.ProcessFootstep = ProcessFootstep
    obj.ResolveCollisions = ResolveCollisions
    obj.UpdateLocation = UpdateLocation
    obj.UpdateVisual = UpdateVisual

    -- Engine events
    obj.OnActorConstructed = OnActorConstructed
    obj.OnInput = OnInputIdle
    obj.OnPostPhysics = OnPostPhysicsIdle

    return obj
end

setmetatable ( Bobby, { __call = Constructor } )

-- Module function: fabric callable for Bobby class.
return Bobby
