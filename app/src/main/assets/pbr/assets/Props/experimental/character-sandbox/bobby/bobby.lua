require "av://engine/actor.lua"
require "av://engine/animation_graph.lua"
require "av://engine/animation_player_node.lua"
require "av://engine/gx_mat4.lua"
require "av://engine/gx_quat.lua"
require "av://engine/gx_vec3.lua"
require "av://engine/input_event.lua"
require "av://engine/material.lua"
require "av://engine/script_component.lua"
require "av://engine/skeletal_mesh_component.lua"


local Bobby = {}

-- Constants
local SCALE = 32.0

local UP = GXVec3 ()
UP:Init ( 0.0, 1.0, 0.0 )

local MOVE_DEAD_ZONE = 0.2
local MOVE_SPEED = 5.0

local ROTATE_SPEED = 5.0
local ROTATE_THRESHOLD = 0.1

-- Forward declaration
local OnInput
local OnUpdate

-- Methods
local function Activate ( self, cameraRight )
    self._cameraRight = cameraRight
    self.OnInput = OnInput
    self.OnUpdate = OnUpdate
end

local function GetForward ( self )
    return self._forward
end

local function GetLocation ( self )
    return self._location
end

local function UpdateVisual ( self )
    local t = GXMat4 ()
    t:Identity ()

    local v = GXVec3 ()
    local forward = self._forward
    v:CrossProduct ( UP, forward )
    v:MultiplyScalar ( v, SCALE )
    t:SetX ( v )

    v:MultiplyScalar ( UP, SCALE )
    t:SetY ( v )

    v:MultiplyScalar ( forward, SCALE )
    t:SetZ ( v )

    v:MultiplyScalar ( self._location, g_scene:GetPhysicsToRendererScaleFactor () )
    t:SetW ( v )

    self._mesh:SetLocal ( t )
end

local function UpdateLocation ( self, deltaTime )
    local cameraRight = self._cameraRight

    local moveDirection = GXVec3 ()
    moveDirection:CrossProduct ( cameraRight, UP )
    moveDirection:MultiplyScalar ( moveDirection, self._moveY )
    moveDirection:SumScaled ( moveDirection, self._moveX, cameraRight )

    local location = self._location
    local forward = self._forward
    location:SumScaled ( location, moveDirection:Length () * deltaTime * MOVE_SPEED, forward )

    local v = GXVec3 ()
    v:CrossProduct ( moveDirection, forward )
    local a = v:GetY ()

    if math.abs ( a ) < ROTATE_THRESHOLD then
        return
    end

    local angle = a > 0.0 and -ROTATE_SPEED or ROTATE_SPEED
    local r = GXQuat ()
    r:FromAxisAngle ( UP, angle * deltaTime )

    local f = GXVec3 ()
    r:TransformFast ( f, forward )
    forward:Clone ( f )
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

    local position = GXVec3 ()
    t:GetW ( position )
    position:MultiplyScalar ( position, g_scene:GetRendererToPhysicsScaleFactor () )
    self._location = position

    local forward = GXVec3 ()
    t:GetZ ( forward )
    self._forward = forward
    self._isMoving = false
    self._moveX = 0.0
    self._moveY = 0.0

    local animationGraph = AnimationGraph ( skeletonFile )
    self._animationGraph = animationGraph

    local runAnimationPlayer = AnimationPlayerNode ()
    self._runAnimationPlayer = runAnimationPlayer
    runAnimationPlayer:LoadAnimation ( "pbr/assets/Props/experimental/character-sandbox/bobby/run.animation" )

    animationGraph:SetInput ( runAnimationPlayer )
    animationGraph:Awake ()

    mesh:SetAnimationGraph ( animationGraph )
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

OnUpdate = function ( self, deltaTime )
    self:UpdateVisual ()

    if self._isMoving then
        self:UpdateLocation ( deltaTime )
    end
end

local function OnUpdateIdle ( self, deltaTime )
    -- NOTHING
end

-- Metamethods
local function Constructor ( self, handle, params )
    local obj = ScriptComponent ( handle )

    -- Methods
    obj.Activate = Activate
    obj.GetForward = GetForward
    obj.GetLocation = GetLocation
    obj.UpdateLocation = UpdateLocation
    obj.UpdateVisual = UpdateVisual

    -- Engine events
    obj.OnActorConstructed = OnActorConstructed
    obj.OnInput = OnInputIdle
    obj.OnUpdate = OnUpdateIdle

    return obj
end

setmetatable ( Bobby, { __call = Constructor } )

-- Module function: fabric callable for Bobby class.
return Bobby
