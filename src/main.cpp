#include <exception>
#include <iostream>

#include "JsonTypes.h"
void test_json_types()
{
    try
    {
        std::string str("hello world");
        std::vector<simpleJson::JsonValue> vec;
        simpleJson::JsonValue json(100.23);
        std::cout << json.getval<simpleJson::JsonType::Float>() << std::endl;
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }
}

int main()
{
    //   std::cout << "hello JsonParser!" << std::endl;
    test_json_types();
    return 0;
}