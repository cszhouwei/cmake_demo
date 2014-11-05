#include "qos_client.h"
#include <boost/thread/thread.hpp>
#include <iostream>

void hello()
{
    std::cout << "hello in thread." << std::endl;
}

int main(int argc, char *argv[])
{
    std::string err_msg;
    ApiInitRoute(0, 0, 0.0, err_msg);
    boost::thread thrd(&hello);
    thrd.join();
    return 0;
}
