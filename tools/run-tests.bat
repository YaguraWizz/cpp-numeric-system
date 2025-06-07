@echo off
setlocal enabledelayedexpansion

REM ============================
REM Объявление переменных и опций в начале
REM ============================
for %%I in ("%~dp0..\") do set "ROOT=%%~fI"
set "BUILD_DIR=.project"
set "GENERATE_DOC=OFF"
set "GENERATE_TESTS=ON"
set "GENERATE_BENCHMARK=ON"
set "CONFIGURATION=Debug"
set "EXTERNAL_DIR=%ROOT%external"

REM ============================
REM Проверка наличия CMake
REM ============================
where cmake >nul 2>&1
if errorlevel 1 (
    echo ERROR: CMake is not installed or not in PATH.
    pause
    exit /b 1
)

REM ============================
REM Проверка каталога external не пустой
REM ============================
if not exist "%EXTERNAL_DIR%\*" (
    echo ERROR: Directory "%EXTERNAL_DIR%" is empty or does not exist.
    pause
    exit /b 1
)

REM ============================
REM Запрос конфигурации сборки у пользователя
REM ============================
:ASK_CONFIG
echo.
echo Select build configuration:
echo [Enter] - Debug (default)
echo R - Release
echo E - Exit script

set /p USER_CONFIG=Your choice (Enter/R/E): 

if /i "%USER_CONFIG%"=="R" (
    set "CONFIGURATION=Release"
) else if /i "%USER_CONFIG%"=="E" (
    echo Exiting script...
    exit /b 0
) else if "%USER_CONFIG%"=="" (
    set "CONFIGURATION=Debug"
) else (
    echo Invalid option, please try again.
    goto ASK_CONFIG
)

echo Selected configuration: %CONFIGURATION%

set "SELECTED_CONFIG=%CONFIGURATION%"
cd ..
REM ============================
REM Шаг 1: Удаляем старую папку сборки и создаём заново
REM ============================
if exist "%BUILD_DIR%" (
    echo Deleting old build directory "%BUILD_DIR%"...
    rmdir /s /q "%BUILD_DIR%"
)

mkdir "%BUILD_DIR%"
cd "%BUILD_DIR%"

REM Пауза и запрос продолжить
:ASK_CONTINUE_1
set /p CONTINUE="Step 1 done. Continue? (Y/N): "
if /i "%CONTINUE%"=="N" (
    echo Script stopped by user.
    exit /b 0
) else if /i "%CONTINUE%"=="Y" (
    goto CONFIGURE
) else (
    echo Please enter Y or N.
    goto ASK_CONTINUE_1
)

:CONFIGURE
REM ============================
REM Шаг 2: Конфигурация CMake
REM ============================
echo.
echo ========== Configuring CMake ==========
cmake -DGENERATE_TESTS=%GENERATE_TESTS% -DGENERATE_BENCHMARK=%GENERATE_BENCHMARK% -DGENERATE_DOC=%GENERATE_DOC% -DCMAKE_BUILD_TYPE=%CONFIGURATION% ..
if errorlevel 1 (
    echo Configuration error!
    pause
    exit /b 1
)

REM Пауза и запрос продолжить
:ASK_CONTINUE_2
set /p CONTINUE="Step 2 done. Continue? (Y/N): "
if /i "%CONTINUE%"=="N" (
    echo Script stopped by user.
    exit /b 0
) else if /i "%CONTINUE%"=="Y" (
    goto BUILD
) else (
    echo Please enter Y or N.
    goto ASK_CONTINUE_2
)

:BUILD
REM ============================
REM Шаг 3: Сборка проекта
REM ============================
echo.
echo ========== Building Project (CMake --build) ==========
cmake --build . --config %CONFIGURATION%
if errorlevel 1 (
    echo Build error!
    pause
    exit /b 1
)

REM Пауза и запрос продолжить
:ASK_CONTINUE_3
set /p CONTINUE="Step 3 done. Continue? (Y/N): "
if /i "%CONTINUE%"=="N" (
    echo Script stopped by user.
    exit /b 0
) else if /i "%CONTINUE%"=="Y" (
    goto TESTS
) else (
    echo Please enter Y or N.
    goto ASK_CONTINUE_3
)

:TESTS
REM ============================
REM Шаг 4: Запуск тестов
REM ============================
echo.
echo ========== Running Tests (ctest) ==========
set "RTEST_JUNIT=rtest-junit.xml"
ctest -C %CONFIGURATION% --output-on-failure --output-junit %RTEST_JUNIT%
if errorlevel 1 (
    echo Test execution error!
    pause
    exit /b 1
)
echo All tests have been completed successfully.
cd ..

REM Пауза и запрос продолжить
:ASK_CONTINUE_4
set /p CONTINUE="Step 4 done. Continue? (Y/N): "
if /i "%CONTINUE%"=="N" (
    echo Script stopped by user.
    exit /b 0
) else if /i "%CONTINUE%"=="Y" (
    goto TOOLS
) else (
    echo Please enter Y or N.
    goto ASK_CONTINUE_4
)

:TOOLS
cd tools

REM ============================
REM Проверка наличия Python (только здесь)
REM ============================
where python >nul 2>&1
if errorlevel 1 (
    echo ERROR: Python is not installed or not in PATH.
    pause
    exit /b 1
)

REM ============================
REM Шаг 5: Генерация графиков через Python в virtualenv
REM ============================
echo.
echo ========== Creating and activating virtual environment ==========

if not exist venv (
    python -m venv venv
)

call venv\Scripts\activate.bat

set "RBENCHMARK=%~dp0..\%BUILD_DIR%\rbenchmark.json"
set "CHARTS_DIR=%~dp0..\doc\charts"
set "IMPL_DIR=%~dp0impl"

pip install -r "%IMPL_DIR%\requirements.txt"
python "%IMPL_DIR%\plot-benchmarks.py" --input "%RBENCHMARK%" --output "%CHARTS_DIR%"
if errorlevel 1 (
    echo Generation of charts execution error!
    pause
    exit /b 1
)

echo Generation of charts execution completed successfully.
pause

endlocal
