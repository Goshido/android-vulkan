require "av://engine/scene.lua"
require "av://engine/script_component.lua"


local Camera = {}

-- Engine event handlers
local function OnActorConstructed ( self, actor )
    local camera = actor:FindComponent ( "Camera" )
    g_scene:SetActiveCamera ( camera )
    self._camera = camera
end

local function OnRenderTargetChanged ( self )
    self._camera:SetAspectRatio ( g_scene:GetRenderTargetAspectRatio () )
end

-- Metamethods
local function Constructor ( self, handle, params )
    local obj = ScriptComponent ( handle )

    -- Engine events
    obj.OnActorConstructed = OnActorConstructed
    obj.OnRenderTargetChanged = OnRenderTargetChanged

    return obj
end

setmetatable ( Camera, { __call = Constructor } )

-- Module function: fabric callable for Camera class.
return Camera
