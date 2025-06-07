#!/bin/bash

# ============================
# Объявление переменных
# ============================
BUILD_DIR=".project"
GENERATE_DOC="OFF"
GENERATE_TESTS="ON"
GENERATE_BENCHMARK="ON"
CONFIGURATION="Debug"  # дефолтный билд
EXTERNAL_DIR="external"

# ============================
# Проверка наличия CMake
# ============================
if ! command -v cmake &> /dev/null; then
    echo "ERROR: CMake is not installed or not in PATH."
    exit 1
fi

# ============================
# Проверка каталога external
# ============================
if [ ! -d "$EXTERNAL_DIR" ] || [ -z "$(ls -A "$EXTERNAL_DIR")" ]; then
    echo "ERROR: Directory \"$EXTERNAL_DIR\" is empty or does not exist."
    exit 1
fi

# ============================
# Запрос конфигурации сборки
# ============================
echo
echo "Select build configuration:"
echo "[Enter] - Debug (default)"
echo "R - Release"
echo "E - Exit script"

read -p "Your choice (Enter/R/E): " USER_CONFIG

case "$USER_CONFIG" in
    [Rr]) CONFIGURATION="Release" ;;
    [Ee]) echo "Exiting script..."; exit 0 ;;
    *) CONFIGURATION="Debug" ;;
esac

echo "Selected configuration: $CONFIGURATION"

# ============================
# Шаг 1: Подготовка build-папки
# ============================
if [ -d "$BUILD_DIR" ]; then
    echo "Deleting old build directory \"$BUILD_DIR\"..."
    rm -rf "$BUILD_DIR"
fi

mkdir "$BUILD_DIR"
cd "$BUILD_DIR" || exit 1

read -p "Step 1 done. Continue? (Y/N): " CONT
[[ "$CONT" =~ ^[Yy]$ ]] || { echo "Script stopped by user."; exit 0; }

# ============================
# Шаг 2: Конфигурация CMake
# ============================
echo
echo "========== Configuring CMake =========="
cmake -DGENERATE_TESTS=$GENERATE_TESTS -DGENERATE_BENCHMARK=$GENERATE_BENCHMARK -DGENERATE_DOC=$GENERATE_DOC -DCMAKE_BUILD_TYPE=$CONFIGURATION ..

if [ $? -ne 0 ]; then
    echo "Configuration error!"
    exit 1
fi

read -p "Step 2 done. Continue? (Y/N): " CONT
[[ "$CONT" =~ ^[Yy]$ ]] || { echo "Script stopped by user."; exit 0; }

# ============================
# Шаг 3: Сборка проекта
# ============================
echo
echo "========== Building Project (CMake --build) =========="
cmake --build . --config "$CONFIGURATION"

if [ $? -ne 0 ]; then
    echo "Build error!"
    exit 1
fi

read -p "Step 3 done. Continue? (Y/N): " CONT
[[ "$CONT" =~ ^[Yy]$ ]] || { echo "Script stopped by user."; exit 0; }

# ============================
# Шаг 4: Запуск тестов
# ============================
echo
echo "========== Running Tests (ctest) =========="
RTEST_JUNIT="rtest-junit.xml"
ctest -C "$CONFIGURATION" --output-on-failure --output-junit "$RTEST_JUNIT"

if [ $? -ne 0 ]; then
    echo "Test execution error!"
    exit 1
fi

echo "All tests have been completed successfully."
cd ..

read -p "Step 4 done. Continue? (Y/N): " CONT
[[ "$CONT" =~ ^[Yy]$ ]] || { echo "Script stopped by user."; exit 0; }

# ============================
# Шаг 5: Генерация графиков
# ============================
cd tools || exit 1

# Проверка наличия Python
if ! command -v python3 &> /dev/null; then
    echo "ERROR: Python is not installed or not in PATH."
    exit 1
fi

echo
echo "========== Creating and activating virtual environment =========="

if [ ! -d venv ]; then
    python3 -m venv venv
fi

source venv/bin/activate

RBENCHMARK="../$BUILD_DIR/rbenchmark.json"
CHARTS_DIR="../doc/charts"
IMPL_DIR="./impl"

pip install -r "$IMPL_DIR/requirements.txt"
python "$IMPL_DIR/plot-benchmarks.py" --input "$RBENCHMARK" --output "$CHARTS_DIR"

if [ $? -ne 0 ]; then
    echo "Generation of charts execution error!"
    exit 1
fi

echo "Generation of charts execution completed successfully."
