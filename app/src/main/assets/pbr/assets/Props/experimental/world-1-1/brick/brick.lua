require "av://engine/script_component.lua"
require "av://engine/logger.lua"


-- Class declaration
local Brick = {}

-- Methods
local function GetOrigin ( self, actor, component )
    local t = GXMat4 ()
    actor:FindComponent ( component ):GetTransform ( t )
    local origin = GXVec3 ()
    t:GetW ( origin )
    return origin
end

local function GetSensorSize ( self, min, max )
    local result = GXVec3 ()
    result:Subtract ( max, min )
    result:MultiplyScalar ( result, g_scene:GetRendererToPhysicsScaleFactor () )
    return result
end

local function GetSensorTransform ( self, min, max )
    local diff = GXVec3 ()
    diff:Subtract ( max, min )

    local origin = GXVec3 ()
    origin:SumScaled ( min, 0.5, diff )
    origin:MultiplyScalar ( origin, g_scene:GetRendererToPhysicsScaleFactor () )

    local result = GXMat4 ()
    result:Identity ()
    result:SetW ( origin )
    return result
end

-- Engine event handlers
local function OnActorConstructed ( self, actor )
    local min = self:GetOrigin ( actor, "SensorMin" )
    local max = self:GetOrigin ( actor, "SensorMax" )
    self._sensorSize = self:GetSensorSize ( min, max )
    self._sensorTransform = self:GetSensorTransform ( min, max )
end

local function OnPrePhysicsMonitor ( self, deltaTime )
    local mario = self._mario

    local result = g_scene:OverlapTestBoxBox ( self._sensorTransform,
        self._sensorSize,
        mario:GetShapeTransform (),
        mario:GetShapeSize ()
    )

    if result then
        LogE ( "BOOM Shaka laka" )
    end
end

local function OnPrePhysicsSeek ( self, deltaTime )
    local mario = g_scene:FindActor ( "Mario" )

    if mario == nil then
        return
    end

    self._mario = mario:FindComponent ( "Script" )
    self.OnPrePhysics = OnPrePhysicsMonitor
end

-- Metamethods
local function Constructor ( self, handle, params )
    local obj = ScriptComponent ( handle )

    -- Methods
    obj.GetOrigin = GetOrigin
    obj.GetSensorSize = GetSensorSize
    obj.GetSensorTransform = GetSensorTransform

    -- Engine events
    obj.OnActorConstructed = OnActorConstructed
    obj.OnPrePhysics = OnPrePhysicsSeek

    return obj
end

setmetatable ( Brick, { __call = Constructor } )

-- Module function: fabric callable for Brick class
return Brick
