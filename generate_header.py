import clang.cindex
import sys

# Functie om de C++ klassen en functies uit een header te extraheren
def get_cpp_classes_and_functions(header_file):
    # Maak een Clang-index
    index = clang.cindex.Index.create()

    # Parse het C++-bestand
    translation_unit = index.parse(header_file)

    # Lijsten om de klassen en functies op te slaan
    classes = []
    functions = []

    # Loop door de Abstract Syntax Tree (AST)
    for cursor in translation_unit.cursor.get_children():
        if cursor.kind == clang.cindex.CursorKind.CLASS_DECL:
            # Als het een klasse is, voeg deze dan toe aan de lijst
            class_info = {
                'name': cursor.spelling,
                'template': get_template_info(cursor),
                'methods': []
            }
            # Loop door de methoden van de klasse
            for method_cursor in cursor.get_children():
                if method_cursor.kind == clang.cindex.CursorKind.CXX_METHOD:
                    method_info = {
                        'name': method_cursor.spelling,
                        'return_type': method_cursor.result_type.spelling,
                        'params': [param.spelling for param in method_cursor.get_children() if param.kind == clang.cindex.CursorKind.PARM_DECL],
                        'template': get_template_info(method_cursor)
                    }
                    class_info['methods'].append(method_info)
            classes.append(class_info)
        
        elif cursor.kind == clang.cindex.CursorKind.FUNCTION_DECL:
            # Als het een losse functie is, voeg deze dan toe aan de lijst
            function_info = {
                'name': cursor.spelling,
                'return_type': cursor.result_type.spelling,
                'params': [param.spelling for param in cursor.get_children() if param.kind == clang.cindex.CursorKind.PARM_DECL],
                'template': get_template_info(cursor)
            }
            functions.append(function_info)

    return classes, functions

# Functie om template-informatie van een cursor te verkrijgen
def get_template_info(cursor):
    # Haal de template parameters op
    template_info = []
    for template_cursor in cursor.get_children():
        if template_cursor.kind == clang.cindex.CursorKind.TEMPLATE_TYPE_PARAMETER:
            template_info.append(template_cursor.spelling)
        elif template_cursor.kind == clang.cindex.CursorKind.TEMPLATE_NON_TYPE_PARAMETER:
            template_info.append(template_cursor.spelling)
        elif template_cursor.kind == clang.cindex.CursorKind.TEMPLATE_TEMPLATE_PARAMETER:
            template_info.append(template_cursor.spelling)
    return template_info

# Functie om de header te genereren op basis van de geëxtraheerde klassen en functies
def generate_header(classes, functions, output_file):
    with open(output_file, 'w') as f:
        f.write("#pragma once\n\n")

        # Genereer de klassen
        for cls in classes:
            f.write(f"class {cls['name']}{format_template(cls['template'])} {{\npublic:\n")
            for method in cls['methods']:
                args = ", ".join([f"{param}" for param in method['params']])
                f.write(f"    {method['return_type']} {method['name']}({args}){format_template(method['template'])};\n")
            f.write("};\n\n")

        # Genereer de losse functies
        for func in functions:
            args = ", ".join([f"{param}" for param in func['params']])
            f.write(f"{func['return_type']} {func['name']}({args}){format_template(func['template'])};\n")

    print(f"Header gegenereerd: {output_file}")

# Functie om templates in de juiste vorm te formatteren
def format_template(template_data):
    if not template_data:
        return ""
    return f"<{', '.join(template_data)}>" if isinstance(template_data, list) else f"<{template_data}>"

# Hoofdprogramma
if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Gebruik: python generate_hpp_with_libclang.py input.cpp output.hpp")
    else:
        # Verkrijg klassen en functies uit het input C++ bestand
        classes, functions = get_cpp_classes_and_functions(sys.argv[1])
        
        # Genereer het output headerbestand
        generate_header(classes, functions, sys.argv[2])
