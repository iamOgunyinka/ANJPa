#ifndef TOKEN_H_INCLUDED
#define TOKEN_H_INCLUDED

namespace JsonParser
{
    enum class TokenType
    {
        Invalid = -1,
        Open_Braces = 0,
        Close_Braces,
        Open_SquareBracket,
        Close_SquareBracket,
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
