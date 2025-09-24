import sys
import re
from multiprocessing import Pool, cpu_count
import struct

tokens = {
    "FUNCTION": r'^def\b',
    "THREADING_FUNCTION": r'^thread\b',
    "IMPORT": r'^import\b',
    "NUMBER": r'^-?\d+(\.\d+)?$',
    "EXPRESSION_NUMBER": r'^-?(?:\d+|\.\d+|\d+\.\d+)[eE][-+]?\d+$',
    "TYPE": r'^(int|float|double|string|list|dict|bool|void|long|BigInt|Task|AtomicInt|AtomicStrig)$',
    "RETURN":r'^return\b',
    "LPAREN": r'^\($',
    "RPAREN": r'^\)$',
    "STRING_LITERAL": r'^".*"$',
    "COMMA": r'^,$',
    "FOR": r'^for\b',
    "WHILE": r'^while\b',
    "IN": r'^in\b',
    "SEMICOLON": r'^;$',
    "CLASS": r'^class\b',
    
    "EQUALS": r'^==$',
    "NOT_EQUALS": r'^!=$',
    "LESS_THAN": r'^<$',
    "GREATER_THAN": r'^>$',
    "LESS_EQUAL": r'^<=$',
    "GREATER_EQUAL": r'^>=$',

    "IF_STATEMENT": r'^if\b',
    "ELIF_STATEMENT": r'^elif\b',
    "ELSE_STATEMENT": r'^else\b',
    "LIST": r'^\[\s*(.*?)\s*\]$',
    "DICT": r'^\{\s*(.*?)\s*\}$',
    
    # Compound assignment operators
    "PLUS_ASSIGN": r'^\+=$',
    "MINUS_ASSIGN": r'^-=$',
    "MULTIPLY_ASSIGN": r'^\*=$',
    "DIVIDE_ASSIGN": r'^/=$',
    "MODULO_ASSIGN": r'^%=$',
   # Arithmetic operators
    "PLUS": r'^\+$',
    "MINUS": r'^-$',
    "MULTIPLY": r'^\*$',
    "DIVIDE": r'^/$',
    "MODULO": r'^%$',
    
    # Compound assignment operators
    "DOUBLE_COLON":r'^::$',
    "COLON": r'^:$',
    "INDENTATION":"    ",
    "VARIABLE ASSIGNMENT": r'^=$', # DO BEFORE EQUALS
    "IDENTIFIER": r'^[a-zA-Z_]\w*(?:(?:\.|::)[a-zA-Z_]\w*)*$', # DO AS LAST
}
def lexer_line(line):
    if not line.strip():
        return []
    # indentatie bepalen
    indent_match = re.match(r'^( *)', line)
    indent_level = len(indent_match.group(1)) if indent_match else 0
    line_tokens = [{"type": "INDENT", "value": ""}] * (indent_level // 4)

    content = line.lstrip()
    parts = re.findall(
        r'"[^"]*"'
        r'|==|!=|<=|>=|<|>|='
        r'|\+|\-|\*|\/|%|\+=|\-=|\*=|\/=|%='
        r'|\d+\.\d+[eE][-+]?\d+'
        r'|\d+\.\d+|\d+'
        r'|\[.*?\]|\{.*?\}'
        r'|[a-zA-Z_]\w*(?:\.[a-zA-Z_]\w*)*'
        r'|::'
        r'|[:(),]',
        content
    )

    # eerste tokenisatie
    raw_tokens = []
    for part in parts:
        matched = False
        for token_name, pattern in tokens.items():
            if re.match(pattern, part):
                token_info = {"type": token_name, "value": part}
                if token_name == "FUNCTION_CALL":
                    func_name = part.split('(')[0]
                    args = part[part.find('(')+1 : part.rfind(')')]
                    token_info["function_name"] = func_name
                    token_info["arguments"] = args
                elif token_name == "IDENTIFIER":
                    token_info["identifier"] = part
                raw_tokens.append(token_info)
                matched = True
                break
        if not matched:
            raw_tokens.append({"type": "SYNTAX_ERROR", "value": part})

    # merge stap: IDENTIFIER :: IDENTIFIER → één IDENTIFIER
    merged_tokens = []
    i = 0
    while i < len(raw_tokens):
        if (i+2 < len(raw_tokens)
            and raw_tokens[i]["type"] == "IDENTIFIER"
            and raw_tokens[i+1]["type"] == "DOUBLE_COLON"
            and raw_tokens[i+2]["type"] == "IDENTIFIER"):
            merged_value = raw_tokens[i]["value"] + "::" + raw_tokens[i+2]["value"]
            merged_tokens.append({
                "type": "IDENTIFIER",
                "value": merged_value,
                "identifier": merged_value
            })
            i += 3
        else:
            merged_tokens.append(raw_tokens[i])
            i += 1

    return line_tokens + merged_tokens

    if not line.strip():
        return []
    indent_match = re.match(r'^( *)', line)
    indent_level = len(indent_match.group(1)) if indent_match else 0
    line_tokens = [{"type": "INDENT", "value": ""}] * (indent_level // 4)
    content = line.lstrip()
    parts = re.findall(
    r'"[^"]*"'
    r'|==|!=|<=|>=|<|>|='
    r'|\+|\-|\*|\/|%|\+=|\-=|\*=|\/=|%='
    r'|\d+\.\d+[eE][-+]?\d+'
    r'|\d+\.\d+|\d+'
    r'|\[.*?\]|\{.*?\}'
    r'|[a-zA-Z_]\w*(?:\.[a-zA-Z_]\w*)*'
    r'|::'
    r'|[:(),]',
    content
)


    for part in parts:
        matched = False
        for token_name, pattern in tokens.items():
            if re.match(pattern, part):
                token_info = {"type": token_name, "value": part}
                if token_name == "FUNCTION_CALL":
                    func_name = part.split('(')[0]
                    args = part[part.find('(')+1 : part.rfind(')')]
                    token_info["function_name"] = func_name
                    token_info["arguments"] = args
                elif token_name == "IDENTIFIER":
                    token_info["identifier"] = part
                line_tokens.append(token_info)
                matched = True
                break
        if not matched:
            line_tokens.append({"type": "SYNTAX_ERROR", "value": part})
    return line_tokens
def lexer(lines, workers=None):
    with Pool(processes=workers or cpu_count()) as pool:
        return pool.map(lexer_line, lines)