require "av://engine/actor.lua"
require "av://engine/animation_graph.lua"
require "av://engine/animation_player_node.lua"
require "av://engine/gx_mat4.lua"
require "av://engine/gx_vec3.lua"
require "av://engine/material.lua"
require "av://engine/script_component.lua"
require "av://engine/skeletal_mesh_component.lua"


local Bobby = {}

-- Constants
local SCALE = 32.0

local UP = GXVec3 ()
UP:Init ( 0.0, 1.0, 0.0 )

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
    self._position = position

    local forward = GXVec3 ()
    t:GetZ ( forward )
    self._forward = forward

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

local function OnInput ( self, inputEvent )
    -- Some implementation
end

local function OnPostPhysics ( self, deltaTime )
    -- Some implementation
end

local function OnUpdate ( self, deltaTime )
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

    v:MultiplyScalar ( self._position, g_scene:GetPhysicsToRendererScaleFactor () )
    t:SetW ( v )

    self._mesh:SetLocal ( t )
end

-- Metamethods
local function Constructor ( self, handle, params )
    local obj = ScriptComponent ( handle )

    -- Engine events
    obj.OnActorConstructed = OnActorConstructed
    obj.OnInput = OnInput
    obj.OnPostPhysics = OnPostPhysics
    obj.OnUpdate = OnUpdate

    return obj
end

setmetatable ( Bobby, { __call = Constructor } )

-- Module function: fabric callable for Bobby class.
return Bobby

