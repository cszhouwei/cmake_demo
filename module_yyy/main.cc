#include "public.h"
#include "demo.pb.h"
#include "qos_client.h"
#include <iostream>

int main(int argc, char *argv[])
{
    test_func();
    std::cout << demo::Request().DebugString();
    std::string err_msg;
    ApiInitRoute(0, 0, 0.0, err_msg);
    return 0;
}
