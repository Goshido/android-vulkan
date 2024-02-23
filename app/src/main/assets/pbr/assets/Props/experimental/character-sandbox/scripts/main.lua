require "av://engine/script_component.lua"


local Main = {}

-- Constants
local GRAVITY = GXVec3 ()
GRAVITY:Init ( 0.0, -9.81, 0.0 )

-- Forward declarations
local OnPrePhysicsIdle

-- Methods
local function FindScript ( self, actorName, field )
    local a = self[ field ]

    if a then
        return a
    end

    a = g_scene:FindActor ( actorName )

    if not a then
        return false
    end

    local script = a:FindComponent ( "Script" )
    self[ field ] = script
    return script
end

-- Engine events
local function OnInput ( self, inputEvent )
    if inputEvent._type == eEventType.KeyUp and inputEvent._key == eKey.Home then
        g_scene:Quit ()
    end
end

local function OnPrePhysics ( self, deltaTime )
    local bobby = self:FindScript ( "Bobby", "_bobby" )
    local camera = self:FindScript ( "Camera", "_camera" )

    if not bobby or not camera then
        return
    end

    camera:Activate ( bobby:GetLocation (), bobby:GetForward () )
    bobby:Activate ( camera:GetRight (), GRAVITY )
    self.OnPrePhysics = OnPrePhysicsIdle
end

OnPrePhysicsIdle = function ( self, deltaTime )
    -- NOTHING
end

-- Metamethods
local function Constructor ( self, handle, params )
    local obj = ScriptComponent ( handle )

    -- Methods
    obj.FindScript = FindScript

    -- Engine events
    obj.OnInput = OnInput
    obj.OnPrePhysics = OnPrePhysics

    return obj
end

setmetatable ( Main, { __call = Constructor } )

-- Module function: fabric callable for Main class.
return Main
