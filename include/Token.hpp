#ifndef TOKEN_H_INCLUDED
#define TOKEN_H_INCLUDED

namespace JParser
{
    enum class TokenType
    {
        Invalid = -1,
        Open_Braces = 0,
        Close_Braces,
        Open_Sqbrac, //square bracket
        Close_Sqbrac,
        Comma,
        Colon,
        String,
        Integer,
        Boolean,
        Null
    };
    enum class JsonType
    {
        Object,
        Array,
        String,
        Integer,
        Boolean,
        Null
    };
}

#endif
