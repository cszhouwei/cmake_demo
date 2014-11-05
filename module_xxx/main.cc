#include "public.h"
#include "demo.pb.h"
#include "Attr_API.h"
#include <iostream>

int main(int argc, char *argv[])
{
    std::cout << "Factorial(5)=" << Factorial(5);
    std::cout << demo::Request().DebugString();
    Attr_API(0, 1);
    return 0;
}
