
#include <iostream>



/*
 This is a test of C++ extensions
 defined by ISOC++11
 
 http://en.wikipedia.org/wiki/C%2B%2B11
 
 it should be compiled with icc -std=c++11
 */
int main ()
{
    int my_array[] = { 1, 2, 3, 4, 5 };
    for (int &x : my_array) {
        std::cout << x << std::endl;
    }
    return 0;
}
