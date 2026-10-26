#include <cmath>
#include <stdexcept>

// возвращение имени функции
extern "C" __declspec(dllexport)
const char* getName()
{
    return "sin";
}

// возвращение количества аргументов
extern "C" __declspec(dllexport)
int getArgumentCount()
{
    return 1;
}

// вычисление sin
extern "C" __declspec(dllexport)
double calculate(const double* args, int count)
{
    if (count != 1) {
        throw std::runtime_error("Sin требует один аргумент");
    }

    const double pi = 3.14159265358979323846;
    double radians = args[0] * pi / 180.0;

    return sin(radians);
}