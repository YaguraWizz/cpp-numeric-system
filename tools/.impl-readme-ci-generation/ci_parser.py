from pathlib import Path
import json
import xml.etree.ElementTree as ET

def parse_junit_xml(xml_path: Path) -> dict:
    """Парсит JUnit XML файл и возвращает сводку результатов."""
    try:
        tree = ET.parse(xml_path)
        root = tree.getroot()

        # JUnit XML может иметь корневой элемент <testsuites> или <testsuite>
        total_tests = 0
        total_failures = 0
        total_errors = 0
        total_skipped = 0

        if root.tag == "testsuites":
            for testsuite in root.findall("testsuite"):
                total_tests += int(testsuite.get("tests", 0))
                total_failures += int(testsuite.get("failures", 0))
                total_errors += int(testsuite.get("errors", 0))
                total_skipped += int(testsuite.get("skipped", 0))
        elif root.tag == "testsuite":
            total_tests = int(root.get("tests", 0))
            total_failures = int(root.get("failures", 0))
            total_errors = int(root.get("errors", 0))
            total_skipped = int(root.get("skipped", 0))
        else:
            raise ValueError(f"Неизвестный корневой тег JUnit XML: {root.tag}")

        passed = (total_failures == 0 and total_errors == 0) and total_tests > 0
        return {
            "total": total_tests,
            "failures": total_failures,
            "errors": total_errors,
            "skipped": total_skipped,
            "passed": passed,
        }
    except (ET.ParseError, ValueError, FileNotFoundError) as e:
        print(f"Ошибка при парсинге {xml_path}: {e}")
        return {"total": 0, "failures": 0, "errors": 0, "skipped": 0, "passed": False}

def parse_benchmark_json(json_path: Path) -> dict:
    """Парсит Google Benchmark JSON файл и возвращает данные бенчмарков и информацию о системе."""
    try:
        with open(json_path, 'r', encoding='utf-8') as f:
            data = json.load(f)

        benchmarks = data.get("benchmarks", [])
        context = data.get("context", {})

        return {
            "benchmarks": benchmarks,
            "system_info": {
                "date": context.get("date"),
                "host_name": context.get("host_name"),
                "num_cpus": context.get("num_cpus"),
                "mhz_per_cpu": context.get("mhz_per_cpu"),
                "caches": context.get("caches", []),
                "library_version": context.get("library_version"),
            }
        }
    except (json.JSONDecodeError, FileNotFoundError) as e:
        print(f"Ошибка при парсинге {json_path}: {e}")
        return {"benchmarks": [], "system_info": {}}