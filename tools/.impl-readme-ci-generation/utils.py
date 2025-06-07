from datetime import datetime


def format_datetime(dt_str):
    # Парсим ISO и возвращаем в формате ГГГГ-ММ-ДД ЧЧ:ММ:СС
    try:
        dt = datetime.fromisoformat(dt_str)
        return dt.strftime("%Y-%m-%d %H:%M:%S")
    except Exception:
        return dt_str  # если парсинг не удался, возвращаем как есть

def format_cache_entry(cache):
    level = cache.get("level", "N/A")
    ctype = cache.get("type", "N/A")
    size_bytes = cache.get("size", 0)
    size_kb = size_bytes // 1024
    sharing = cache.get("num_sharing", "N/A")

    # Формат: 32KB(2sh)
    return f"{size_kb}KB({sharing}sh)"