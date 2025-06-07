def apply_template(template_path, output_path, substitutions: dict):
    with open(template_path, 'r', encoding='utf-8') as f:
        content = f.read()

    for key, value in substitutions.items():
        content = content.replace(f"{{{{{key}}}}}", value)

    with open(output_path, 'w', encoding='utf-8') as f:
        f.write(content)
