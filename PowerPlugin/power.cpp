#include <cmath>
#include <stdexcept>

// возвращение имени функции
extern "C" __declspec(dllexport)
const char* getName()
{
    return "power";
}

// возвращение количества аргументов
extern "C" __declspec(dllexport)
int getArgumentCount()
{
    return 2;
}

// выполнение вычисления
extern "C" __declspec(dllexport)
double calculate(const double* args, int count)
{
    if (count != 2) {
        throw std::runtime_error("Power требует два аргумента");
    }

    return pow(args[0], args[1]);
}