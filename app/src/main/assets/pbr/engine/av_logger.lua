-- Debug message.
function AVLogD ( format, ... )
    av_LogD ( string.format ( format, ... ) )
end

-- Error message.
function AVLogE ( format, ... )
    av_LogE ( string.format ( format, ... ) )
end

-- Info message.
function AVLogI ( format, ... )
    av_LogI ( string.format ( format, ... ) )
end

-- Warning message.
function AVLogW ( format, ... )
    av_LogW ( string.format ( format, ... ) )
end

-- module contract
return nil
