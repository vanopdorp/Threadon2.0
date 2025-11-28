from lexer import lexer
from parser import Parser
from codegenerator import CodeGen
import subprocess
import argparse
from os import listdir
from debugger import FaultDecoder
print(__file__)
def compile_threadon_to_cpp(source_code,filename, output_cpp_file,arg_list):
    # Lexing
    token_lines = lexer(source_code.splitlines())
    # Parsing
    parser = Parser()
    ast = parser.parse(token_lines)

    # Code Generation
    codegen = CodeGen(output_file=output_cpp_file,arg_list=arg_list)
    builded = codegen.build(ast)
    cpp_code = builded[0]
    linker_command = builded[1]

    # Write generated C++ code to file
    with open("build/output.cpp", "w") as f:
        f.write(cpp_code)
    print("COMPILING WITH: ",linker_command)
    print(f"C++ code generated and written to {output_cpp_file}")
    return linker_command

def compile_with_linker_command(linker_command):
    try:
        result = subprocess.run(
            linker_command.split(),
            check=True,
            capture_output=True,
            text=True
        )
    except subprocess.CalledProcessError as e:

        FaultDecoder.debug(e.stdout)
if __name__ == "__main__":
    parser = argparse.ArgumentParser(
    prog='Threadon',
    description='Compiles a threadon program',
    )
    parser.add_argument('-c', '--compile') # option that takes a value
    parser.add_argument('-o', '--output') # option that takes a value
    parser.add_argument('-n', '--noptimalisations',help='does not add any optimalisation', action='store_true') 
    args = parser.parse_args()
    with open(args.compile, "r") as f:
        source_code = f.read()
    cpp_file = 'output'
    if args.output:
        cpp_file = args.output
    arg_list = []
    if args.noptimalisations:
        arg_list.append('noptimalisation')
    linker_command = compile_threadon_to_cpp(source_code,args.compile, cpp_file,arg_list)
    compile_with_linker_command(linker_command)
