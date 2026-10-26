#include <cmath>
#include <stdexcept>

// возвращение имени функции
extern "C" __declspec(dllexport)
const char* getName()
{
    return "ln";
}

// возвращение количества аргументов
extern "C" __declspec(dllexport)
int getArgumentCount()
{
    return 1;
}

// вычисление ln
extern "C" __declspec(dllexport)
double calculate(const double* args, int count)
{
    if (count != 1) {
        throw std::runtime_error("Ln требует один аргумент");
    }

    if (args[0] <= 0) {
        throw std::runtime_error(
            "Аргумент ln должен быть больше нуля"
        );
    }

    return log(args[0]);
}