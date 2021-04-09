#include "../lib/csp_std.hpp"

// This is the main file of the CSpydr standart library

void println(std::string msg)
{
    printf("%s\n", msg.c_str());
}

void print(std::string msg)
{
    printf("%s", msg.c_str());
}