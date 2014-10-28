#include "public.h"
#include "demo.pb.h"
#include "Attr_API.h"
#include <iostream>

int main(int argc, char *argv[])
{
    test_func();
    std::cout << demo::Request().DebugString();
    Attr_API(0, 1);
    return 0;
}
