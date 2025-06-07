import sys
import os
import argparse
from pathlib import Path
from ci_parser import parse_junit_xml, parse_benchmark_json
from generators.compatibility import generate_compatibility_table
from generators.system_info import generate_benchmark_system_info_table
from generators.benchmark_table import generate_benchmark_table
from example_processor import insert_examples

# Абсолютный путь к корню проекта (предположим, что скрипт лежит в tools/.impl-readme-ci-generation/)
script_dir = os.path.dirname(os.path.abspath(__file__))
project_root = os.path.abspath(os.path.join(script_dir, '..', '..'))

if project_root not in sys.path:
    sys.path.insert(0, project_root)


def main():
    print("=" * 50)
    print("🛠️  Генерация README.md запущена")
    print("=" * 50)

    parser = argparse.ArgumentParser(description="Генерация README.md на основе результатов CI.")
    parser.add_argument("--input-folder", required=True, help="Путь к корневому каталогу артефактов (например, 'downloaded_artifacts').")
    parser.add_argument("--output-readme", default="README.md", help="Путь для сохранения сгенерированного README.md.")
    parser.add_argument("--template-path", default="README.md.template", help="Путь к файлу шаблона README.md.")
    args = parser.parse_args()

    input_base_path = Path(args.input_folder)
    output_readme_path = Path(args.output_readme)
    template_path = Path(args.template_path)

    if not input_base_path.exists():
        print(f"[ERROR] Каталог артефактов '{input_base_path}' не найден.")
        return

    print("[INFO] Начинаю сбор данных из каталогов артефактов... ", end="", flush=True)
    all_results = {}

    expected_combinations = [
        "ubuntu-latest-gcc",
        "ubuntu-latest-clang",
        "windows-latest-msvc"
    ]

    for combo_dir_name in expected_combinations:
        print(f"\n  Обрабатываю конфигурацию: {combo_dir_name} ... ", end="", flush=True)
        combo_path = input_base_path / combo_dir_name

        test_xml_path = combo_path / "rtest-junit.xml"
        benchmark_json_path = combo_path / "rbenchmark.json"

        test_results = {"total": 0, "failures": 0, "errors": 0, "skipped": 0, "passed": False}
        benchmark_data = {"benchmarks": [], "system_info": {}}

        if combo_path.is_dir():
            if test_xml_path.is_file():
                print(f"парсю тесты... ", end="", flush=True)
                test_results = parse_junit_xml(test_xml_path)
                print("ok", end="", flush=True)
            else:
                print(f"тесты отсутствуют", end="", flush=True)

            print(", ", end="", flush=True)

            if benchmark_json_path.is_file():
                print(f"парсю бенчмарки... ", end="", flush=True)
                benchmark_data = parse_benchmark_json(benchmark_json_path)
                print("ok", end="", flush=True)
            else:
                print(f"бенчмарки отсутствуют", end="", flush=True)
        else:
            print(f"каталог отсутствует", end="", flush=True)

        all_results[combo_dir_name] = {
            "test_results": test_results,
            "benchmark_results": benchmark_data.get("benchmarks", []),
            "system_info": benchmark_data.get("system_info", {})
        }
    print("\n[INFO] Сбор данных завершён. ok")

    # Проверка, что хотя бы в одном из результатов есть данные о тестах или бенчмарках
    has_valid_data = any(
        (res["test_results"]["total"] > 0 or len(res["benchmark_results"]) > 0)
        for res in all_results.values()
    )
    if not has_valid_data:
        print("[WARN] Не найдено ни одного валидного каталога артефактов. README.md будет сгенерирован с заглушками.")

    print("[INFO] Генерирую таблицу совместимости... ", end="", flush=True)
    compatibility_table_md = generate_compatibility_table(all_results)
    print("ok")

    print("[INFO] Генерирую информацию о системе для бенчмарков... ", end="", flush=True)
    system_info_dict = {
        key: val["system_info"]
        for key, val in all_results.items()
        if val.get("system_info")
    }
    if system_info_dict:
        system_info_md = generate_benchmark_system_info_table(system_info_dict)
        print("ok")
    else:
        system_info_md = "Информация о системе для бенчмарков недоступна.\n"
        print("отсутствует")

    print("[INFO] Генерирую таблицу бенчмарков... ", end="", flush=True)
    benchmark_table_md = generate_benchmark_table(all_results)
    print("ok")

    try:
        print(f"[INFO] Читаю шаблон README из {template_path} ... ", end="", flush=True)
        with open(template_path, 'r', encoding='utf-8') as f:
            readme_template = f.read()
        print("ok")

        print("[INFO] Заменяю стандартные макросы в шаблоне... ", end="", flush=True)
        readme_content = readme_template.replace("{{COMPATIBILITY_TABLE}}", compatibility_table_md)
        readme_content = readme_content.replace("{{SYSTEM_INFO}}", system_info_md)
        readme_content = readme_content.replace("{{BENCHMARK_TABLE}}", benchmark_table_md)
        print("ok")

        # Путь к примерам относительно корня проекта, чтобы избежать проблем
        examples_dir = Path(project_root) / "tests" / "examples"
        print(f"[INFO] Вставляю примеры из {examples_dir} ... ", end="", flush=True)
        readme_content = insert_examples(readme_content, examples_dir)
        print("ok")

        print(f"[INFO] Записываю итоговый README в {output_readme_path} ... ", end="", flush=True)
        with open(output_readme_path, 'w', encoding='utf-8') as f:
            f.write(readme_content)
        print("ok")

        print("\n[УСПЕХ] README.md сгенерирован:")
        print(f" → {output_readme_path.resolve()}")

    except FileNotFoundError:
        print(f"\n[ERROR] Файл шаблона README.md.template не найден по пути: {template_path}")
    except Exception as e:
        print("\n[ОШИБКА] Не удалось сгенерировать README.md")
        print(f"[ИСКЛЮЧЕНИЕ] {type(e).__name__}: {e}")


if __name__ == "__main__":
    main()
