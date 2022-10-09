require "av://engine/gx_vec4.lua"
require "av://engine/material.lua"
require "av://engine/script_component.lua"


-- Constants
local LIFE_TIME = 4.0
local FADE_TIME = 1.0


-- Class declaration
Debris = {}

-- Methods
local function Fade ( self, deltaTime )
    self._timer = self._timer - deltaTime

    if self._timer < 0.0 then
        self._actor:Destroy ()
        self._actor = nil
        return
    end

    local c = GXVec4 ()
    c:Init ( 1.0, 1.0, 1.0, self._timer )
    self._mesh:SetColor0 ( c )
end

local function Life ( self, deltaTime )
    self._timer = self._timer - deltaTime

    if self._timer > 0.0 then
        return
    end

    self._timer = FADE_TIME
    self._mesh:SetMaterial ( Material ( "pbr/assets/Props/experimental/world-1-1/brick/brick-stipple.mtl" ) )
    self.OnUpdate = Fade
end

-- Engine event handlers
local function OnActorConstructed ( self, actor )
    self._actor = actor
    self._mesh = actor:FindComponent ( "Mesh" )
end

-- Metamethods
local function Constructor ( self, handle, params )
    local obj = ScriptComponent ( handle )

    -- Engine events
    obj.OnActorConstructed = OnActorConstructed
    obj.OnUpdate = Life

    -- Data
    obj._timer = LIFE_TIME;

    return obj;
end

setmetatable ( Debris, { __call = Constructor } )

-- Module function: fabric callable for Debris class
return Debris
