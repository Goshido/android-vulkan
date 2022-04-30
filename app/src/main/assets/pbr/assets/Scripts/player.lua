require "av://engine/gx_math.lua"
require "av://engine/logger.lua"
require "av://engine/script_component.lua"


local Player = {}

-- Engine events
local function OnActorConstructed ( self, actor )
    LogD ( "Player:OnActorConstructed >>>" )

    for groupKey, group in pairs ( actor._components ) do
        for k, v in pairs ( group ) do
            LogD ( "    %s", v:GetName () )
        end
    end

    LogD ( "<<<" )
end

local function OnPostPhysics ( self, deltaTime )
    -- TODO
end

local function OnPrePhysics ( self, deltaTime )
    -- TODO
end

local function OnUpdate ( self, deltaTime )
    -- TODO
end

-- Metamethods
local function Constructor ( self, handle, params )
    local obj = ScriptComponent ( handle )

    LogD ( ">>> Player params:" )

    for k, v in pairs ( params ) do
        LogD ( "    " .. k .. ": " .. v )
    end

    LogD ( "<<<" )

    -- Data
    obj._health = 100
    obj._params = params
    obj._transform = GXMat4 ()
    obj._oneShot = true

    -- Engine events
    obj.OnActorConstructed = OnActorConstructed
    obj.OnPostPhysics = OnPostPhysics
    obj.OnPrePhysics = OnPrePhysics
    obj.OnUpdate = OnUpdate

    return obj
end

setmetatable ( Player, { __call = Constructor } )

-- Module function: fabric callable for Player class.
return Player
