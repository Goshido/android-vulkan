require "av://engine/animation_graph.lua"
require "av://engine/animation_player_node.lua"
require "av://engine/material.lua"
require "av://engine/script_component.lua"
require "av://engine/skeletal_mesh_component.lua"


-- Class declaration
local Player = {}

-- Methods
local function InitAnimationGraph ( self )
    local animationGraph = AnimationGraph ( "pbr/assets/Props/experimental/skeletal-mesh-sandbox/human/human.skeleton" )
    local walkAnimationPlayer = AnimationPlayerNode ()

    local success = walkAnimationPlayer:LoadAnimation (
        "pbr/assets/Props/experimental/skeletal-mesh-sandbox/human/walk.animation"
    )

    if not success then
        return
    end

    walkAnimationPlayer:SetPlaybackSpeed ( 0.777 )

    animationGraph:SetInput ( walkAnimationPlayer )
    animationGraph:Awake ()

    self._meshComponent:SetAnimationGraph ( animationGraph )
    self._walkAnimationPlayer = walkAnimationPlayer
    self._animationGraph = animationGraph
end

local function InitSkeletalMesh ( self, actor )
    local t = GXMat4 ()
    actor:FindComponent ( "Origin" ):GetTransform ( t )

    local origin = GXVec3 ()
    t:GetW ( origin )

    local meshActor = Actor ( "PlayerMesh" )

    local meshComponent = SkeletalMeshComponent ( "Mesh",
        "pbr/assets/Props/experimental/skeletal-mesh-sandbox/human/human.mesh2",
        "pbr/assets/Props/experimental/skeletal-mesh-sandbox/human/human.skin",
        "pbr/assets/Props/experimental/skeletal-mesh-sandbox/human/human.skeleton"
    )

    local m = GXMat4 ()
    m:Scale ( 0.5, 0.5, 0.5 )

    m:SetW ( origin )
    meshComponent:SetLocal ( m )
    meshComponent:SetMaterial ( Material ( "pbr/assets/Props/experimental/skeletal-mesh-sandbox/human/human.mtl" ) )

    meshActor:AppendComponent ( meshComponent )
    g_scene:AppendActor ( meshActor )

    self._meshComponent = meshComponent
    self._meshActor = meshActor
end

-- Engine event handlers
local function OnInput ( self, inputEvent )
    if inputEvent._type == eEventType.KeyUp and inputEvent._key == eKey.Home then
        g_scene:Quit ()
    end
end

local function OnActorConstructed ( self, actor )
    self:InitSkeletalMesh ( actor )
    self:InitAnimationGraph ()
end

-- Metamethods
local function Constructor ( self, handle, params )
    local obj = ScriptComponent ( handle )

    -- Methods
    obj.InitAnimationGraph = InitAnimationGraph
    obj.InitSkeletalMesh = InitSkeletalMesh

    -- Engine events
    obj.OnActorConstructed = OnActorConstructed
    obj.OnInput = OnInput

    return obj
end

setmetatable ( Player, { __call = Constructor } )

-- Module function: fabric callable for Player class.
return Player
