#include <iostream>

extern "C" __declspec(dllexport)
void someOtherFunction()
{
    std::cout << "Это не плагин калькулятора" << std::endl;
}