from pathlib import Path
import re

def unwrap_delete_define_macro(text: str) -> str:
    import re

    # 1) Разворачиваем вызовы DELETE_DEFINE_MACRO(...) как раньше
    pattern_macro = r'DELETE_DEFINE_MACRO\s*\(\s*((?:[^()]*|\([^()]*\))*)\s*\)\s*;?'
    text = re.sub(pattern_macro, lambda m: m.group(1), text, flags=re.DOTALL)

    # 2) Удаляем строки, содержащие '#include "gtest/gtest.h"' и комментарий //[DELETE_DEFINE_MACRO]
    pattern_gtest_include = r'^.*#include\s+"gtest/gtest\.h".*//\[DELETE_DEFINE_MACRO\].*\n?'
    text = re.sub(pattern_gtest_include, '', text, flags=re.MULTILINE)

    return text



def unwrap_insert_example_body(text: str) -> str:
    """
    Находит все вхождения INSERT_EXAMPLE_BODY(...),
    корректно учитывая вложенные скобки,
    и заменяет вызов на содержимое между скобками, без самих скобок.
    """
    import re

    pattern = re.compile(r'INSERT_EXAMPLE_BODY\s*\(')

    pos = 0
    result = ''
    while True:
        match = pattern.search(text, pos)
        if not match:
            result += text[pos:]
            break
        
        start = match.end()  # позиция после открывающей скобки '('
        depth = 1
        i = start
        while i < len(text) and depth > 0:
            if text[i] == '(':
                depth += 1
            elif text[i] == ')':
                depth -= 1
            i += 1

        if depth != 0:
            # Ошибка: не нашли закрывающую скобку, вставляем остаток и выходим
            result += text[pos:]
            break

        # Вырезаем содержимое между скобками
        inner_content = text[start:i-1]

        # Добавляем всё до начала вызова + содержимое без скобок
        result += text[pos:match.start()] + inner_content

        pos = i  # продолжаем после закрывающей скобки

    return result



def remove_expect_true(text):
    return re.sub(r'EXPECT_TRUE\s*\([^)]*\)\s*;?', '', text)

def remove_gtest_include(text):
    return re.sub(r'#include\s+"gtest/gtest.h".*\n', '', text)

def remove_define_macros(text):
    # Удаляет все строки, начинающиеся с #define EXAMPLE_BEGIN_MARK, #define DELETE_DEFINE_MACRO и т.д.
    return re.sub(r'^\s*#define\s+.*\n', '', text, flags=re.MULTILINE)

def process_test_blocks_to_main(text):
    def find_matching_brace(s, start_pos):
        depth = 0
        for i in range(start_pos, len(s)):
            if s[i] == '{':
                depth += 1
            elif s[i] == '}':
                depth -= 1
                if depth == 0:
                    return i
        return -1

    pattern = re.compile(r'TEST\s*\([^)]*\)\s*\{', re.MULTILINE)
    match = pattern.search(text)

    if not match:
        return text

    start_body = match.end()
    end_brace_pos = find_matching_brace(text, start_body - 1)

    if end_brace_pos == -1:
        return text

    test_body = text[start_body:end_brace_pos]

    # Удаляем namespace numsystem
    namespace_pattern = r'namespace\s+numsystem\s*\{(?:.|\n)*?\}\s*//\s*namespace\s+numsystem'
    text = re.sub(namespace_pattern, '', text, flags=re.DOTALL)

    # Функция для сдвига строк на 4 пробела
    def indent_lines(s, indent='    '):
        return '\n'.join(indent + line if line.strip() else '' for line in s.splitlines())

    indented_body = indent_lines(test_body)

    main_function_code = f"""int main() {{
{indented_body}
    return 0;
}}"""

    # Удаляем маркеры
    text = re.sub(r'#define EXAMPLE_BEGIN_MARK', '', text)
    text = re.sub(r'#define EXAMPLE_END_MARK', '', text)

    # Возвращаем исходный текст без TEST(...) {...} плюс main()
    # Можно убрать TEST-блок из текста, чтобы не дублировался
    text_without_test = pattern.sub('', text)
    text_without_test = text_without_test[:start_body-5] + text_without_test[end_brace_pos+1:]  # вырезаем тело теста

    return text_without_test.strip() + '\n\n' + main_function_code


def replace_cout(text):
    # Заменяет COUT(...) на std::cout << ...;
    pattern = r'COUT\s*\(\s*(.*?)\s*\)\s*;?'
    return re.sub(pattern, r'std::cout << \1;', text)

def remove_ifdef_block(text: str, macro_name: str) -> str:
    pattern = re.compile(
        rf'#ifdef\s+{macro_name}\b.*?#endif', 
        flags=re.DOTALL
    )
    return pattern.sub('', text)


def preprocess_cpp_code(text: str) -> str:
    text = unwrap_delete_define_macro(text)
    text = unwrap_insert_example_body(text)  
    text = remove_expect_true(text)
    text = remove_gtest_include(text)
    text = remove_ifdef_block(text, 'ENABLE_OUTPUT') 
    text = process_test_blocks_to_main(text)
    text = replace_cout(text)
    text = remove_define_macros(text)
    text = re.sub(r'\n\s*\n', '\n\n', text).strip()
    return text



def insert_examples(template: str, examples_dir: Path) -> str:
    def replacer(match):
        index = match.group(1)
        example_path = examples_dir / f"ex-{index}.cpp"
        if example_path.exists():
            with open(example_path, 'r', encoding='utf-8') as f:
                code = f.read()
            processed_code = preprocess_cpp_code(code)
            # Добавляем markdown блок с языком cpp
            return f"\n```cpp\n{processed_code}\n```\n"
        else:
            return f"// Example ex-{index}.cpp not found"
    return re.sub(r"\{\{EXAMPLE(\d+)\}\}", replacer, template)
