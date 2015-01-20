#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED

#include "Lexer.hpp"
#include <memory>
#include <vector>

namespace JParser
{
    class JsonExpression
    {
    public:
        using json_expr = typename std::shared_ptr< JsonExpression >;
        using const_json_expr = typename std::shared_ptr< JsonExpression const >;

        virtual JsonType get_type ( void ) const = 0;
        
        virtual json_expr get_key () = 0;
        virtual const_json_expr get_key () const = 0;
        virtual std::string to_string() const = 0;
    };

    typedef JsonExpression::json_expr json_expr;
    typedef JsonExpression::const_json_expr const_json_expr;
    typedef std::vector< json_expr > json_expr_array;
    
    struct JsonTerminalExpression: public JsonExpression
    {
    protected:
        std::pair< json_expr, json_expr > child;
    public:
        JsonTerminalExpression( ): child { nullptr, nullptr } { }
        JsonTerminalExpression( json_expr key, json_expr value ): child { key, value } { }
        virtual json_expr get_key() override { return child.first; }
        virtual const_json_expr get_key() const override { return child.first; }
        virtual std::string to_string() const override { return get_key()->to_string(); }

        virtual json_expr get_value() { return child.second; }
        virtual const_json_expr get_value() const { return child.second; }
    };

    struct JsonBinaryExpression: JsonExpression
    {
    protected:
        std::pair< json_expr, json_expr_array > child;
    public:
        typedef json_expr_array::size_type size_type;

        JsonBinaryExpression( json_expr key ): child{ key, {} } {}
        virtual json_expr get_key() override { return child.first; }
        virtual const_json_expr get_key() const override { return child.first; }
        virtual std::string to_string() const override { return get_key()->to_string(); }
        virtual void add_element( json_expr expr ){ child.second.push_back( expr ); }

        json_expr_array::iterator begin() { return child.second.begin(); }
        json_expr_array::const_iterator cbegin() const { return child.second.cbegin(); }

        json_expr_array::iterator end() { return child.second.end(); }
        json_expr_array::const_iterator cend() const { return child.second.cend(); }
        
        virtual size_type size() const { return child.second.size(); }
    };
    
    struct JObject: public JsonBinaryExpression
    {
    public:
        JObject( json_expr key ): JsonBinaryExpression{ key } { }
        virtual JsonType get_type() const final { return JsonType::Object; }
    };

    struct JArray: public JsonBinaryExpression
    {
    public:
        JArray( json_expr key ): JsonBinaryExpression { key } { }
        json_expr& operator []( size_t i ) { return child.second[ i ]; }
        virtual JsonType get_type() const final { return JsonType::Array; }
    };

    using namespace Support;
    
    struct JString: public JsonTerminalExpression
    {
    private:
        StringBuffer key_string, value_string;
    public:
        virtual JsonType get_type () const final { return JsonType::String; }
        JString( StringBuffer && key, StringBuffer && value ):
                key_string( std::move( key ) ),
                value_string( std::move( value ) )
        {
        }
        
        std::string to_string() const override { return key_string.to_string(); }
    };

    struct JInteger: public JsonTerminalExpression
    {
        virtual JsonType get_type () const final { return JsonType::Integer; }
        JInteger( double const & c ): m_c { c } {}

    private:
        double m_c;
    };

    struct JNull: public JsonTerminalExpression
    {
        virtual JsonType get_type() const final { return JsonType::Null; }
        JNull( json_expr key, json_expr value ): JsonTerminalExpression{ key, value } {}
    };
    
    struct JBoolean: public JsonTerminalExpression
    {
        virtual JsonType get_type() const final { return JsonType::Boolean; }
        JBoolean( json_expr key, json_expr value ): JsonTerminalExpression { key, value } {}
    };

    inline namespace HelperFunctions
    {
        json_expr   make_object( json_expr key ) { return std::make_shared< JObject > ( key ); }
        json_expr   make_array ( json_expr key ) { return std::make_shared< JArray > ( key ); }
        json_expr   make_string( StringBuffer && key, StringBuffer && value ) { return std::make_shared< JString > ( std::move( key ), std::move( value ) ); }
        json_expr   make_integer( double const & c = 0.0 ) { return std::make_shared< JInteger > ( c ); }
        json_expr   make_bool( json_expr key, json_expr value ) { return std::make_shared< JBoolean> ( key, value ); }
        json_expr   make_null( json_expr key, json_expr value ) { return std::make_shared< JNull > ( key, value ); }
    }
}

#endif //PARSER_H_INCLUDED
