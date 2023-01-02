require "av://engine/component.lua"
require "av://engine/sound_channel.lua"


SoundEmitterGlobalComponent = {}

-- Methods
local function GetVolume ( self )
    assert ( type ( self ) == "table" and self._type == eObjectType.SoundEmitterGlobalComponent,
        [[SoundEmitterGlobalComponent:GetVolume - Calling not via ":" syntax.]]
    )

    return av_SoundEmitterGlobalComponentGetVolume ( self._handle )
end

local function IsPlaying ( self )
    assert ( type ( self ) == "table" and self._type == eObjectType.SoundEmitterGlobalComponent,
        [[SoundEmitterGlobalComponent:IsPlaying - Calling not via ":" syntax.]]
    )

    return av_SoundEmitterGlobalComponentIsPlaying ( self._handle )
end

local function Pause ( self )
    assert ( type ( self ) == "table" and self._type == eObjectType.SoundEmitterGlobalComponent,
        [[SoundEmitterGlobalComponent:Pause - Calling not via ":" syntax.]]
    )

    return av_SoundEmitterGlobalComponentPause ( self._handle )
end

local function Play ( self )
    assert ( type ( self ) == "table" and self._type == eObjectType.SoundEmitterGlobalComponent,
        [[SoundEmitterGlobalComponent:Play - Calling not via ":" syntax.]]
    )

    return av_SoundEmitterGlobalComponentPlay ( self._handle )
end

local function SetSoundAsset ( self, asset, looped )
    assert ( type ( self ) == "table" and self._type == eObjectType.SoundEmitterGlobalComponent,
        [[SoundEmitterGlobalComponent:SetSoundAsset - Calling not via ":" syntax.]]
    )

    assert ( type ( asset ) == "string", [[SoundEmitterGlobalComponent:SetSoundAsset - "asset" is not a string.]] )
    assert ( type ( looped ) == "boolean", [[SoundEmitterGlobalComponent:SetSoundAsset - "looped" is not a boolean.]] )

    return av_SoundEmitterGlobalComponentSetSoundAsset ( self._handle, asset, looped )
end

local function SetVolume ( self, volume )
    assert ( type ( self ) == "table" and self._type == eObjectType.SoundEmitterGlobalComponent,
        [[SoundEmitterGlobalComponent:SetVolume - Calling not via ":" syntax.]]
    )

    assert ( type ( volume ) == "number", [[SoundEmitterGlobalComponent:SetVolume - "volume" is not a number.]] )
    return av_SoundEmitterGlobalComponentSetVolume ( self._handle, volume )
end

local function Stop ( self )
    assert ( type ( self ) == "table" and self._type == eObjectType.SoundEmitterGlobalComponent,
        [[SoundEmitterGlobalComponent:Stop - Calling not via ":" syntax.]]
    )

    return av_SoundEmitterGlobalComponentStop ( self._handle )
end

-- Engine event handlers
local function OnDestroy ( self )
    av_SoundEmitterGlobalComponentDestroy ( self._handle )
end

-- Metamethods
local mt = {
    __gc = function ( self )
        av_SoundEmitterGlobalComponentCollectGarbage ( self._handle )
    end
}

-- This function is exported to C++ side.
function RegisterSoundEmitterGlobalComponent ( handle )
    local obj = Component ( eObjectType.SoundEmitterGlobalComponent, handle )

    -- Methods
    obj.GetVolume = GetVolume
    obj.IsPlaying = IsPlaying
    obj.Pause = Pause
    obj.Play = Play
    obj.SetSoundAsset = SetSoundAsset
    obj.SetVolume = SetVolume
    obj.Stop = Stop

    -- Engine events
    obj.OnDestroy = OnDestroy

    return setmetatable ( obj, mt )
end

local function Constructor ( self, name, soundChannel )
    assert ( type ( name ) == "string", [[SoundEmitterGlobalComponent - "name" is not a string."]] )
    assert ( type ( soundChannel ) == "number", [[SoundEmitterGlobalComponent - "soundChannel" is not a number."]] )

    assert ( soundChannel >= 0 and soundChannel < eSoundChannel.TOTAL,
        "SoundEmitterGlobalComponent - Incorrect sound channel. Component name: " .. name
    )

    return RegisterSoundEmitterGlobalComponent ( av_SoundEmitterGlobalComponentCreate ( name, soundChannel ) )
end

setmetatable ( SoundEmitterGlobalComponent, { __call = Constructor } )

-- Module contract
return nil
