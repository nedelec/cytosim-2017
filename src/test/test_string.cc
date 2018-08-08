
#include "exceptions.h"
#include <iostream>
#include <string>

using std::string;

int main(int argc, char *argv[]) 
{
    std::cout << " Hello World!\n";
    
    try {
        throw InvalidParameter("illegal code ", 10, "=3");
    } catch( Exception & e ) {
        std::cout << "[" << e.what() << "]\n";
    }
    
    return 0;
}