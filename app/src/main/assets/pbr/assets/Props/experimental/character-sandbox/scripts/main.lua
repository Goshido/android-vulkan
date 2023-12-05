require "av://engine/script_component.lua"


local Main = {}

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
    bobby:Activate ( camera:GetRight () )
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
