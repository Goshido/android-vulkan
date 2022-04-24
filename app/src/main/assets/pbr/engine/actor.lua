require "av://engine/object.lua"


Actor = {}

-- methods
local function AppendComponent ( self, component )
    local name = component:GetName ()
    local components = self._components
    local list = components[ name ]

    if not list then
        list = {}
        components[ name ] = list
    end

    table.insert ( list, component )
end

local function FindComponent ( self, name )
    local query = self._components[ name ]

    -- Lua style ternary operator
    return query and query[ 1 ] or nil
end

local function FindComponents ( self, name )
    return self._components[ name ]
end

-- helper
local function MakeActor ( handle, name )
    local obj = Object ( eObjectType.Actor )

    -- data
    obj._components = {}
    obj._handle = handle
    obj._name = name        -- TODO: replace by native getter method

    -- methods
    obj.AppendComponent = AppendComponent
    obj.FindComponent = FindComponent
    obj.FindComponents = FindComponents

    return obj
end

-- metamethods
local function Constructor ( self, name )
    return MakeActor ( av_ActorCreate ( name ), name )
end

setmetatable ( Actor, { __call = Constructor } )

-- module contract
RegisterActor = MakeActor
return nil
