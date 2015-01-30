#ifndef JSON_EXPRESSION_BUILDER_H_INCLUDED
#define JSON_EXPRESSION_BUILDER_H_INCLUDED

#include "Parser.hpp"
#include <fstream>

namespace JsonParser
{
    using namespace JParser;
    using namespace JLexer;
    
    struct Parser
    {
    public:
        Parser( std::string const & json_string );
        ~Parser();
    public:
        void init_parser();

        json_expr begin() { return root->begin(); }
        json_expr end() { return root->end(); }

        const_json_expr cbegin() const { return root->cbegin(); }
        const_json_expr cend() const { return root->cend(); }
        json_expr get_object() { return root; }
        
        size_t size() const { return root->size(); }
    private:
        inline void program_block_start( json_expr & );
        inline void statements( json_expr & );
        inline void other_statements( json_expr & );
        inline void other_statements_helper( json_expr & );

        inline void stmt( json_expr & );
        inline void value( std::string const &, json_expr & );
        inline void array_arguments( json_expr & );
        inline void other_array_arguments( json_expr & );

        inline void match( char ch, Token & );    
    private:
        json_expr root;
        Token current_token;
        Lexer lexer;
    };

    Parser::Parser( std::string const & json_string ):
        root{ nullptr },
        current_token { ' ', TokenType::Invalid },
        lexer{ json_string }
    {
    }

    Parser::~Parser()
    {
    }

    void Parser::init_parser()
    {
        program_block_start( root );
    }

    void Parser::program_block_start( json_expr & node )
    {
        current_token = lexer.get_next_token();
        if( current_token.get_type() != TokenType::Open_Braces ){
            throw JErrorMessages::InvalidToken { "Invalid Token found" };
        }
        node = make_object( "" );

        current_token = lexer.get_next_token();
        statements( node );

        if( current_token.get_type() != TokenType::Close_Braces ) {
            throw JErrorMessages::InvalidToken { "Invalid Token found" };
        }
    }

    void Parser::statements( json_expr & node )
    {
        if( current_token.get_type() == TokenType::Close_Braces ){//We found an empty json file
            return;
        }
        other_statements( node );
    }

    void Parser::other_statements( json_expr & node )
    {
        stmt( node );
        other_statements_helper( node );
    }

    void Parser::other_statements_helper( json_expr & node )
    {
        if( current_token.get_type() != TokenType::Comma ){
            return;
        }

        stmt( node );
        other_statements_helper( node );
    }

    void Parser::stmt( json_expr & node )
    {
        if( current_token.get_type() != TokenType::String ){
            throw JErrorMessages::InvalidToken { "Expected a string before" + current_token.get_lexeme().to_string() };
        }

        auto saved_token_name = current_token.get_lexeme().to_string();
        current_token = lexer.get_next_token();
        
        if( current_token.get_type() != TokenType::Colon ){
            throw JErrorMessages::InvalidToken{ "Expected a colon seperator before " + current_token.get_lexeme().to_string() };
        }
        current_token = lexer.get_next_token();
        value( saved_token_name, node );
    }
    
    void Parser::value( std::string const & saved_token_name, json_expr & node )
    {
        json_expr value_consumer = nullptr;
        
        switch( current_token.get_type() )
        {
            case TokenType::Null:
                node->add_element( make_null( saved_token_name, current_token.get_lexeme().to_string() ) );
                current_token = lexer.get_next_token();
                break;
            case TokenType::Boolean:
                node->add_element( make_bool( saved_token_name, current_token.get_lexeme().to_string() ) );
                current_token = lexer.get_next_token();
                break;
            case TokenType::String:
                node->add_element( make_string( saved_token_name, current_token.get_lexeme().to_string() ) );
                current_token = lexer.get_next_token();
                break;
            case TokenType::Integer:
                node->add_element( make_integer( saved_token_name, current_token.get_lexeme().to_string() ) );
                current_token = lexer.get_next_token();
                break;
            case TokenType::Open_Sqbrac:
                value_consumer = make_array( saved_token_name );
                current_token = lexer.get_next_token();
                array_arguments( value_consumer );
                node->add_element( value_consumer );
                match( ']', current_token );
                break;
            case TokenType::Open_Braces:
                value_consumer = make_object( saved_token_name );
                current_token = lexer.get_next_token();
                other_statements( value_consumer );
                node->add_element( value_consumer );
                match( '}', current_token );
            default:
                throw JErrorMessages::InvalidToken { "Invalid Token found" };
                break;
        }
    }

    void Parser::array_arguments( json_expr & node )
    {
        value( "", node ); //~ array arguments are nameless
        other_array_arguments( node );
    }

    void Parser::other_array_arguments( json_expr & node )
    {
        if( current_token.get_type() == TokenType::Comma ){
            value( "", node );
            other_array_arguments( node );
        }
        return;
    }
    
    void Parser::match( char ch, Token & tk )
    {
        if( ch != tk.get_lexeme().to_string()[0] ){
            throw JErrorMessages::InvalidToken { "Expected a string before" + current_token.get_lexeme().to_string() };
        }
        tk = lexer.get_next_token();
    }

    struct JsonDocument
    {
        JsonDocument( std::ifstream & file );
        JsonDocument( std::string const & filename );
        ~JsonDocument();

        json_expr parse();
    private:
        std::string m_filename;
        std::unique_ptr< std::ifstream > ptr;
        std::ifstream & m_file;
    };

    json_expr JsonDocument::parse()
    {
        std::string json_string{};

        if( m_file ){
            std::string lines{};
            while( std::getline( m_file, lines ) ){
                json_string += lines;
                json_string += "\n";
            }
        }
        Parser parser { json_string };
        parser.init_parser();
        return parser.get_object();
    }
    
    JsonDocument::JsonDocument( std::ifstream & file ):
        m_filename {},
        ptr { nullptr },
        m_file ( file )
    {
    }

    JsonDocument::JsonDocument( std::string const & filename ):
        m_filename{ filename },
        ptr { new std::ifstream { filename } },
        m_file ( *ptr )
    {
    }

    JsonDocument::~JsonDocument() = default;
}
#endif // JSON_EXPRESSION_BUILDER_H_INCLUDED
