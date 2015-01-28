#ifndef LEXER_H_INCLUDED
#define LEXER_H_INCLUDED

#include "Support/StringBuffer.hpp"
#include "Token.hpp"
#include <stdexcept>

namespace JParser
{
    namespace JLexer
    {
        inline namespace JErrorMessages
        {
            struct InvalidToken: virtual std::runtime_error { InvalidToken(): std::runtime_error( "Invalid Token found\n" ){} };
            struct EndOfString: virtual std::runtime_error { EndOfString( char const * ch ): std::runtime_error( ch ) {} };
        }
        
        namespace HelperFunctions
        {
            inline bool is_space( int ch )
            {
                return isspace( ch );
            }
            inline bool is_alphabet( int ch )
            {
                return isalpha( ch ) || ch == '_';
            }
            inline bool is_numeric_constant( int ch )
            {
                return isdigit( ch );
            }
            inline bool is_alphanumeric( int ch )
            {
                return isalnum( ch );
            }
        }
        
        using namespace HelperFunctions;
        using namespace Support;
        
        struct Token
        {
            Token( char const &c, TokenType tk ):
                str_value( 2 ),
                token( tk )
            {
                str_value.append( c );
            }
            Token( Token const & ) = default;
            Token( Token && ) = default;
            
            Token& operator =( Token && tok )
            {
                str_value = std::move( tok.str_value );
                token = std::move( tok.token );
                return *this;
            }
            
            Token( StringBuffer && strbuf, TokenType tk ):
                str_value( std::move( strbuf ) ),
                token( tk )
            {
            }
            
            Token( char const *ch, TokenType tk ):
                str_value( ch ),
                token( tk )
            {
            }

            const StringBuffer& get_string() const
            {
                return str_value;
            }
            
            static inline Token punctuator( char const *buf, TokenType tk )
            {
                return Token{ buf, tk };
            }
            
        private:
            StringBuffer str_value;
            TokenType token;
        };
        
        struct Lexer
        {
        private:
            StringBuffer strbuf;
            size_t current_index;
            size_t end_of_file;
            char current_character;
            
        public:
            Lexer() = delete;
            explicit Lexer( char const * json_file ):
                strbuf{ json_file },
                current_index { 0 },
                end_of_file { strbuf.length() },
                current_character{ }
            {
                update_current_token();
            }

        private:
            inline void update_current_token()
            {
                if( eof() ){
                    return;
                }
                
                current_character = strbuf[current_index];
                ++current_index;
            }

        public:
            inline bool eof()
            {
                return current_index >= end_of_file;
            }

            Token get_next_token()
            {
                for( ; ; )
                {
                    switch( current_character )
                    {
                        case ' ': case '\t': case '\n': case '\v': case '\f':
                            update_current_token();
                            continue;
                        case '{':
                            update_current_token();
                            return Token::punctuator( "{", TokenType::Open_Braces );
                        case '}':
                            update_current_token();
                            return Token::punctuator( "}", TokenType::Close_Braces );
                        case '[':
                            update_current_token();
                            return Token::punctuator( "[", TokenType::Open_Sqbrac );
                        case ']':
                            update_current_token();
                            return Token::punctuator( "]", TokenType::Close_Sqbrac );
                        case ':':
                            update_current_token();
                            return Token::punctuator( ":", TokenType::Colon );
                        case ',':
                            update_current_token();
                            return Token::punctuator( ",", TokenType::Comma );
                        case '"':
                            return extract_string_literals();
                        case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': case '0': case '-':
                            return extract_integer_literals();
                        case 't': case 'f':
                            return extract_boolean_literals();
                        case 'n':
                            return extract_null_literals();
                        default:
                            update_current_token();
                            throw InvalidToken{ };
                    }
                }
            }

            Token extract_string_literals()
            {
                StringBuffer string_extracted {};
                update_current_token();
                
                while ( true ){
                    if( current_character == '\"' ) {
                        update_current_token();
                        break;
                    } else if ( current_character == '\\' ) {
                        string_extracted.append( current_character );
                        update_current_token();
                        switch( current_character ) {
                            case '\"':
                            case '\\':
                            case 'b':
                            case 'f':
                            case 'n':
                            case 'r':
                            case 't':
                                break;

                            default:
                                JLexer::InvalidToken{};

                        }
                    } else if( eof() ) {
                        throw EndOfString{ "Expected a \" before the end of string" };
                    }
                    string_extracted.append( current_character );
                    update_current_token();
                }
                return Token{ std::move( string_extracted ), TokenType::String };
            }

            Token extract_integer_literals()
            {
                StringBuffer buf;

                while( is_numeric_constant( current_character ) ){
                    buf.append( current_character );
                    update_current_token();
                }
                
                return Token { std::move( buf ), TokenType::Integer };
            }

            Token extract_null_literals()
            {
                char const * null_value = "null";
                StringBuffer buf( 5 );
                
                for( int i = 0; i != 4; ++i ){
                    if( current_character != null_value[ i ] ){
                        throw JErrorMessages::InvalidToken{};
                    }
                    buf.append( current_character );
                    update_current_token();
                }
                return Token{ std::move( buf ), TokenType::Null };
            }
            
            Token extract_boolean_literals()
            {
                char const *true_bool = "true", *false_bool = "false";
                
                if( current_character == 't' ){
                    if( memcmp( strbuf.data + current_index - 1, true_bool, 4 ) == 0 ){
                        current_index += 3;
                        update_current_token();
                        return Token{ true_bool, TokenType::Boolean };
                    } else {
                        throw JErrorMessages::InvalidToken{};
                    }
                } else if ( current_character == 'f' ){
                    if( memcmp( strbuf.data + current_index - 1, false_bool, 5 ) == 0 ){
                        current_index += 4;
                        update_current_token();
                        return Token{ false_bool, TokenType::Boolean };
                    } else {
                        throw JErrorMessages::InvalidToken {};
                    }
                }
                return Token { "", TokenType::Invalid };
            }
        };
    }
}

#endif // LEXER_H_INCLUDED
