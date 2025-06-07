from utils import format_cache_entry

def generate_benchmark_system_info_table(system_info_dict: dict) -> str:
    headers = [
        "OS/Compiler", "Ядер CPU", "Частота CPU (МГц)",
        "L1D", "L1I", "L2U", "L3U"
    ]
    header_line = "| " + " | ".join(headers) + " |"
    separator_line = "|---" * len(headers) + "|"

    lines = [header_line, separator_line]
    for os_compiler, info in system_info_dict.items():
        num_cpus = str(info.get("num_cpus", "N/A"))
        mhz = str(info.get("mhz_per_cpu", "N/A"))

        # Собираем кеши в словарь по ключу level+type
        caches = info.get("caches", [])
        cache_map = {}
        for c in caches:
            key = f"L{c.get('level', 'N')}"+c.get('type', 'N')[0].upper()  # L1D, L1I, L2U, L3U
            cache_map[key] = format_cache_entry(c)

        # Берём значения с дефолтом "-"
        l1d = cache_map.get("L1D", "-")
        l1i = cache_map.get("L1I", "-")
        l2u = cache_map.get("L2U", "-")
        l3u = cache_map.get("L3U", "-")

        row = [os_compiler, num_cpus, mhz, l1d, l1i, l2u, l3u]
        line = "| " + " | ".join(row) + " |"
        lines.append(line)
    return "\n".join(lines)


