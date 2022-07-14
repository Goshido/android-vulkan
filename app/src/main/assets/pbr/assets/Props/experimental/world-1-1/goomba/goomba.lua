require "av://engine/script_component.lua"


-- Class declaration
local Goomba = {}

-- Methods
local function GetOrigin ( self, origin, actor, component )
    local t = GXMat4 ()
    actor:FindComponent ( component ):GetTransform ( t )
    t:GetW ( origin )
end

local function GetCenter ( self, center, actor, min, max )
    local minOrigin = GXVec3 ()
    self:GetOrigin ( minOrigin, actor, min )

    local maxOrigin = GXVec3 ()
    self:GetOrigin ( maxOrigin, actor, max )

    local diff = GXVec3 ()
    diff:Subtract ( maxOrigin, minOrigin )

    center:SumScaled ( minOrigin, 0.5, diff )
end

local function GetOffset ( self, offset, parentOrigin, actor, min, max )
    local origin = GXVec3 ()
    self:GetCenter ( origin, actor, min, max )
    offset:Subtract ( origin, parentOrigin )
end

local function MoveTo ( self, origin )
    -- TODO
end

-- Engine event handlers
local function OnActorConstructed ( self, actor )
    local origin = GXVec3 ()
    self:GetCenter ( origin, actor, "BodyMin", "BodyMax" )

    self._localMatrix:Identity ()
    self._localMatrix:SetW ( origin )

    self:GetOffset ( self._sensorLeftOffset, origin, actor, "SensorLeftMin", "SensorLeftMax" )
    self:GetOffset ( self._sensorRightOffset, origin, actor, "SensorRightMin", "SensorRightMax" )
    self:GetOffset ( self._sensorTopOffset, origin, actor, "SensorTopMin", "SensorTopMax" )
end

-- Metamethods
local function Constructor ( self, handle, params )
    local obj = ScriptComponent ( handle )

    -- Data
    obj._localMatrix = GXMat4 ()
    obj._sensorLeftOffset = GXVec3 ()
    obj._sensorRightOffset = GXVec3 ()
    obj._sensorTopOffset = GXVec3 ()

    -- Methods
    obj.GetCenter = GetCenter
    obj.GetOffset = GetOffset
    obj.GetOrigin = GetOrigin
    obj.MoveTo = MoveTo

    -- Engine events
    obj.OnActorConstructed = OnActorConstructed
    return obj
end

setmetatable ( Goomba, { __call = Constructor } )

-- Module function: fabric callable for Goomba class
return Goomba
