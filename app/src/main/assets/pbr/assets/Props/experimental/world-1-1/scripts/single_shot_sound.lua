require "av://engine/script_component.lua"
require "av://engine/sound_emitter_spatial_component.lua"


-- Class declaration
SingleShotSound = {}

-- Methods
local function Life ( self, deltaTime )
    if self._actor == nil then
        return
    end

    self._duration = self._duration - deltaTime

    if self._duration > 0.0 then
        return
    end

    self._actor:Destroy ()
    self._actor = nil
end

-- Engine event handlers
local function OnActorConstructed ( self, actor )
    local c = SoundEmitterSpatialComponent ( "Sound", self._soundChannel )
    actor:AppendComponent ( c )

    c:SetSoundAsset ( self._soundAsset, false )
    c:SetDistance ( self._distance )
    c:SetLocation ( self._location )
    c:SetVolume ( self._volume )
    c:Play ()

    self._distance = nil
    self._location = nil
    self._soundAsset = nil
    self._soundChannel = nil
    self._volume = nil

    self._actor = actor
end

-- Metamethods
local function Constructor ( self, handle, params )
    local obj = ScriptComponent ( handle )

    -- Engine events
    obj.OnActorConstructed = OnActorConstructed
    obj.OnUpdate = Life

    -- Data
    obj._duration = params._duration
    obj._distance = params._distance
    obj._location = params._location
    obj._soundAsset = params._soundAsset
    obj._soundChannel = params._soundChannel
    obj._volume = params._volume

    return obj;
end

setmetatable ( SingleShotSound, { __call = Constructor } )

-- Module function: fabric callable for SingleShotSound class
return SingleShotSound
