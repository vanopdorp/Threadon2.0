import debugger
import nodes
from parser import ExpressionTransformer
from debugger import DebugError
from os import listdir
def compile_expr_from_tokens(tokens):
    result = ""
    last_type = None
    for token in tokens:
        if last_type == "IDENTIFIER" and token["type"] == "IDENTIFIER":
            result += " "
        result += token["value"]
        last_type = token["type"]
    return result

class CodeGen:
    def __init__(self):
        self.expr_transformer = ExpressionTransformer()
        self.threads = []
        self.main_code = ""
    type_map = {
        "int":"int",
        "float":"float",
        "string":"std::string",
        "bool":"bool",
        "long":"long",
        "long":"long long",
        "BigInt":"BigInt",
        "Task":"Task",
        "AtomicInt":"AtomicInt",
        "AtomicString":"AtomicString",
        "AtomicFloat":"AtomicFloat",

    }
    variable_map = {} # bijv. a = int and b = float
    def compile_expr(self, node):
        # Gebruik de klassen direct, niet via node.<Class>
        from nodes import NumberNode, IdentifierNode, StringNode, BinaryOpNode, FunctionCallNode
        if isinstance(node, NumberNode):
            return str(node.value)
        elif isinstance(node, IdentifierNode):
            # Vervang None/True/False door C++ equivalenten
            if node.name == "None":
                return "nullptr"
            elif node.name == "True":
                return "true"
            elif node.name == "False":
                return "false"
            return node.name
        elif isinstance(node, StringNode):
            val = node.value
            if val.startswith('"') and val.endswith('"'):
                val = val[1:-1]
            return f'"{val}"'

        elif isinstance(node, BinaryOpNode):
            return f"({self.compile_expr(node.left)} {node.operator} {self.compile_expr(node.right)})"
        elif isinstance(node, FunctionCallNode):
            # Correct: just use node.arguments as a flat list
            args = ", ".join(self.compile_expr(arg) for arg in node.arguments)
            return f"{node.name}({args})"
        else:
            return str(node)
    def compile_expr_from_tokens(self, tokens):
        # Voor IF-statements without echte expr-parser (fallback)
        return " ".join(t["value"] for t in tokens)
    def compile_function(self, node):
        to_return = ""
        # Genereer argumentenlijst met default values
        def arg_cpp(t, n, d):
            typ = self.type_map.get(t, t)
            if d is not None:
                # Default value: als string, strip quotes voor C++
                if t == "string":
                    val = d
                    if val.startswith('"') and val.endswith('"'):
                        val = val[1:-1]
                    return f'{typ} {n} = "{val}"'
                else:
                    return f'{typ} {n} = {d}'
            else:
                return f'{typ} {n}'
        arg_str = ", ".join(arg_cpp(t, n, d) for t, n, d in node.arguments)
        if node.name == "main":
            to_return =  f"int {node.name}({arg_str}) {{thread_starter();\n"
        else:
            to_return +=  f"{self.type_map.get(node.return_type, node.return_type)} {node.name}({arg_str}) {{"
        for child in node.body:
            if child.type == "RETURN":
                return_expr = self.compile_expr(child.value)
                to_return += f"    return {return_expr};\n"
            else:
                to_return += self.build_one(child) + '\n'
        if node.name == "main":
            self.main_code += to_return + "}"
            return ""
        elif node.return_type == "Task":
            to_return += "co_return;}"
            return to_return
        else:
            to_return += '}\n'
            return to_return
    def compile_var_decl(self,node):
        rhs = self.compile_expr(node.value)
        return f"{self.type_map[node.var_type]} {node.identifier} = {rhs};"
    def compile_class(self,node):
        to_return = f"class {node.name}"
        if len(node.bases) > 0:
            to_return += " : "
            for base in node.bases:
                to_return += f"public {base},"
            to_return = to_return[:-1] # remove the last comma
        to_return += "{\npublic:\n"
        print(to_return)
        for child in node.body:
            to_return += self.build_one(child) + '\n'
        to_return += '};\n'
        return to_return
    def compile_class_instatation(self,node):
        if len(node.arguments) != 0:
            arguments_as_string= "\n".join("," + arg for arg in node.arguments)
            return f"{node.class_type} {node.identifier}({arguments_as_string});"
        else:
            return f"{node.class_type} {node.identifier};"
    def build_one(self, node):
        if node is None:
            return ""
        if node.type == "FUNCTION":
            return self.compile_function(node)
        elif node.type == "VARIABLE_DECLARATION":
            return self.compile_var_decl(node)
        elif node.type == "RETURN":
            return "return " + self.compile_expr(node.value) + ";"
        elif node.type == "FUNCTION_CALL":
            return self.compile_expr(node) + ";"
        elif node.type == "IF_STATEMENT":
            # (optioneel: betere ondersteuning voor nested IF)
            expr_node = self.expr_transformer.parse_expression(node.expression)
            cond_expr = self.compile_expr(expr_node)

            body_code = "\n".join("    " + self.build_one(stmt) for stmt in node.body)
            return f"if ({cond_expr}) {{\n{body_code}\n}}"
        elif node.type == "BINARY_OP" and node.operator in ("+=", "-=", "*=", "/=", "%="):
            return f"{self.compile_expr(node.left)} {node.operator} {self.compile_expr(node.right)};"
        elif node.type == "ELIF_STATEMENT":
            expr_node = self.expr_transformer.parse_expression(node.expression)
            cond_expr = self.compile_expr(expr_node)

            body_code = "\n".join("    " + self.build_one(stmt) for stmt in node.body)
            return f"else if ({cond_expr}) {{\n{body_code}\n}}"
        elif node.type == "ELSE_STATEMENT":
            body_code = "\n".join("    " + self.build_one(stmt) for stmt in node.body)
            return f"else {{\n{body_code}\n}}"
        elif node.type == "FOR_LOOP":
            iterable_expr = self.expr_transformer.parse_expression(node.iterable)
            iterable_code = compile_expr_from_tokens(node.iterable)
            body_code = "\n".join("    " + self.build_one(stmt) for stmt in node.body)
            
            iter_type = "auto"  # Of bijvoorbeeld node.var_type als dat beter is
            iter_var = node.iterator.name
            
            # For loop met iterator expliciet, waarbij de waarde wordt toegekend aan iter_var
            return (f"for ({node.var_type} {iter_var}  : {iterable_code}) {{\n"
                    f"{body_code}\n}}")
        elif node.type == "WHILE":
            expr_node = self.expr_transformer.parse_expression(node.expression)
            cond_expr = self.compile_expr(expr_node)

            body_code = "\n".join("    " + self.build_one(stmt) for stmt in node.body)
            return f"while ({cond_expr}) {{\n{body_code}\n}}"
        elif node.type == "THREADING_FUNCTION":
            body_code = "\n".join("    " + self.build_one(stmt) for stmt in node.body)
            self.threads.append(node.name)
            return f"void {node.name}() {{{body_code}\n}}"
        elif node.type == "VARIABLE_ASSIGNMENT":
            rhs = self.compile_expr(node.value)
            return f"{node.identifier} = {rhs};"
        elif node.type == "CLASS":
            return self.compile_class(node)
        elif node.type == "CLASS_INSTANTIATION":
            return self.compile_class_instatation(node)
        DebugError(f"Warning: Unknown node type {node.type}")
        return "// [Unknown node]\n"

    def build(self, nodes):
        lib_path = "lib"
        # Verzamel import nodes
        import_files = [n["filename"] for n in nodes if isinstance(n, dict) and n.get("type") == "IMPORT"]
        begin_code = f"""
#include "{lib_path}/sttlib/sttlib.hpp"
using namespace std;
"""
        # Verzamel globale variabelen (alleen VARIABLE_DECLARATION nodes op toplevel)
        global_vars = []
        other_nodes = []
        for node in nodes:
            if hasattr(node, 'type') and node.type == "VARIABLE_DECLARATION":
                global_vars.append(self.compile_var_decl(node))
            else:
                other_nodes.append(node)
        global_code = "\n".join(global_vars) + ("\n" if global_vars else "")
        # Genereer de rest van de code (functies, threads, etc)
        extra_code = ""
        for node in other_nodes:
            # Sla dicts (zoals import nodes) over
            if isinstance(node, dict):
                continue
            bd = self.build_one(node)
            if bd != None:
                extra_code += bd
        thread_code = "void thread_starter() {\n"
        for thread in self.threads:
            thread_code += f"std::thread t_{thread}({thread});\nt_{thread}.detach();\n"
        thread_code += "}\n"
        self.code = begin_code + global_code + extra_code + thread_code + self.main_code + "\n"
        linker_line = f'g++ output.cpp {lib_path}/sttlib/sttlib.cpp -Ofast -std=c++20 -I{lib_path}'
        # Voeg een build instructie toe voor g++ met alle .o files
        if import_files:
            print(import_files)
            for import_file in import_files:
                self.code = f"#include \"{lib_path}/{import_file}/{import_file}.hpp\"\n" + self.code
                linker_line +=" "+ open(f"{lib_path}/{import_file}/flags.txt").read().replace("\n","")+" "
        linker_line += ' -o output\n'
        return [self.code,linker_line]

def print_classes(classes, indent=0):
    for cls in classes:
        print("  " * indent + str(cls))
        if hasattr(cls, 'body') and isinstance(cls.body, list):
            print_classes(cls.body, indent + 1)