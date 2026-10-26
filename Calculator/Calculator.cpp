#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <cctype>
#include <map>

using namespace std;

string expression;
int position = 0;

typedef double (*CalculateFunction)(const double*, int);
typedef int (*GetArgumentCountFunction)();

struct Plugin {
    HMODULE library;
    CalculateFunction calculate;
    GetArgumentCountFunction getArgumentCount;
};

map<string, Plugin> pluginFunctions;

double parseExpression();

void skipSpaces();

double parseFunction()
{
    skipSpaces();

    size_t start = position;

    while (position < expression.size() &&
        (isalpha(static_cast<unsigned char>(expression[position])) ||
            expression[position] == '_')) {
        position++;
    }

    if (start == position) {
        throw runtime_error("Ожидалось название функции");
    }

    string name = expression.substr(start, position - start);

    skipSpaces();

    if (position >= expression.size() || expression[position] != '(') {
        throw runtime_error("После названия функции ожидалась скобка");
    }

    position++;

    vector<double> args;

    skipSpaces();

    if (position < expression.size() && expression[position] != ')') {
        while (true) {
            args.push_back(parseExpression());

            skipSpaces();

            if (position < expression.size() && expression[position] == ',') {
                position++;
            }
            else {
                break;
            }
        }
    }

    skipSpaces();

    if (position >= expression.size() || expression[position] != ')') {
        throw runtime_error("Нет закрывающей скобки функции");
    }

    position++;

    auto it = pluginFunctions.find(name);

    if (it == pluginFunctions.end()) {
        throw runtime_error("Неизвестная функция: " + name);
    }

    if (args.size() != it->second.getArgumentCount()) {
        throw runtime_error("Неверное количество аргументов функции " + name);
    }

    try {
        return it->second.calculate(
            args.data(),
            static_cast<int>(args.size())
        );
    }
    catch (const exception& error) {
        throw runtime_error(
            "Ошибка плагина " + name + ": " + error.what()
        );
    }
    catch (...) {
        throw runtime_error("Неизвестная ошибка внутри плагина " + name);
    }
}

// пропуск пробелов
void skipSpaces() {
    while (position < expression.size() &&
        isspace(static_cast<unsigned char>(expression[position]))) {
        position++;
    }
}

// прочтение числа/выражений в скобках
double parseNumber() {
    skipSpaces();

    if (position < expression.size() && expression[position] == '(') {
        position++;

        double result = parseExpression();

        skipSpaces();
        if (position >= expression.size() || expression[position] != ')') {
            throw runtime_error("Нет закрывающей скобки");
        }

        position++;
        return result;
    }

    if (position < expression.size() && expression[position] == '-') {
        position++;
        return -parseNumber();
    }

    if (position < expression.size() && expression[position] == '+') {
        position++;
        return parseNumber();
    }

    skipSpaces();

    if (position < expression.size() &&
        (isalpha(static_cast<unsigned char>(expression[position])) ||
            expression[position] == '_')) {
        return parseFunction();
    }

    size_t start = position;

    while (position < expression.size() &&
        (isdigit(static_cast<unsigned char>(expression[position])) ||
            expression[position] == '.')) {
        position++;
    }

    if (start == position) {
        throw runtime_error("Ожидалось число");
    }

    return stod(expression.substr(start, position - start));
}

// возведение в степень через плагин
double parsePower() {
    double base = parseNumber();

    skipSpaces();

    if (position < expression.size() && expression[position] == '^') {
        position++;

        double exponent = parsePower();

        auto it = pluginFunctions.find("power");

        if (it == pluginFunctions.end()) {
            throw runtime_error("Плагин power не загружен");
        }

        double args[2] = { base, exponent };

        try {
            base = it->second.calculate(args, 2);
        }
        catch (const exception& error) {
            throw runtime_error(
                "Ошибка плагина power: " + string(error.what())
            );
        }
        catch (...) {
            throw runtime_error("Неизвестная ошибка внутри плагина power");
        }
    }

    return base;
}

// умножение и деление
double parseTerm() {
    double result = parsePower();

    while (true) {
        skipSpaces();

        if (position < expression.size() && expression[position] == '*') {
            position++;
            result *= parsePower();
        }
        else if (position < expression.size() && expression[position] == '/') {
            position++;

            double divisor = parsePower();

            if (divisor == 0) {
                throw runtime_error("Деление на ноль");
            }

            result /= divisor;
        }
        else {
            break;
        }
    }

    return result;
}

