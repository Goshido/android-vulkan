require "av://engine/script_component.lua"


-- Class declaration
local Mario = {}

-- Methods
local function Jump ( self )
    -- TODO
end

local function Move ( self, inputEvent )
    -- TODO
end

-- Engine event handlers
local function OnInputActive ( self, inputEvent )
    if inputEvent._type == eEventType.LeftStick then
        self:Move ( inputEvent )
        return
    end

    if inputEvent._type == eEventType.KeyDown and inputEvent._key == eKey.A then
        self:Jump ()
    end
end

local function OnInputIdle ( self, inputEvent )
    -- NOTHING
end

local function OnActorConstructed ( self, actor )
    self._rigidBody = actor:FindComponent ( "RigidBody" )
    self.OnInput = OnInputActive
end

-- Metamethods
local function Constructor ( self, handle, params )
    local obj = ScriptComponent ( handle )

    -- Methods
    obj.Jump = Jump
    obj.Move = Move

    -- Engine events
    obj.OnActorConstructed = OnActorConstructed
    obj.OnInput = OnInputIdle

    return obj
end

setmetatable ( Mario, { __call = Constructor } )

-- Module function: fabric callable for Mario class.
return Mario
