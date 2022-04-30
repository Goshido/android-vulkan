-- Debug message.
function LogD ( format, ... )
    av_LogD ( string.format ( format, ... ) )
end

-- Error message.
function LogE ( format, ... )
    av_LogE ( string.format ( format, ... ) )
end

-- Info message.
function LogI ( format, ... )
    av_LogI ( string.format ( format, ... ) )
end

-- Warning message.
function LogW ( format, ... )
    av_LogW ( string.format ( format, ... ) )
end

-- Module contract
return nil
