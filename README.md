# 📐 cpp-numeric-system — арифметика в нестандартных системах счисления

**`cpp-numeric-system`** — это C++17-библиотека, реализующая базовую арифметику, аналогичную работе с `int`, но в **альтернативных системах счисления**: стандартной бинарной и экспериментальной факториальной. Проект создан в рамках ВКР и ориентирован на исследование представления больших чисел.

---

## 🚀 Краткий обзор

- 🔢 **BinaryArithmetic** — работа с числами в двоичной форме через вектор слов.
- 🧮 **FactorialArithmetic** — факториальное представление чисел с эффективной упаковкой коэффициентов.
- ⚙️ Поддержка базовых арифметических и сравнительных операций.
- 🧪 Модульные тесты ([Google Test](https://github.com/google/googletest.git)), бенчмарки ([Google Benchmark](https://github.com/google/benchmark.git)).
- 📘 Генерация документации через [Doxygen](https://doxygen.nl/index.html).
- 🧰 [CMake](https://cmake.org/) и интеграция с [Python](https://www.python.org/)-скриптами для автоматизации.

---

## 📦 Структура библиотеки

Библиотека предоставляет два ключевых класса пространства имён `numsystem`:

### `numsystem::BinaryArithmetic`

- Использует **вектор слов**, представляющий бинарное значение.
- Подходит для моделирования работы с большими числами в **двоичной системе счисления**.

### `numsystem::FactorialArithmetic`

- Реализует **факториальную систему счисления**, где значение хранится в виде последовательности коэффициентов.
- Каждый коэффициент использует минимальное количество бит (`log2(Idx)`).
- Поддерживает значения до `Idx = 2^63 - 1`.

---

## ➕ Поддерживаемые операции

Оба класса реализуют базовые числовые и логические операции:

```cpp
+  -  *  /  %         // арифметика
<  <=  ==  !=  >=  >  // сравнение
```

Дополнительно доступны внешние функции:

```cpp
to_string(x)  // преобразование в строку
sqrt(x)       // квадратный корень
pow(x, y)     // возведение в степень
abs(x)        // модуль числа
```

> ❗ **Ограничение**: библиотека не поддерживает побитовые операции (`&`, `|`, `<<`, `>>` и др.).

---

## ⚙️ Сборка проекта

Для сборки необходим только **CMake**. Опционально — **Python**, если вы хотите автоматизировать построение графиков по результатам тестирования.

### 📂 Клонирование репозитория

```bash
git clone --recursive https://github.com/YaguraWizz/cpp-numeric-system.git
cd cpp-numeric-system
```

### 🔧 Сборка с опциями

```bash
cmake -S . -B build \
    -DGENERATE_TESTS=ON \
    -DGENERATE_BENCHMARK=ON \
    -DGENERATE_DOC=ON
cmake --build build --config Release
```

| Опция                | Назначение                            | По умолчанию |
| -------------------- | ------------------------------------- | ------------ |
| `GENERATE_TESTS`     | Включить сборку тестов (Google Test)  | `OFF`        |
| `GENERATE_BENCHMARK` | Включить бенчмарки (Google Benchmark) | `OFF`        |
| `GENERATE_DOC`       | Генерация документации через Doxygen  | `OFF`        |

---

## 🧪 Тестирование и бенчмарки

После сборки можно запустить тесты и производительные замеры:

```bash
ctest -C Release               # Все тесты
ctest -L unit -C Release       # Только модульные тесты
ctest -R rbenchmark -C Release # Только бенчмарки
```

На Windows/Linux доступны удобные скрипты:

```bash
tools/run-tests.bat    # или run-tests.sh
```

> При наличии Python скрипты предложат сгенерировать графики и проверят зависимости.

---

## 📈 Примеры


```cpp
#include "BinaryArithmetic.h"
#include "FactorialArithmetic.h"
#include <string>
#include <iostream>

int main() {

        using namespace numsystem;
        std::string largeNumStr1 = "123456789012345678901234567890";
        std::string largeNumStr2 = "98765432109876543210987654321";

        BinaryArithmetic binA(largeNumStr1);
        BinaryArithmetic binB(largeNumStr2);
        BinaryArithmetic binSum = binA + binB;
        std::cout << "Binary Sum:      " << to_string(binSum;);

        FactorialArithmetic factA(largeNumStr1);
        FactorialArithmetic factB(largeNumStr2);
        FactorialArithmetic factSum = factA + factB;
        std::cout << "Factorial Sum:   " << to_string(factSum;);

        bool expect = to_string(binSum) == to_string(factSum);

        if (expect) {
            std::cout << "Results match for addition!";
        } else {
           std::cout << "Results DO NOT match for addition!";
        }

        std::string numToMultiply = "5";
        BinaryArithmetic binProduct = binA * BinaryArithmetic(numToMultiply);
        FactorialArithmetic factProduct = factA * FactorialArithmetic(numToMultiply);

        expect = to_string(binProduct) == to_string(factProduct);

        std::cout << "Binary Product:  " << to_string(binProduct;);
        std::cout << "Factorial Prod.: " << to_string(factProduct;);
        if (expect) {
            std::cout << "Results match for multiplication!";
        } else {
            std::cout << "Results DO NOT match for multiplication!";
        }

    return 0;
}
```


---

## 📚 Документация

Полная документация по API и архитектуре библиотеки доступна в директории [`doc/`](https://github.com/YaguraWizz/cpp-numeric-system/tree/main/doc) репозитория.

> Документация сгенерирована автоматически с использованием **Doxygen** и охватывает все публичные компоненты библиотеки.

---

## 🧪 CI/CD и результаты тестирования

### ✅ Поддержка платформ и компиляторов

| ОС/Компилятор | GCC   | Clang | MSVC  |
|---------------|-------|-------|-------|
| Ubuntu        | ✅    | ✅     | N/A   |
| Windows       | N/A   | N/A   | ✅     |


> Таблица демонстрирует успешное прохождение тестов на целевых конфигурациях, подтверждая стабильность работы под поддерживаемыми компиляторами и ОС.

### ⚙️ Конфигурация системы CI

| OS/Compiler | Ядер CPU | Частота CPU (МГц) | L1D | L1I | L2U | L3U |
|---|---|---|---|---|---|---|
| ubuntu-latest-gcc | 4 | 3257 | 32KB(2sh) | 32KB(2sh) | 512KB(2sh) | 32768KB(4sh) |
| ubuntu-latest-clang | 4 | 3285 | 32KB(2sh) | 32KB(2sh) | 512KB(2sh) | 32768KB(4sh) |
| windows-latest-msvc | 4 | 2445 | 32KB(2sh) | 32KB(2sh) | 512KB(2sh) | 32768KB(4sh) |

> Системные характеристики, использованные для CI-бенчмарков.

### 📊 Результаты производительности

| Операция        | Ubuntu (GCC)       | Ubuntu (Clang)     | Windows (MSVC)     |
|-----------------|-------------------|-------------------|-------------------|
| Binary-Add      | 13535.61 нс        | 13636.75 нс        | **539.69 нс**      |
| Binary-Comparison | 434.67 нс          | 455.93 нс          | **19.32 нс**       |
| Binary-Div      | 4605572.40 нс      | 4844459.83 нс      | **262133.47 нс**   |
| Binary-Mod      | 7532847.36 нс      | 7606268.91 нс      | **414048.99 нс**   |
| Binary-Mul      | 12111914.09 нс     | 11862523.62 нс     | **602847.64 нс**   |
| Binary-Sub      | 15770.54 нс        | 15787.78 нс        | **880.40 нс**      |
| Factorial-Add   | 39433.84 нс        | 39657.02 нс        | **3938.98 нс**     |
| Factorial-Comparison | 1997.50 нс         | 1670.28 нс         | **170.49 нс**      |
| Factorial-Div   | 3403393.43 нс      | 3388927.19 нс      | **976273.77 нс**   |
| Factorial-Mod   | 5317064.44 нс      | 5122476.58 нс      | **1747667.46 нс**  |
| Factorial-Mul   | 4282199.65 нс      | 4265068.10 нс      | **1509638.81 нс**  |
| Factorial-Sub   | 42772.55 нс        | 43107.97 нс        | **4615.18 нс**     |


> Значения времени операций в наносекундах. Учитывается реальное время ожидания.

---

## ⚖️ Лицензия

Проект распространяется под лицензией **MIT**. Подробнее см. в файле [`LICENSE`](https://github.com/YaguraWizz/cpp-numeric-system/tree/main/LICENSE.txt).

---