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
        virtual void add_element( json_expr expr ) = 0;
        virtual std::string get_key () const = 0;
        virtual std::string get_value () = 0;
        virtual size_t size() const = 0;
        virtual json_expr& operator []( size_t ) = 0;

        virtual bool isNull() const = 0;
        virtual bool isBoolean() const = 0;
        virtual bool isInteger() const = 0;
        virtual bool isString() const = 0;
        virtual bool isArray() const = 0;
        virtual bool isObject() const = 0;
    };

    typedef JsonExpression::json_expr json_expr;
    typedef JsonExpression::const_json_expr const_json_expr;
    typedef std::vector< json_expr > json_expr_array;

    struct JsonTerminalExpression: public JsonExpression
    {
    protected:
        std::pair< std::string const , std::string const > child;
    public:
        JsonTerminalExpression( ): child { nullptr, nullptr } { }
        JsonTerminalExpression( std::string const & key, std::string const & value ): child { key, value } { }
        virtual size_t size() const override { return 1; }
        virtual std::string get_key() const override { return child.first; }
        virtual void add_element( json_expr ) override { }
        virtual std::string get_value() { return child.second; }
        virtual json_expr& operator []( size_t i ) { return dynamic_cast< json_expr &>( *this ); }

        virtual bool isArray() const { return false; }
        virtual bool isObject() const { return false; }
    };

    struct JsonBinaryExpression: JsonExpression
    {
    protected:
        std::pair< std::string, json_expr_array > child;
    public:
        typedef json_expr_array::size_type size_type;

        JsonBinaryExpression( std::string const & name ): child{ name, {} } {}
        virtual std::string get_key() const override { return child.first; }
        virtual std::string get_value() override { return ""; }
        virtual void add_element( json_expr expr ) override { child.second.push_back( expr ); }

        json_expr_array::iterator begin() { return child.second.begin(); }
        json_expr_array::const_iterator cbegin() const { return child.second.cbegin(); }

        json_expr_array::iterator end() { return child.second.end(); }
        json_expr_array::const_iterator cend() const { return child.second.cend(); }
        virtual json_expr& operator []( size_t i ) { return child.second[ i ]; }
        
        virtual size_type size() const { return child.second.size(); }

        virtual bool isNull() const override { return false; }
        virtual bool isBoolean() const override { return false; }
        virtual bool isInteger() const override { return false; }
        virtual bool isString() const override { return false; }
    };
    
    struct JObject: public JsonBinaryExpression
    {
    public:
        JObject( std::string const & name ): JsonBinaryExpression{ name } { }
        virtual JsonType get_type() const final { return JsonType::Object; }

        virtual bool isArray() const { return false; }
        virtual bool isObject() const { return true; }

    };

    struct JArray: public JsonBinaryExpression
    {
    public:
        JArray( std::string const & name ): JsonBinaryExpression { name } { }
        json_expr& operator []( size_t i ) { return child.second[ i ]; }
        virtual JsonType get_type() const final { return JsonType::Array; }

        virtual bool isArray() const { return true; }
        virtual bool isObject() const { return false; }
    };

    using namespace Support;
    
    struct JString: public JsonTerminalExpression
    {
    public:
        virtual JsonType get_type () const final { return JsonType::String; }
        JString( std::string const & name, std::string const & value ): JsonTerminalExpression{ name, value }
        {
        }
        virtual json_expr& operator []( size_t i ) { return dynamic_cast< json_expr &>( *this ); }

        virtual bool isNull() const override { return false; }
        virtual bool isBoolean() const override { return false; }
        virtual bool isInteger() const override { return false; }
        virtual bool isString() const override { return true; }
    };

    struct JInteger: public JsonTerminalExpression
    {
        virtual JsonType get_type () const final { return JsonType::Integer; }
        JInteger( std::string const & name, std::string const & value ): JsonTerminalExpression { name, value } {}

        virtual bool isNull() const override { return false; }
        virtual bool isBoolean() const override { return false; }
        virtual bool isInteger() const override { return true; }
        virtual bool isString() const override { return false; }
    };

    struct JNull: public JsonTerminalExpression
    {
        virtual JsonType get_type() const final { return JsonType::Null; }
        JNull( std::string const & name, std::string const & value ): JsonTerminalExpression{ name, value } {}

        virtual bool isNull() const override { return true; }
        virtual bool isBoolean() const override { return false; }
        virtual bool isInteger() const override { return false; }
        virtual bool isString() const override { return false; }
    };
    
    struct JBoolean: public JsonTerminalExpression
    {
        virtual JsonType get_type() const final { return JsonType::Boolean; }
        JBoolean( std::string const &name, std::string const &value ): JsonTerminalExpression { name, value } {}

        virtual bool isNull() const override { return false; }
        virtual bool isBoolean() const override { return true; }
        virtual bool isInteger() const override { return false; }
        virtual bool isString() const override { return false; }
    };

    inline namespace HelperFunctions
    {
        json_expr   make_object( std::string const & name ) { return std::make_shared< JObject > ( name ); }
        json_expr   make_array ( std::string const & name ) { return std::make_shared< JArray > ( name ); }
        json_expr   make_string( std::string const & key, std::string const & value ) { return std::make_shared< JString > ( key, value ); }
        json_expr   make_integer( std::string const & name, std::string const & c ) { return std::make_shared< JInteger > ( name, c ); }
        json_expr   make_bool( std::string const & name, std::string const &value ) { return std::make_shared< JBoolean > ( name, value ); }
        json_expr   make_null( std::string const & name, std::string const & value ) { return std::make_shared< JNull > ( name, value ); }
    }
}

#endif //PARSER_H_INCLUDED
