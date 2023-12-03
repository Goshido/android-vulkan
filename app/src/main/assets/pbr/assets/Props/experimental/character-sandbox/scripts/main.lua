require "av://engine/script_component.lua"


local Main = {}


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

-- Forward declarations
local OnPrePhysicsIdle
local OnPrePhysicsActive

-- Engine events
local function OnActorConstructed ( self, actor )
    self.OnPrePhysics = OnPrePhysicsActive
end

OnPrePhysicsActive = function ( self, deltaTime )
    local bobby = self:FindActor ( "Bobby", "_bobby" )
    local camera = self:FindActor ( "Camera", "_camera" )

    if not bobby or not camera then
        return
    end

    camera:Activate ( bobby:GetPosition (), bobby:GetForward () )
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
    obj.OnActorConstructed = OnActorConstructed
    obj.OnPrePhysics = OnPrePhysicsIdle

    return obj
end

setmetatable ( Main, { __call = Constructor } )

-- Module function: fabric callable for Main class.
return Main
