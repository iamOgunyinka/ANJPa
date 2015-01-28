#ifndef JSON_EXPRESSION_BUILDER_H_INCLUDED
#define JSON_EXPRESSION_BUILDER_H_INCLUDED

#include "Parser.hpp"

namespace JsonParser
{
    using namespace JParser;
    using namespace JLexer;
    
    struct Parser
    {
    public:
        typedef std::vector<JsonExpression> json_vector;
        typedef json_vector::size_type size_type;
        typedef json_vector::iterator iterator;
        typedef json_vector::const_iterator const_iterator;

    public:
        Parser( std::ifstream & file );
        Parser( std::string const & filename );
        ~Parser() {}
    public:
        iterator begin() { return root.begin(); }
        iterator end() { return root.end(); }
        const_iterator cbegin() const { return root.cbegin(); }
        const_iterator cend() const { return root.cend(); }
        size_type size() const { return root.size(); }

        void init_parser();

    private:
        inline void program();
        inline void block();
        inline void stmts();
        inline void stmt();
        inline void st();
        inline void st_helper();
        inline void value();
        inline void args();
        inline void args_helper();
        inline void terminals();
        
    private:
        json_vector root;
        Token current_token;
        Lexer lexer;
    
    };
}
#endif // JSON_EXPRESSION_BUILDER_H_INCLUDED
