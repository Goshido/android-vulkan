require "av://engine/animation_graph.lua"
require "av://engine/logger.lua"
require "av://engine/script_component.lua"
require "av://engine/skeletal_mesh_component.lua"


-- Class declaration
local Player = {}

-- Engine event handlers
local function OnActorConstructed ( self, actor )
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
    )

    local m = GXMat4 ()
    m:Scale ( 0.5, 0.5, 0.5 )
    m:SetW ( origin )
    meshComponent:SetLocal ( m )

    meshActor:AppendComponent ( meshComponent )
    g_scene:AppendActor ( meshActor )

    self._meshActor = meshActor
    self._animationGraph = AnimationGraph ( "pbr/assets/Props/experimental/exporter/leg.skeleton" )
end

-- Metamethods
local function Constructor ( self, handle, params )
    local obj = ScriptComponent ( handle )

    -- Engine events
    obj.OnActorConstructed = OnActorConstructed

    return obj
end

setmetatable ( Player, { __call = Constructor } )

-- Module function: fabric callable for Player class.
return Player
