#ifndef STRING_BUFFER_H_INCLUDED
#define STRING_BUFFER_H_INCLUDED

#include <string> // C++ strings for std::stod
#include <string.h> // C-strings

namespace JParser
{
    namespace Support
    {
        struct StringBuffer
        {
        public:
            char *data;
        private:
            char *current, *end;
            size_t size;
        public:
            explicit StringBuffer( size_t _size ):
                data{ static_cast<char *>( calloc ( _size, 1 ) ) },
                current { data },
                end{ data + _size },
                size { _size }
            {
            }

            StringBuffer(): StringBuffer{ 255 }
            {
            }

            explicit StringBuffer( char const * ch ): StringBuffer{ strlen( ch ) + 1 }
            {
                strcpy( data, ch );
                current += size;
            }
            
            StringBuffer( StringBuffer && strbuf ):
                data( strbuf.data ),
                current( strbuf.current ),
                end( strbuf.end ),
                size( strbuf.size )
            {
                strbuf.data = nullptr;
                strbuf.current = nullptr;
                strbuf.end = nullptr;
                strbuf.size = 0;
            }

            //~ Too lazy to implement these
            StringBuffer& operator=( StringBuffer const & strbuf ) = delete;
            StringBuffer( StringBuffer const & ) = delete;

            StringBuffer& operator=( StringBuffer && buf )
            {
                if( this != &buf ){
                    free( data );
                    data = buf.data; buf.data = nullptr;
                    current = buf.current; buf.current = nullptr;
                    end = buf.end; buf.end = nullptr;
                    size = buf.size; buf.size = 0;
                }
                return *this;
            }
            ~StringBuffer()
            {
                free( data );
            }

            void clear()
            {
                if( current > data ){
                    memset( data, '\0', length() );
                    current = data;
                }
            }
            
            size_t length() const
            {
                return current - data;
            }
            
            size_t capacity() const
            {
                return size;
            }
            
            void enlarge_space()
            {
                size_t new_size = size * 2;
                char *new_data = static_cast< char * >( calloc( new_size, 1 ) );
                char *new_current = new_data + length();
                char *new_end = new_data + new_size;

                memcpy( new_data, data, size );
                free( data );
                
                data = new_data;
                current = new_current;
                end = new_end;
                size = new_size;
            }
            char& operator[]( size_t pos )
            {
                return data[ pos ];
            }
            
            char const & operator[]( size_t pos ) const
            {
                return data[ pos ];
            }
            
            StringBuffer& operator+=( char const * str )
            {
                append( str );
                return *this;
            }

            std::string to_string () const
            {
                return std::string { data };
            }

            int to_int() const
            {
                return std::stod( data );
            }
            void append( char const * str )
            {
                size_t length_of_string = strlen( str );
                while( current + length_of_string >= end ){
                    enlarge_space();
                }
                strcpy( current, str );
                current += length_of_string;
            }
            void append( std::string const & str )
            {
                append( str.c_str() );
            }
            
            void append( char const ch )
            {
                if( current + 1 >= end ){
                    enlarge_space();
                }
                *current = ch;
                ++current;
            }
            template< typename T >
            friend inline T &operator<<( T & os, StringBuffer const & strbuf )
            {
                return os << strbuf.data;
            }
        };
    }
}

#endif // STRING_BUFFER_H_INCLUDED
