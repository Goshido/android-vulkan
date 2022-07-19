require "av://engine/script_component.lua"


-- Constants
local SPEED = 44.44448

-- Class declaration
local Goomba = {}

-- Methods
local function GetCenter ( self, min, max )
    local diff = GXVec3 ()
    diff:Subtract ( max, min )

    local result = GXVec3 ()
    result:SumScaled ( min, 0.5, diff )
    return result
end

local function GetOrigin ( self, origin, actor, component )
    local t = GXMat4 ()
    actor:FindComponent ( component ):GetTransform ( t )
    t:GetW ( origin )
end

local function GetSensorOffset ( self, parentOrigin, actor, min, max )
    local result = GXVec3 ()
    result:Subtract ( self:GetCenter ( actor, min, max ), parentOrigin )
    result:MultiplyScalar ( result, g_scene:GetRendererToPhysicsScaleFactor () )
    return result
end

local function GetSensorParentTransform ( self, sensor, toParentTransform )
    local m = GXMat4 ()
    sensor:GetLocal ( m )

    local result = GXMat4 ()
    result:Multiply ( m, toParentTransform )
    return result
end

local function GetSensorSize ( self, min, max )
    local result = GXVec3 ()
    result:Subtract ( max, min )
    result:MultiplyScalar ( result, g_scene:GetRendererToPhysicsScaleFactor () )
    return result
end

local function MoveTo ( self, origin )
    local m = self._localMatrix
    m:SetW ( origin )
    self._mesh:SetLocal ( m )

    local attachment = GXMat4 ()
    attachment:Multiply ( self._sensorTopParent, m )
    self._sensorTop:SetLocal ( attachment )

    attachment:Multiply ( self._sensorLeftParent, m )
    self._sensorLeft:SetLocal ( attachment )

    attachment:Multiply ( self._sensorRightParent, m )
    self._sensorRight:SetLocal ( attachment )
end

-- Engine event handlers
local function OnActorConstructed ( self, actor )
    local min = GXVec3 ()
    local max = GXVec3 ()

    self:GetOrigin ( min, actor, "BodyMin" )
    self:GetOrigin ( max, actor, "BodyMax" )
    local origin = self:GetCenter ( min, max )

    self._localMatrix = GXMat4 ()
    self._localMatrix:Identity ()
    self._localMatrix:SetW ( origin )

    self._mesh = actor:FindComponent ( "Mesh" )
    local p = GXMat4 ()
    self._mesh:GetLocal ( p )
    local toParentTransform = GXMat4 ()
    toParentTransform:Inverse ( p )

    self:GetOrigin ( min, actor, "SensorTopMin" )
    self:GetOrigin ( max, actor, "SensorTopMax" )
    self._sensorTop = actor:FindComponent ( "SensorTop" )
    self._sensorTopSize = self:GetSensorSize ( min, max )
    self._sensorTopOffset = self:GetSensorOffset ( origin, min, max )
    self._sensorTopParent = self:GetSensorParentTransform ( self._sensorTop, toParentTransform )

    self:GetOrigin ( min, actor, "SensorLeftMin" )
    self:GetOrigin ( max, actor, "SensorLeftMax" )
    self._sensorLeft = actor:FindComponent ( "SensorLeft" )
    self._sensorLeftSize = self:GetSensorSize ( min, max )
    self._sensorLeftOffset = self:GetSensorOffset ( origin, min, max )
    self._sensorLeftParent = self:GetSensorParentTransform ( self._sensorLeft, toParentTransform )

    self:GetOrigin ( min, actor, "SensorRightMin" )
    self:GetOrigin ( max, actor, "SensorRightMax" )
    self._sensorRight = actor:FindComponent ( "SensorRight" )
    self._sensorRightSize = self:GetSensorSize ( min, max )
    self._sensorRightOffset = self:GetSensorOffset ( origin, min, max )
    self._sensorRightParent = self:GetSensorParentTransform ( self._sensorRight, toParentTransform )
end

local function OnPostPhysics ( self, deltaTime )
    local localMatrix = self._localMatrix

    local m = GXMat4 ()
    m:Clone ( localMatrix )

    local v = GXVec3 ()
    localMatrix:GetW ( v )

    v:MultiplyScalar ( v, g_scene:GetRendererToPhysicsScaleFactor () )
    m:SetW ( v )

    -- TODO
end

-- Metamethods
local function Constructor ( self, handle, params )
    local obj = ScriptComponent ( handle )

    -- Methods
    obj.GetCenter = GetCenter
    obj.GetOrigin = GetOrigin
    obj.GetSensorOffset = GetSensorOffset
    obj.GetSensorParentTransform = GetSensorParentTransform
    obj.GetSensorSize = GetSensorSize
    obj.MoveTo = MoveTo
    obj.UpdateRoute = UpdateRoute

    -- Engine events
    obj.OnActorConstructed = OnActorConstructed
    obj.OnPostPhysics = OnPostPhysics
    return obj
end

setmetatable ( Goomba, { __call = Constructor } )

-- Module function: fabric callable for Goomba class
return Goomba
