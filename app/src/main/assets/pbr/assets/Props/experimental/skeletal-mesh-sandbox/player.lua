require "av://engine/animation_graph.lua"
require "av://engine/animation_player_node.lua"
require "av://engine/logger.lua"
require "av://engine/script_component.lua"
require "av://engine/skeletal_mesh_component.lua"


-- Class declaration
local Player = {}

-- Methods
local function InitAnimationGraph ( self )
    local animationGraph = AnimationGraph ( "pbr/assets/Props/experimental/exporter/leg.skeleton" )

    local bendAnimationPlayer = AnimationPlayerNode ()

    if not bendAnimationPlayer:LoadAnimation ( "pbr/assets/Props/experimental/exporter/bend.animation" ) then
        return
    end

    animationGraph:SetInput ( bendAnimationPlayer )
    animationGraph:Awake ()

    self._bendAnimationPlayer = bendAnimationPlayer
    self._animationGraph = animationGraph
end

local function InitSkeletalMesh ( self, actor )
    local t = GXMat4 ()
    actor:FindComponent ( "Origin" ):GetTransform ( t )

    local origin = GXVec3 ()
    t:GetW ( origin )

    local meshActor = Actor ( "PlayerMesh" )

    local meshComponent = SkeletalMeshComponent ( "Mesh",
        "pbr/assets/Props/experimental/exporter/leg.mesh2",
        "pbr/assets/Props/experimental/exporter/leg.skin",
        "pbr/assets/Props/experimental/exporter/leg.skeleton",
        "pbr/assets/Props/experimental/exporter/material.mtl"
        --"pbr/assets/System/DefaultCSG.mtl"
    )

    local m = GXMat4 ()
    m:Scale ( 0.5, 0.5, 0.5 )
    m:SetW ( origin )
    meshComponent:SetLocal ( m )

    meshActor:AppendComponent ( meshComponent )
    g_scene:AppendActor ( meshActor )

    self._meshActor = meshActor
end

-- Engine event handlers
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

    return obj
end

setmetatable ( Player, { __call = Constructor } )

-- Module function: fabric callable for Player class.
return Player
