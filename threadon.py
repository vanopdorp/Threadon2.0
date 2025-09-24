from lexer import lexer
from parser import Parser
from codegenerator import CodeGen
import subprocess

def compile_threadon_to_cpp(source_code, output_cpp_file):
    # Lexing
    token_lines = lexer(source_code.splitlines())
    # Parsing
    parser = Parser()
    ast = parser.parse(token_lines)

    # Code Generation
    codegen = CodeGen()
    builded = codegen.build(ast)
    cpp_code = builded[0]
    linker_command = builded[1]

    # Write generated C++ code to file
    with open(output_cpp_file, "w") as f:
        f.write(cpp_code)
    print("COMPILING WITH: ",linker_command)
    print(f"C++ code generated and written to {output_cpp_file}")
    return linker_command

def compile_with_linker_command(linker_command):
    try:
        compile_command = linker_command.split()
        subprocess.run(compile_command, check=True)
        print("Executable compiled successfully using linker_command.")
    except subprocess.CalledProcessError as e:
        print(f"Compilation failed: {e}")

if __name__ == "__main__":
    source_file = "example.th"
    cpp_file = "output.cpp"

    with open(source_file, "r") as f:
        source_code = f.read()

    linker_command = compile_threadon_to_cpp(source_code, cpp_file)
    compile_with_linker_command(linker_command)
