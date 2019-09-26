// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "test_param.h"
#include <cstdlib>
#include "glossary.h"

Glossary glos;
Prop     prop;

int number;

int main(int argc, char* argv[])
{
    glos.readFile("config.cym");
    printf("%lu keys:\n", glos.nb_keys());
    glos.write(std::cout);
    
    glos.set(number, "number");
    printf("number = %i\n", number);
    return EXIT_SUCCESS;
}
