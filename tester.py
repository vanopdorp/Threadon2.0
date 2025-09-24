import lexer
from parser import Parser,print_classes
code = """
a::b("hello")
test(0)
"""
lines = code.splitlines()
tokens = lexer.lexer(lines=lines)
print(tokens)
parser = Parser()
ast = parser.parse(tokens)
print_classes(ast)
print(ast)