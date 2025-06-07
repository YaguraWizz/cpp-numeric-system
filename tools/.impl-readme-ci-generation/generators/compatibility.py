

def generate_compatibility_table(results: dict) -> str:
    """Генерирует Markdown таблицу совместимости."""
    os_compilers = {
        "ubuntu-latest": {"gcc": False, "clang": False, "msvc": False},
        "windows-latest": {"gcc": False, "clang": False, "msvc": False},
    }
    
    # Заполнение статусов на основе фактических результатов
    for key, data in results.items():
        if "ubuntu" in key:
            os_name = "ubuntu-latest"
        elif "windows" in key:
            os_name = "windows-latest"
        else:
            continue

        if "gcc" in key:
            compiler = "gcc"
        elif "clang" in key:
            compiler = "clang"
        elif "msvc" in key:
            compiler = "msvc"
        else:
            continue
        
        if data["test_results"]["passed"]:
            os_compilers[os_name][compiler] = True

    table = "| ОС/Компилятор | GCC   | Clang | MSVC  |\n"
    table += "|---------------|-------|-------|-------|\n"

    # Обратите внимание, что в таблице мы ожидаем только поддерживаемые сочетания
    # Исключаем ubuntu-msvc, windows-gcc, windows-clang
    
    table += f"| Ubuntu        | {'✅' if os_compilers['ubuntu-latest']['gcc'] else '❌'}    | {'✅' if os_compilers['ubuntu-latest']['clang'] else '❌'}     | N/A   |\n"
    table += f"| Windows       | N/A   | N/A   | {'✅' if os_compilers['windows-latest']['msvc'] else '❌'}     |\n"

    return table


    # Форматируем кеши в компактный вид: L1D:32KB(2sh); L1I:32KB(2sh); ...
    if not caches:
        return "—"
    
    parts = []
    for c in caches:
        level = c.get("level", "?")
        ctype = c.get("type", "?")
        # Сократим тип: Data -> D, Instruction -> I, Unified -> U
        ctype_short = {"Data": "D", "Instruction": "I", "Unified": "U"}.get(ctype, ctype)
        
        size_bytes = c.get("size", 0)
        size_kb = size_bytes // 1024 if isinstance(size_bytes, int) else size_bytes
        
        sharing = c.get("num_sharing", "?")
        parts.append(f"L{level}{ctype_short}:{size_kb}KB({sharing}sh)")
    return "; ".join(parts)

