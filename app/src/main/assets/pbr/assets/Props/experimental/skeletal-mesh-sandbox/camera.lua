require "av://engine/scene.lua"
require "av://engine/script_component.lua"


local Camera = {}

-- Engine event handlers
local function OnActorConstructed ( self, actor )
    g_scene:SetActiveCamera ( actor:FindComponent ( "Camera" ) )
end

-- Metamethods
local function Constructor ( self, handle, params )
    local obj = ScriptComponent ( handle )

    -- Engine events
    obj.OnActorConstructed = OnActorConstructed

    return obj
end

setmetatable ( Camera, { __call = Constructor } )

-- Module function: fabric callable for Camera class.
return Camera
