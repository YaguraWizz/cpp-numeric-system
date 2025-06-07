import sys
import os
import argparse
from pathlib import Path
from ci_parser import parse_junit_xml, parse_benchmark_json
from generators.compatibility import generate_compatibility_table
from generators.system_info import generate_benchmark_system_info_table
from generators.benchmark_table import generate_benchmark_table
from example_processor import insert_examples

# Абсолютный путь к корню проекта
script_dir = os.path.dirname(os.path.abspath(__file__))
project_root = os.path.abspath(os.path.join(script_dir, '..', '..'))

if project_root not in sys.path:
    sys.path.insert(0, project_root)


def parse_arguments():
    parser = argparse.ArgumentParser(description="Генерация README.md на основе результатов CI.")
    parser.add_argument("--input-folder", required=True, help="Путь к каталогу артефактов.")
    parser.add_argument("--output-readme", default="README.md", help="Файл для сохранения README.")
    parser.add_argument("--template-path", default="README.md.template", help="Путь к шаблону.")
    return parser.parse_args()


def check_input_paths(input_base_path: Path, template_path: Path) -> Path | None:
    if not input_base_path.exists():
        print(f"[ERROR] Каталог артефактов '{input_base_path}' не найден.")
        return None

    if template_path.is_dir():
        template_path = template_path / "README.md.template"

    return template_path


def process_artifacts(input_base_path: Path, expected_combinations: list[str]):
    print("[INFO] Начинаю сбор данных из каталогов артефактов... ", end="", flush=True)
    results = {}

    for combo in expected_combinations:
        print(f"\n  Обрабатываю конфигурацию: {combo} ... ", end="", flush=True)
        combo_path = input_base_path / combo
        test_xml = combo_path / ".project" / "rtest-junit.xml"
        bench_json = combo_path / ".project" / "rbenchmark.json"

        test_results = {"total": 0, "failures": 0, "errors": 0, "skipped": 0, "passed": False}
        benchmark_data = {"benchmarks": [], "system_info": {}}

        if combo_path.is_dir():
            if test_xml.is_file():
                print("парсю тесты... ", end="", flush=True)
                test_results = parse_junit_xml(test_xml)
                print("ok", end=", ", flush=True)
            else:
                print("тесты отсутствуют", end=", ", flush=True)

            if bench_json.is_file():
                print("парсю бенчмарки... ", end="", flush=True)
                benchmark_data = parse_benchmark_json(bench_json)
                print("ok", end="", flush=True)
            else:
                print("бенчмарки отсутствуют", end="", flush=True)
        else:
            print("каталог отсутствует", end="", flush=True)

        results[combo] = {
            "test_results": test_results,
            "benchmark_results": benchmark_data.get("benchmarks", []),
            "system_info": benchmark_data.get("system_info", {})
        }

    print("\n[INFO] Сбор данных завершён. ok")
    return results


def should_abort_due_to_failed_ci(results: dict) -> bool:
    for combo, data in results.items():
        if data["test_results"]["total"] == 0:
            print(f"[ERROR] CI для '{combo}' не выполнил тесты. Прерывание генерации README.")
            return True
    return False


def generate_sections(results: dict):
    print("[INFO] Генерирую таблицу совместимости... ", end="", flush=True)
    compatibility = generate_compatibility_table(results)
    print("ok")

    print("[INFO] Генерирую информацию о системе... ", end="", flush=True)
    system_info_dict = {k: v["system_info"] for k, v in results.items() if v["system_info"]}
    if system_info_dict:
        system_info = generate_benchmark_system_info_table(system_info_dict)
        print("ok")
    else:
        system_info = "Информация о системе для бенчмарков недоступна.\n"
        print("отсутствует")

    print("[INFO] Генерирую таблицу бенчмарков... ", end="", flush=True)
    benchmark_table = generate_benchmark_table(results)
    print("ok")

    return compatibility, system_info, benchmark_table


def build_readme(template_path: Path, output_path: Path, compatibility_md: str, system_info_md: str, benchmark_md: str):
    try:
        print(f"[INFO] Читаю шаблон из {template_path} ... ", end="", flush=True)
        with open(template_path, 'r', encoding='utf-8') as f:
            template = f.read()
        print("ok")

        print("[INFO] Заменяю макросы... ", end="", flush=True)
        readme = template.replace("{{COMPATIBILITY_TABLE}}", compatibility_md)
        readme = readme.replace("{{SYSTEM_INFO}}", system_info_md)
        readme = readme.replace("{{BENCHMARK_TABLE}}", benchmark_md)
        print("ok")

        examples_path = Path(project_root) / "tests" / "examples"
        print(f"[INFO] Вставляю примеры из {examples_path} ... ", end="", flush=True)
        readme = insert_examples(readme, examples_path)
        print("ok")

        print(f"[INFO] Записываю README в {output_path} ... ", end="", flush=True)
        with open(output_path, 'w', encoding='utf-8') as f:
            f.write(readme)
        print("ok")

        print(f"\n[УСПЕХ] README.md сгенерирован по пути: {output_path.resolve()}")

    except Exception as e:
        print(f"\n[ОШИБКА] {type(e).__name__}: {e}")


def main():
    print("=" * 50)
    print("🛠️  Генерация README.md запущена")
    print("=" * 50)

    args = parse_arguments()
    input_base = Path(args.input_folder)
    output_readme = Path(args.output_readme)
    template_path = check_input_paths(input_base, Path(args.template_path))
    if not template_path:
        return

    expected_combinations = ["ubuntu-latest-gcc", "ubuntu-latest-clang", "windows-latest-msvc"]
    results = process_artifacts(input_base, expected_combinations)

    if should_abort_due_to_failed_ci(results):
        print("[WARNING] Некоторые CI не выполнили тесты, но README будет сгенерирован с доступными данными.")
        return

    compatibility, system_info, benchmark = generate_sections(results)
    build_readme(template_path, output_readme, compatibility, system_info, benchmark)


if __name__ == "__main__":
    main()
