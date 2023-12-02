require "av://engine/script_component.lua"


local Camera = {}

-- Methods
local function Activate ( self )
    local c = self._camera
    c:SetAspectRatio ( g_scene:GetRenderTargetAspectRatio () )
    g_scene:SetActiveCamera ( c )
end

-- Engine events
local function OnActorConstructed ( self, actor )
    self._camera = actor:FindComponent ( "Camera" )
end

local function OnInput ( self, inputEvent )
    -- TODO
end

local function OnRenderTargetChanged ( self )
    local c = self._camera

    if c then
        c:SetAspectRatio ( g_scene:GetRenderTargetAspectRatio () )
    end
end

local function OnUpdate ( self, deltaTime )
    -- TODO
end

-- Metamethods
local function Constructor ( self, handle, params )
    local obj = ScriptComponent ( handle )

    -- Methods
    obj.Activate = Activate

    -- Engine events
    obj.OnActorConstructed = OnActorConstructed
    obj.OnInput = OnInput
    obj.OnRenderTargetChanged = OnRenderTargetChanged
    obj.OnUpdate = OnUpdate

    return obj
end

setmetatable ( Camera, { __call = Constructor } )

-- Module function: fabric callable for Camera class.
return Camera
