import os
import json
import argparse
import re
import matplotlib.pyplot as plt

def parse_args():
    """
    Разбираем аргументы командной строки:
      1) --input  : путь к rbenchmark.json
      2) --output : путь к папке, куда сохранять графики
    Если папки output нет – создаём её.
    """
    parser = argparse.ArgumentParser(
        description="Парсит rbenchmark.json и строит графики real_time для операций Binary vs Factorial."
    )
    parser.add_argument(
        "--input",
        "-i",
        required=True,
        help="Путь к rbenchmark.json"
    )
    parser.add_argument(
        "--output",
        "-o",
        required=True,
        help="Путь к директории для сохранения графиков"
    )
    args = parser.parse_args()

    # Проверка: существует ли входной JSON-файл
    if not os.path.isfile(args.input):
        print(f"[ERROR] File not found: {args.input}")
        sys.exit(1)

    # Если папки нет, создаём
    if not os.path.isdir(args.output):
        os.makedirs(args.output, exist_ok=True)
    return args

def load_data(path):
    """
    Считываем JSON из файла и возвращаем список записей (benchmarks).
    """
    with open(path, "r", encoding="utf-8") as f:
        data = json.load(f)
    # В формате Google Benchmark данные лежат в ключе "benchmarks"
    return data.get("benchmarks", [])

def extract_info(entry):
    """
    Извлекаем из поля 'name' три компонента:
      - system (Binary или Factorial)
      - operation (Add, Sub, Mul, Div, Mod, Comparison)
      - size (число, например 10, 16, 32...)
    Ожидаем, что name формата: "Binary-Add/10/min_time:0.010"
    """
    name = entry.get("name", "")
    m = re.match(r"(?P<system>\w+)-(?P<op>\w+)/(?P<size>\d+)", name)
    if not m:
        return None, None, None
    system = m.group("system")
    op = m.group("op")
    size = int(m.group("size"))
    return system, op, size

def collect_data(benchmarks):
    """
    Собираем из списка benchmarks словарь вида:
      data[operation][system] = list of (size, real_time)
    Где operation – строка "Add", "Sub", ...
    system – "Binary" или "Factorial"
    Оставляем только записи run_type == "iteration".
    """
    data = {}
    for entry in benchmarks:
        if entry.get("run_type") != "iteration":
            continue

        system, op, size = extract_info(entry)
        if system is None:
            continue  # если не удалось распознать формат имени

        real_time = entry.get("real_time", None)
        if real_time is None:
            continue

        # Инициализируем вложенные словари
        data.setdefault(op, {})
        data[op].setdefault(system, [])
        # Добавляем кортеж (size, real_time)
        data[op][system].append((size, real_time))

    # Сортируем по size внутри каждой пары (чтобы график строился «по возрастанию»)
    for op in data:
        for system in data[op]:
            data[op][system].sort(key=lambda x: x[0])

    return data

def plot_operation(op_name, sys_data, output_dir):
    """
    Для одной операции op_name (например, "Add") строим график:
      - sys_data: словарь { "Binary": [(size, time), ...], "Factorial": [...] }
      - Сохраняем в файл f"{output_dir}/график_{op_name}.png"
    Подписи:
      X: Размер числа (количество условных единиц)
      Y: Время выполнения, нс (логарифмический масштаб)
    Легенда: "Бинарная система" и "Факториальная система"
    """
    plt.figure(figsize=(8, 6))

    # Цвета и стили линий можно поменять, но не задаём вручную, чтобы не нарушать ГОСТ.
    for system, points in sys_data.items():
        sizes = [p[0] for p in points]
        times = [p[1] for p in points]
        label = "Бинарная система" if system == "Binary" else "Факториальная система"
        plt.plot(
            sizes,
            times,
            marker="o",
            linestyle="-",
            label=label
        )

    # Добавляем заголовок сверху с названием операции
    plt.title(f"График времени выполнения операции: {op_name}", fontsize=14, fontweight='bold')

    # Подписи осей по ГОСТ: на русском, шрифт обычно Times New Roman, но matplotlib –– стандарт.
    plt.xlabel("Размер числа, условные единицы", fontsize=12)
    plt.ylabel("Время выполнения, нс (логарифмический масштаб)", fontsize=12)

    # Логарифмический масштаб по оси Y
    plt.yscale("log")

    # Сетка
    plt.grid(True, which="both", linestyle="--", linewidth=0.5)

    # Легенда (обычно справа сверху, но можно внизу, в зависимости от ГОСТ; здесь справа)
    plt.legend(loc="upper left", fontsize=10)

    # Немного отступов, чтобы подписи не обрезались
    plt.tight_layout()

    # Сохраняем файл
    filename = f"chart_{op_name}.png"
    out_path = os.path.join(output_dir, filename)
    plt.savefig(out_path, dpi=300)
    plt.close()

def main():
    args = parse_args()
    benchmarks = load_data(args.input)
    data = collect_data(benchmarks)

    # Для каждой операции строим отдельный график
    for op_name, sys_data in data.items():
        # Если для какой-то системы нет данных – пропускаем
        if "Binary" not in sys_data or "Factorial" not in sys_data:
            continue
        plot_operation(op_name, sys_data, args.output)

    print("Graphs have been successfully saved in the directory:", args.output)

if __name__ == "__main__":
    main()