// сложение и вычитание
double parseExpression() {
    double result = parseTerm();

    while (true) {
        skipSpaces();

        if (position < expression.size() && expression[position] == '+') {
            position++;
            result += parseTerm();
        }
        else if (position < expression.size() && expression[position] == '-') {
            position++;
            result -= parseTerm();
        }
        else {
            break;
        }
    }

    return result;
}

vector<string> findPlugins()
{
    vector<string> plugins;

    WIN32_FIND_DATAA fileData;

    HANDLE searchHandle = FindFirstFileA(
        "plugins\\*.dll",
        &fileData
    );

    if (searchHandle == INVALID_HANDLE_VALUE) {
        return plugins;
    }

    do {
        if (!(fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            plugins.push_back(fileData.cFileName);
        }
    } while (FindNextFileA(searchHandle, &fileData));

    FindClose(searchHandle);

    return plugins;
}

int main() {
    setlocale(LC_ALL, "Russian");

    char currentPath[MAX_PATH];

    GetCurrentDirectoryA(MAX_PATH, currentPath);

    cout << "Рабочая папка: " << currentPath << endl;

    vector<string> plugins = findPlugins();

    if (plugins.empty()) {
        cout << "В папке plugins не найдено DLL-плагинов." << endl;
    }

    cout << "Найдено плагинов: " << plugins.size() << endl;

    for (const string& plugin : plugins) {
        cout << plugin << endl;
    }

    // цикл загрузки найденных плагинов
    for (const string& plugin : plugins) {
        string path = "plugins\\" + plugin;

        cout << "\nЗагрузка плагина: " << plugin << endl;

        HMODULE library = LoadLibraryA(path.c_str());

        if (library == NULL) {
            cout << "Не удалось загрузить плагин. Код ошибки: "
                << GetLastError() << endl;
            continue;
        }

        typedef const char* (*GetNameFunction)();

        CalculateFunction calculate =
            (CalculateFunction)GetProcAddress(library, "calculate");

        GetNameFunction getName =
            (GetNameFunction)GetProcAddress(library, "getName");

        GetArgumentCountFunction getArgumentCount =
            (GetArgumentCountFunction)GetProcAddress(
                library,
                "getArgumentCount"
            );

        if (calculate == NULL ||
            getName == NULL ||
            getArgumentCount == NULL) {
            cout << "В DLL отсутствуют необходимые функции" << endl;
            FreeLibrary(library);
            continue;
        }

        const char* functionName = getName();

        if (functionName == NULL || string(functionName).empty()) {
            cout << "У плагина отсутствует имя функции" << endl;
            FreeLibrary(library);
            continue;
        }

        if (pluginFunctions.find(functionName) != pluginFunctions.end()) {
            cout << "Функция " << functionName
                << " уже зарегистрирована. Плагин пропущен." << endl;
            FreeLibrary(library);
            continue;
        }

        cout << "Название функции: " << getName() << endl;

        cout << "Количество аргументов: "
            << getArgumentCount() << endl;

        Plugin plugin;
        plugin.library = library;
        plugin.calculate = calculate;
        plugin.getArgumentCount = getArgumentCount;

        pluginFunctions[getName()] = plugin;

        try {
            if (string(getName()) == "power") {
                double args[2] = { 2, 4 };
                cout << "Проверка: 2 и 4 -> "
                    << calculate(args, 2) << endl;
            }
            else if (string(getName()) == "sin") {
                double args[1] = { 90 };
                cout << "Проверка: sin(90) -> "
                    << calculate(args, 1) << endl;
            }
        }
        catch (const exception& error) {
            cout << "Ошибка плагина: " << error.what() << endl;
        }
    }

    cout << "Калькулятор. Введите выражение или exit для выхода." << endl;

    while (true) {
        cout << "> ";

        if (!getline(cin, expression)) {
            break;
        }

        if (expression == "exit") {
            break;
        }

        position = 0;

        try {
            double result = parseExpression();

            skipSpaces();

            if (position != expression.size()) {
                throw runtime_error("Лишние символы в выражении");
            }

            cout << "= " << result << endl;
        }
        catch (const exception& error) {
            cout << "Ошибка: " << error.what() << endl;
        }
    }

    for (auto& item : pluginFunctions) {
        FreeLibrary(item.second.library);
    }

    return 0;
}