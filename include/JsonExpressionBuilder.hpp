#ifndef JSON_EXPRESSION_BUILDER_H_INCLUDED
#define JSON_EXPRESSION_BUILDER_H_INCLUDED

#include <fstream>
#include "Parser.hpp"

namespace JsonParser
{
    struct Parser
    {
    public:
        Parser( std::string const & json_string );
        ~Parser();
    public:
        
        json_expr_ptr begin() { return root->begin(); }
        json_expr_ptr end() { return root->end(); }

        const_json_expr_ptr cbegin() const { return root->cbegin(); }
        const_json_expr_ptr cend() const { return root->cend(); }

        json_expr_ptr get_object() { return root; }
        
        std::size_t size() const { return root->size(); }
        bool is_empty();
    private:
        inline void program_block_start( json_expr_ptr & );
        inline void statements( json_expr_ptr & );
        inline void other_statements( json_expr_ptr & );
        inline void other_statements_helper( json_expr_ptr & );

        inline void stmt( json_expr_ptr & );
        inline void value( json_expr_ptr &, std::string const & );
        inline void array_arguments( json_expr_ptr &, std::string const &name = "" );
        inline void other_array_arguments( json_expr_ptr & );

        inline void match( char ch, Token & );    
    private:
        json_expr_ptr root;
        Token current_token;
        Lexer lexer;
        bool found_empty_file;
        
    };

    Parser::Parser( std::string const & json_string ):
        root{ nullptr },
        current_token { ' ', TokenType::Invalid },
        lexer{ json_string },
        found_empty_file { false }
    {
        program_block_start( root );
    }

    Parser::~Parser()
    {
    }

    void Parser::program_block_start( json_expr_ptr & node )
    {
        current_token = lexer.get_next_token();
        std::string const & node_name = "__ROOT_ELEMENT__";

        if( current_token.get_type() == TokenType::Open_Braces ){
            node = make_object( node_name );

            current_token = lexer.get_next_token();
            statements( node );

            if( current_token.get_type() != TokenType::Close_Braces ){
                throw JErrorMessages::InvalidToken { "Invalid Token found at the end of document. Expected a closing braces '}'" };
            }
        } else if ( current_token.get_type() == TokenType::Open_SquareBracket ){
            node = make_array( node_name );
            
            current_token = lexer.get_next_token();
            array_arguments( node );

            if( current_token.get_type() != TokenType::Close_SquareBracket ){
                throw JErrorMessages::InvalidToken { "Invalid Token found at the end of document. "
                                                        "Expected a closing square bracket ']'" };
            }
        } else {
            throw JErrorMessages::InvalidToken { "Invalid Token found. Expected a Json Object at the start of document." };
        }
    }

    void Parser::statements( json_expr_ptr & node )
    {
        if( current_token.get_type() == TokenType::Close_Braces ){
            found_empty_file = true;
            return;
        }
        other_statements( node );
    }

    bool Parser::is_empty()
    {
        return found_empty_file;
    }
    
    void Parser::other_statements( json_expr_ptr & node )
    {
        stmt( node );
        other_statements_helper( node );
    }

    void Parser::other_statements_helper( json_expr_ptr & node )
    {
        if( current_token.get_type() != TokenType::Comma ){
            return;
        }
        current_token = lexer.get_next_token();
        stmt( node );
        other_statements_helper( node );
    }

    void Parser::stmt( json_expr_ptr & node )
    {
        if( current_token.get_type() != TokenType::String ){
            throw JErrorMessages::InvalidToken { "Expected a string before '" + current_token.get_lexeme().to_string() + "'" };
        }

        auto saved_token_name = current_token.get_lexeme().to_string();
        current_token = lexer.get_next_token();
        
        if( current_token.get_type() != TokenType::Colon ){
            throw JErrorMessages::InvalidToken{ "Expected a colon seperator before " + current_token.get_lexeme().to_string() };
        }
        current_token = lexer.get_next_token();
        value( node, saved_token_name );
    }
    
    void Parser::value( json_expr_ptr & node, std::string const & saved_token_name )
    {
        json_expr_ptr value_consumer = nullptr;
        
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
            case TokenType::Open_SquareBracket:
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
                return;
        }
    }

    void Parser::array_arguments( json_expr_ptr & node, std::string const &name )
    {
        value( node, name );
        other_array_arguments( node );
    }

    void Parser::other_array_arguments( json_expr_ptr & node )
    {
        if( current_token.get_type() == TokenType::Comma ){
            current_token = lexer.get_next_token();
            value( node, "" );
            other_array_arguments( node );
        }
    }
    
    void Parser::match( char ch, Token & tk )
    {
        if( ch != tk.get_lexeme().to_string()[0] ){
            throw JErrorMessages::InvalidToken { "Expected a string before '" + current_token.get_lexeme().to_string() + "'" };
        }
        tk = lexer.get_next_token();
    }

    struct JsonDocument
    {
        JsonDocument( std::ifstream & file );
        JsonDocument( std::string const & filename );
        ~JsonDocument();

        json_expr_ptr parse();
    private:
        std::string m_filename;
        std::unique_ptr< std::ifstream > ptr;
        std::ifstream & m_file;
    };

    json_expr_ptr JsonDocument::parse()
    {
        std::string json_string{};

        if( m_file ){
            std::string lines{};
            while( std::getline( m_file, lines ) ){
                json_string += lines;
            }
        }
        Parser parser { json_string };
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
