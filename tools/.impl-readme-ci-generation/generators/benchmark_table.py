def generate_benchmark_table(results: dict) -> str:
    """Генерирует Markdown таблицу с результатами бенчмарков в миллисекундах, выделяя лучшее время."""

    all_benchmark_names = set()
    for data in results.values():
        for bm in data.get("benchmark_results", []):
            if bm.get("run_type") == "aggregate":
                continue
            all_benchmark_names.add(bm["name"].split('/')[0])

    if not all_benchmark_names:
        return "Данные бенчмарков недоступны.\n"

    sorted_benchmark_names = sorted(all_benchmark_names)

    # Заголовок с указанием измерения в миллисекундах (ms)
    header = "| Операция        "
    separator = "|-----------------"
    
    platform_cols = []
    platform_order = []
    if "ubuntu-latest-gcc" in results:
        platform_order.append("ubuntu-latest-gcc")
    if "ubuntu-latest-clang" in results:
        platform_order.append("ubuntu-latest-clang")
    if "windows-latest-msvc" in results:
        platform_order.append("windows-latest-msvc")

    display_names = {
        "ubuntu-latest-gcc": "Ubuntu (GCC)",
        "ubuntu-latest-clang": "Ubuntu (Clang)",
        "windows-latest-msvc": "Windows (MSVC)",
    }

    for platform_key in platform_order:
        header += f"| {display_names.get(platform_key, platform_key):<18}, ms "
        separator += f"|{'-'*23}-"
        platform_cols.append(platform_key)

    header += "|\n"
    separator += "|\n"

    table_rows = []
    for bm_name_prefix in sorted_benchmark_names:
        row = f"| {bm_name_prefix:<15} "
        times_list = []

        # Считаем среднее время для каждой платформы, конвертируем из нс в мс
        for platform_key in platform_cols:
            platform_data = results.get(platform_key, {}).get("benchmark_results", [])
            times = [
                bm["real_time"]
                for bm in platform_data
                if bm.get("run_type") != "aggregate" and bm["name"].startswith(bm_name_prefix)
            ]
            avg_time_ns = sum(times) / len(times) if times else None
            avg_time_ms = avg_time_ns / 1_000_000 if avg_time_ns is not None else None
            times_list.append(avg_time_ms)

        # Находим минимальное время среди платформ (если есть)
        filtered_times = [t for t in times_list if t is not None]
        min_time = min(filtered_times) if filtered_times else None

        # Формируем строку с выделением минимального времени
        for t in times_list:
            if t is None:
                cell = "N/A"
            elif min_time is not None and abs(t - min_time) < 1e-6:
                cell = f"**{t:.2f}**"
            else:
                cell = f"{t:.2f}"
            row += f"| {cell:<23} "
        row += "|\n"
        table_rows.append(row)

    return header + separator + "".join(table_rows)
