require "av://engine/object.lua"


UILayer = {}

-- Methods
local function Find ( self, id )
    assert ( type ( self ) == "table" and self._type == eObjectType.UILayer,
        [[UILayer:Find - Calling not via ":" syntax.]]
    )

    assert ( type ( id ) == "string", [[UILayer:Find - "id" is not string.]] )
    return av_UILayerFind ( self._handle, id )
end

local function Hide ( self )
    assert ( type ( self ) == "table" and self._type == eObjectType.UILayer,
        [[UILayer:Hide - Calling not via ":" syntax.]]
    )

    av_UILayerHide ( self._handle )
end

local function Show ( self )
    assert ( type ( self ) == "table" and self._type == eObjectType.UILayer,
        [[UILayer:Show - Calling not via ":" syntax.]]
    )

    av_UILayerShow ( self._handle )
end

local function IsVisible ( self )
    assert ( type ( self ) == "table" and self._type == eObjectType.UILayer,
        [[UILayer:IsVisible - Calling not via ":" syntax.]]
    )

    return av_UILayerIsVisible ( self._handle )
end

-- Metamethods
local mt = {
    __gc = function ( self )
        av_UILayerCollectGarbage ( self._handle )
    end
}

local function Constructor ( self, path )
    assert ( type ( path ) == "string", [[UILayer:Constructor - "path" is not string.]] )

    local obj = Object ( eObjectType.UILayer )

    -- Data
    obj._handle = av_UILayerCreate ( path )

    -- Methods
    obj.Find = Find
    obj.Hide = Hide
    obj.Show = Show
    obj.IsVisible = IsVisible

    return setmetatable ( obj, mt )
end

setmetatable ( UILayer, { __call = Constructor } )

-- Module contract
return nil