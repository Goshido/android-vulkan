require "av://engine/component.lua"
require "av://engine/sound_channel.lua"


SoundEmitterSpatialComponent = {}

-- Methods
local function GetVolume ( self )
    assert ( type ( self ) == "table" and self._type == eObjectType.SoundEmitterSpatialComponent,
        [[SoundEmitterSpatialComponent:GetVolume - Calling not via ":" syntax.]]
    )

    return av_SoundEmitterSpatialComponentGetVolume ( self._handle )
end

local function IsPlaying ( self )
    assert ( type ( self ) == "table" and self._type == eObjectType.SoundEmitterSpatialComponent,
        [[SoundEmitterSpatialComponent:IsPlaying - Calling not via ":" syntax.]]
    )

    return av_SoundEmitterSpatialComponentIsPlaying ( self._handle )
end

local function Pause ( self )
    assert ( type ( self ) == "table" and self._type == eObjectType.SoundEmitterSpatialComponent,
        [[SoundEmitterSpatialComponent:Pause - Calling not via ":" syntax.]]
    )

    return av_SoundEmitterSpatialComponentPause ( self._handle )
end

local function Play ( self )
    assert ( type ( self ) == "table" and self._type == eObjectType.SoundEmitterSpatialComponent,
        [[SoundEmitterSpatialComponent:Play - Calling not via ":" syntax.]]
    )

    return av_SoundEmitterSpatialComponentPlay ( self._handle )
end

local function SetDistance ( self, distance )
    assert ( type ( self ) == "table" and self._type == eObjectType.SoundEmitterSpatialComponent,
        [[SoundEmitterSpatialComponent:SetDistance - Calling not via ":" syntax.]]
    )

    assert ( type ( distance ) == "number", [[SoundEmitterSpatialComponent:SetDistance - "distance" is not number.]] )
    av_SoundEmitterSpatialComponentSetDistance ( self._handle, distance )
end

local function SetLocation ( self, location )
    assert ( type ( self ) == "table" and self._type == eObjectType.SoundEmitterSpatialComponent,
        [[SoundEmitterSpatialComponent:SetLocation - Calling not via ":" syntax.]]
    )

    assert ( type ( location ) == "table" and location._type == eObjectType.GXVec3,
        [[SoundEmitterSpatialComponent:SetLocation - "location" is not a GXVec3.]]
    )

    av_SoundEmitterSpatialComponentSetLocation ( self._handle, location._handle )
end

local function SetSoundAsset ( self, asset, looped )
    assert ( type ( self ) == "table" and self._type == eObjectType.SoundEmitterSpatialComponent,
        [[SoundEmitterSpatialComponent:SetSoundAsset - Calling not via ":" syntax.]]
    )

    assert ( type ( asset ) == "string", [[SoundEmitterSpatialComponent:SetSoundAsset - "asset" is not a string.]] )
    assert ( type ( looped ) == "boolean", [[SoundEmitterSpatialComponent:SetSoundAsset - "looped" is not a boolean.]] )

    return av_SoundEmitterSpatialComponentSetSoundAsset ( self._handle, asset, looped )
end

local function SetVolume ( self, volume )
    assert ( type ( self ) == "table" and self._type == eObjectType.SoundEmitterSpatialComponent,
        [[SoundEmitterSpatialComponent:SetVolume - Calling not via ":" syntax.]]
    )

    assert ( type ( volume ) == "number", [[SoundEmitterSpatialComponent:SetVolume - "volume" is not number.]] )
    return av_SoundEmitterSpatialComponentSetVolume ( self._handle, volume )
end

local function Stop ( self )
    assert ( type ( self ) == "table" and self._type == eObjectType.SoundEmitterSpatialComponent,
        [[SoundEmitterSpatialComponent:Stop - Calling not via ":" syntax.]]
    )

    return av_SoundEmitterSpatialComponentStop ( self._handle )
end

-- Engine event handlers
local function OnDestroy ( self )
    av_SoundEmitterSpatialComponentDestroy ( self._handle )
end

-- Metamethods
local mt = {
    __gc = function ( self )
        av_SoundEmitterSpatialComponentCollectGarbage ( self._handle )
    end
}

-- This function is exported to C++ side.
function RegisterSoundEmitterSpatialComponent ( handle )
    local obj = Component ( eObjectType.SoundEmitterSpatialComponent, handle )

    -- Methods
    obj.GetVolume = GetVolume
    obj.IsPlaying = IsPlaying
    obj.Pause = Pause
    obj.Play = Play
    obj.SetDistance = SetDistance
    obj.SetLocation = SetLocation
    obj.SetSoundAsset = SetSoundAsset
    obj.SetVolume = SetVolume
    obj.Stop = Stop

    -- Engine events
    obj.OnDestroy = OnDestroy

    return setmetatable ( obj, mt )
end

local function Constructor ( self, name, soundChannel )
    assert ( type ( name ) == "string", [[SoundEmitterSpatialComponent - "name" is not a string."]] )
    assert ( type ( soundChannel ) == "number", [[SoundEmitterSpatialComponent - "soundChannel" is not number."]] )

    assert ( soundChannel >= 0 and soundChannel < eSoundChannel.TOTAL,
        "SoundEmitterSpatialComponent - Incorrect sound channel. Component name: " .. name
    )

    return RegisterSoundEmitterSpatialComponent ( av_SoundEmitterSpatialComponentCreate ( name, soundChannel ) )
end

setmetatable ( SoundEmitterSpatialComponent, { __call = Constructor } )

-- Module contract
return nil
