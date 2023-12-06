require "av://engine/gx_vec3.lua"
require "av://engine/script_component.lua"


local Main = {}

-- Constants
local GRAVITY = GXVec3 ()
GRAVITY:Init ( 0.0, -9.81, 0.0 )

-- Forward declarations
local OnPrePhysicsIdle

-- Methods
local function FindActor ( self, actorName, field )
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
local function OnPrePhysics ( self, deltaTime )
    local bobby = self:FindActor ( "Bobby", "_bobby" )
    local camera = self:FindActor ( "Camera", "_camera" )

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
    obj.FindActor = FindActor

    -- Engine events
    obj.OnPrePhysics = OnPrePhysics

    return obj
end

setmetatable ( Main, { __call = Constructor } )

-- Module function: fabric callable for Main class.
return Main
