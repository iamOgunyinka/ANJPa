#include <iostream>
#include "../jparser.hpp"

using namespace JsonParser;

void process( json_expr_ptr & obj )
{
    if( !obj ) return;

    std::cout << ( obj->isArray() ? "Array ": "Object " ) << "name is " << obj->get_key() << std::endl;
    for( std::size_t i = 0; i != obj->size(); ++i )
    {
        auto object = ( *obj )[i];
        if( object->isNull() || object->isBoolean() || object->isString() || object->isInteger() ){
            std::cout << "Name " << object->get_key() << ", Value is " << object->get_value() << std::endl;
        } else if( object->isArray() ){
            process( object );
        } else {
            process( object );
        }
    }
}

int main()
{
    JsonDocument document { "MOCK_DATA.json" };

    auto object = document.parse();
    
    process( object );
    return 0;
}
