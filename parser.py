import nodes
from debugger import DebugError
import sys
def remove_indentation_block(token_line):
    i = 0
    while i < len(token_line) and token_line[i]["type"] == "INDENT":
        i += 1
    return token_line[i:]



def print_classes(classes, indent=0):
    for cls in classes:
        print("  " * indent + str(cls))
        if hasattr(cls, 'body') and isinstance(cls.body, list):
            print_classes(cls.body, indent + 1)


class ExpressionTransformer:
    def __init__(self):
        # Operator precedence: higher number = higher precedence
        self.operators = {
            '==': 0, '!=': 0, '<': 0, '>': 0, '<=': 0, '>=': 0,  # comparison
            '+': 1, '-': 1,
            '*': 2, '/': 2, '%': 2,
        }
    def split_arguments(self, tokens):
        args = []
        current = []
        depth = 0
        for token in tokens:
            if token["type"] == "LPAREN":
                depth += 1
            elif token["type"] == "RPAREN":
                depth -= 1
            elif token["value"] == ',' and depth == 0:
                args.append(current)
                current = []
                continue
            current.append(token)
        if current:
            args.append(current)
        return args

    def parse_expression(self, tokens):
        output = []
        operators = []

        def pop_operator():
            op_token = operators.pop()
            right = output.pop()
            left = output.pop()
            output.append(nodes.BinaryOpNode(left, op_token["value"], right))

        i = 0
        while i < len(tokens):
            token = tokens[i]
            ttype = token["type"]
            tval = token["value"]

            if ttype == "NUMBER":
                output.append(nodes.NumberNode(tval))
            elif ttype == "STRING_LITERAL":
                output.append(nodes.StringNode(tval))

            elif ttype == "IDENTIFIER":
                # Handle function call: IDENTIFIER LPAREN ... RPAREN
                if i + 2 < len(tokens) and tokens[i+1]["type"] == "LPAREN":
                    j = i + 2
                    arg_tokens = []
                    depth = 1
                    while j < len(tokens):
                        if tokens[j]["type"] == "LPAREN":
                            depth += 1
                        elif tokens[j]["type"] == "RPAREN":
                            depth -= 1
                            if depth == 0:
                                break
                        arg_tokens.append(tokens[j])
                        j += 1
                    # Fix: split arguments at top level, parse each as expression
                    args_split = self.split_arguments(arg_tokens)
                    arguments = []
                    for arg in args_split:
                        # Only skip empty arg if it's truly empty (not [IdentifierNode])
                        if arg is not None and len(arg) > 0:
                            expr = self.parse_expression(arg)
                            arguments.append(expr)
                    output.append(nodes.FunctionCallNode(tval, arguments))
                    i = j  # Skip to after RPAREN
                else:
                    output.append(nodes.IdentifierNode(tval))
            elif tval in self.operators:
                while (operators and operators[-1]["value"] in self.operators and
                       self.operators[operators[-1]["value"]] >= self.operators[tval]):
                    pop_operator()
                operators.append(token)
            else:
                # Ignore other tokens for now (e.g. parenthesis inside args handled above)
                pass
            i += 1

        while operators:
            pop_operator()

        if output:
            return output[0]
        return None

class Parser:
    def parse_import(self, line):
        line = remove_indentation_block(line)
        if len(line) < 2:
            DebugError("Error: Invalid import statement")
            return None
        if line[0]["type"] != "IMPORT":
            DebugError("Error: Import statement must start with 'import'")
            return None
        # import "libname.o" of import test.o
        # Ondersteun zowel STRING_LITERAL als IDENTIFIER IDENTIFIER (zoals test o)
        if line[1]["type"] == "STRING_LITERAL":
            filename = line[1]["value"].strip('"')
            return {"type": "IMPORT", "filename": filename}
        elif line[1]["type"] == "IDENTIFIER":
            # Ondersteun import test.o (zonder quotes)
            # Combineer alle IDENTIFIER tokens tot een bestandsnaam
            parts = [line[1]["value"]]
            if len(line) > 2 and line[2]["type"] == "IDENTIFIER":
                parts.append(line[2]["value"])
            filename = '.'.join(parts)
            return {"type": "IMPORT", "filename": filename}
        else:
            DebugError("Error: Import statement must specify a .o file as string literal or as identifiers")
            return None
    def __init__(self):
        self.expr_transformer = ExpressionTransformer()

    def parse_variable_declaration(self, line):
        line = remove_indentation_block(line)
        if len(line) < 4:
            DebugError("Error: Invalid variable declaration")
            return None
        if line[0]["type"] != "TYPE":
            DebugError("Error: Variable declaration must start with a type")
            return None
        if line[1]["type"] != "IDENTIFIER":
            DebugError("Error: Variable declaration must have a variable name")
            return None
        if line[2]["type"] != "VARIABLE ASSIGNMENT":
            DebugError("Error: Variable declaration must use '='")
            return None
        var_type = line[0]["value"]
        identifier = line[1]["value"]
        # Parse the right-hand side as an expression (could be function call, literal, etc)
        value_expr = self.expr_transformer.parse_expression(line[3:])
        return nodes.VariableDeclarationNode(var_type, identifier, value_expr)
    def parse_function_call(self, line):
        line = remove_indentation_block(line)
        if len(line) < 3:
            DebugError("Error: Invalid function call")
            return None
        if line[0]["type"] != "IDENTIFIER":
            DebugError("Error: Function call must start with identifier")
            return None
        if line[1]["type"] != "LPAREN" or line[-1]["type"] != "RPAREN":
            DebugError("Error: Missing parentheses in function call")
            return None

        name = line[0]["value"]
        args_tokens = line[2:-1]

        # Allow empty argument list
        if not args_tokens:
            arguments = []
        else:
            args_split = self.expr_transformer.split_arguments(args_tokens)
            arguments = [self.expr_transformer.parse_expression(arg) for arg in args_split if arg]

        return nodes.FunctionCallNode(name, arguments)



    def parse_compound_assignment(self, line):
        line = remove_indentation_block(line)
        # Herken zowel i += 1 als i + = 1
        if len(line) < 3:
            return None
        if line[0]["type"] != "IDENTIFIER":
            return None
        # Vorm 1: i += 1
        if line[1]["type"] in ("PLUS_ASSIGN", "MINUS_ASSIGN", "MULTIPLY_ASSIGN", "DIVIDE_ASSIGN", "MODULO_ASSIGN"):
            target = nodes.dentifierNode(line[0]["value"])
            op = line[1]["value"][:-1]  # "+=" -> "+"
            expr = self.expr_transformer.parse_expression(line[2:])
            return nodes.BinaryOpNode(target, op + "=", expr)
        # Vorm 2: i + = 1, i - = 1, etc.
        if (
            len(line) >= 4 and
            line[1]["type"] in ("PLUS", "MINUS", "MULTIPLY", "DIVIDE", "MODULO") and
            line[2]["type"] == "VARIABLE ASSIGNMENT"
        ):
            op_map = {
                "PLUS": "+",
                "MINUS": "-",
                "MULTIPLY": "*",
                "DIVIDE": "/",
                "MODULO": "%",
            }
            op = op_map[line[1]["type"]]
            target = nodes.IdentifierNode(line[0]["value"])
            expr = self.expr_transformer.parse_expression(line[3:])
            return nodes.BinaryOpNode(target, op + "=", expr)
        return None
    def parse_class_instantiation(self, line):
        # Example: Myclass test("hello")
        # Should parse as: type=Myclass, identifier=test, args=("hello")
        line = remove_indentation_block(line)
        if len(line) < 3:
            return None
        if line[0]["type"] == "IDENTIFIER" and line[1]["type"] == "IDENTIFIER" and line[2]["type"] == "LPAREN":
            class_type = line[0]["value"]
            identifier = line[1]["value"]
            # Find RPAREN
            rparen_idx = None
            for idx in range(2, len(line)):
                if line[idx]["type"] == "RPAREN":
                    rparen_idx = idx
                    break
            if rparen_idx is None:
                return None
            # Handle empty argument list
            if rparen_idx == 3:
                arguments = []
            else:
                arg_tokens = line[3:rparen_idx]
                args_split = self.expr_transformer.split_arguments(arg_tokens)
                arguments = [self.expr_transformer.parse_expression(arg) for arg in args_split if arg]
            # Store constructor arguments in value (could be a tuple or list)
            return nodes.ClassInstantiationNode(class_type, identifier, arguments)
        return None
    def parse_function(self, line):
        line = remove_indentation_block(line)
        if len(line) < 4:
            DebugError("Error: Invalid function definition")
            return None
        if line[0]["type"] != "FUNCTION":
            DebugError("Error: Function definition must start with 'def'")
            return None
        # Check for optional return type
        if line[1]["type"] == "TYPE":
            return_type = line[1]["value"]
            name_idx = 2
        else:
            return_type = "void"  # default type, or 'void' if you prefer
            name_idx = 1
        if line[name_idx]["type"] != "IDENTIFIER":
            DebugError("Error: Function must have a name")
            return None
        # Find LPAREN and RPAREN
        lparen_idx = name_idx + 1
        if line[lparen_idx]["type"] != "LPAREN":
            DebugError("Error: Function definition syntax is invalid (missing left parenthesis)")
            return None
        # Find bijbehorende RPAREN
        # Zoek van lparen_idx+1 tot einde naar de eerste RPAREN
        rparen_idx = None
        for idx in range(lparen_idx+1, len(line)):
            if line[idx]["type"] == "RPAREN":
                rparen_idx = idx
                break
        if rparen_idx is None or line[-1]["type"] != "COLON":
            DebugError("Error: Function definition syntax is invalid (missing right parenthesis or colon)")
            return None
        # Parse argumenten tussen lparen_idx+1 en rparen_idx
        arg_tokens = line[lparen_idx+1:rparen_idx]
        # Argumenten zijn van de vorm: TYPE IDENTIFIER (optioneel komma's ertussen)
        arguments = []
        i = 0
        while i < len(arg_tokens):
            if arg_tokens[i]["type"] == "TYPE":
                arg_type = arg_tokens[i]["value"]
                if i+1 < len(arg_tokens) and arg_tokens[i+1]["type"] == "IDENTIFIER":
                    arg_name = arg_tokens[i+1]["value"]
                    # Check for default value: TYPE IDENTIFIER = VALUE
                    if i+2 < len(arg_tokens) and arg_tokens[i+2]["type"] == "VARIABLE ASSIGNMENT":
                        # Default value aanwezig
                        if i+3 < len(arg_tokens):
                            default_token = arg_tokens[i+3]
                            # default_token mag NUMBER, STRING_LITERAL, IDENTIFIER zijn
                            if default_token["type"] in ("NUMBER", "STRING_LITERAL", "IDENTIFIER"):
                                arguments.append((arg_type, arg_name, default_token["value"]))
                                i += 4
                                # Skip comma if present
                                if i < len(arg_tokens) and arg_tokens[i]["value"] == ',':
                                    i += 1
                            else:
                                DebugError("Error: Unsupported default value type in argument: {default_token}")
                                return None
                        else:
                            DebugError("Error: Expected value after '=' in argument list")
                            return None
                    else:
                        arguments.append((arg_type, arg_name, None))
                        i += 2
                        if i < len(arg_tokens) and arg_tokens[i]["value"] == ',':
                            i += 1
                else:
                    DebugError("Error: Argument missing name after type")
                    return None
            elif arg_tokens[i]["value"] == ',':
                i += 1
            else:
                DebugError(f"Error: Unexpected token in argument list: {arg_tokens[i]}")
                return None
        name = line[name_idx]["value"]
        return nodes.FUNCTIONNode(return_type, name, arguments)
    def parse_threading_function(self, line):
        line = remove_indentation_block(line)
        if len(line) < 4:
            DebugError("Error: Invalid function definition")
            return None
        if line[0]["type"] != "THREADING_FUNCTION":
            DebugError("Error: Function definition must start with 'thread'")
            return None
        if line[1]["type"] != "IDENTIFIER":
            DebugError("Error: Function must have a name")
            return None
        if line[2]["type"] != "LPAREN":
            DebugError("Error: Function definition syntax is invalid (missing left parenthesis)")
            return None
        if line[3]["type"] != "RPAREN":
            DebugError("Error: Function definition syntax is invalid (missing right parenthesis or have function arguments)")
            return None
        if line[4]["type"] != "COLON":
            DebugError("Error: Function definition syntax is invalid (missing colon)")
            return None
        name = line[1]["value"]
        return nodes.THREADFUNCTIONNode(name)

    def parse_if(self,line):
        line = remove_indentation_block(line)
        if line[0]["type"] != "IF_STATEMENT":
            DebugError("Error cannot parse if statement: It does not starts with if")
        if line[-1:][0]["type"] != "COLON":
            DebugError("Error cannot parse if statement does not end with :")
        expression = line[1:-1]
        return nodes.IFNode(expression)

    def parse_elif(self,line):
        line = remove_indentation_block(line)
        if line[0]["type"] != "ELIF_STATEMENT":
            DebugError("Error cannot parse elif statement: It does not starts with elif")
        if line[-1:][0]["type"] != "COLON":
            DebugError("Error cannot parse elif statement does not end with :")
        expression = line[1:-1]
        return nodes.ELIFNode(expression)
    def parse_else(self,line):
        line = remove_indentation_block(line)
        if line[0]["type"] != "ELSE_STATEMENT":
            DebugError("Error cannot parse elif statement: It does not starts with elif")
        if line[1]["type"] != "COLON":
            DebugError("Error cannot parse elif statement does not end with :")
        return nodes.ELSENode()
    def parse_while(self,line):
        line = remove_indentation_block(line)
        if line[0]["type"] != "WHILE":
            DebugError("Error cannot parse while statement: It does not starts with if")
        if line[-1:][0]["type"] != "COLON":
            DebugError("Error cannot parse while statement does not end with :")
        expression = line[1:-1]
        return nodes.WHILENode(expression)
    def parse_return(self, line):
        line = remove_indentation_block(line)
        if line[0]["type"] != "RETURN":
            DebugError("Error: not a return statement")
            return None

        expr_tokens = line[1:]
        expr = self.expr_transformer.parse_expression(expr_tokens)
        return nodes.ReturnNode(expr)
    def parse_for(self, line):
        line = remove_indentation_block(line)
        if line[0]["type"] != "FOR":
            DebugError("Error: For loop must start with 'for'")
            return None
        if line[1]["type"] != "TYPE":
            DebugError("Error: For loop must have a type before iterator")
        if line[2]["type"] != "IDENTIFIER":
            DebugError("Error: For loop must have an iterator variable")
            return None
        if line[3]["type"] != "IN":
            DebugError("Error: For loop must have 'in' after iterator")
            return None
        if line[-1]["type"] != "COLON":
            DebugError("Error: For loop must end with ':'")
            return None

        iterator = nodes.IdentifierNode(line[2]["value"])
        return nodes.ForLoopNode(var_type=line[1]["value"],iterator=iterator,iterable=line[4:-1])
    def parse_class(self, line):
        line = remove_indentation_block(line)
        if line[0]["type"] != "CLASS":
            DebugError("Error: Class definition must start with 'class'")
            return None
        if line[1]["type"] != "IDENTIFIER":
            DebugError("Error: Class must have a name")
            return None
        name = line[1]["value"]
        # Check for optional inheritance
        bases = []
        if len(line) > 4 and line[2]["type"] == "LPAREN":
            # Zoek bijbehorende RPAREN
            rparen_idx = None
            for idx in range(3, len(line)):
                if line[idx]["type"] == "RPAREN":
                    rparen_idx = idx
                    break
            if rparen_idx is None or line[rparen_idx+1]["type"] != "COLON":
                DebugError("Error: Class definition syntax is invalid (missing right parenthesis or colon)")
                return None
            # Parse base classes tussen LPAREN en RPAREN
            base_tokens = line[3:rparen_idx]
            i = 0
            while i < len(base_tokens):
                if base_tokens[i]["type"] == "IDENTIFIER":
                    bases.append(base_tokens[i]["value"])
                    i += 1
                    if i < len(base_tokens) and base_tokens[i]["value"] == ',':
                        i += 1
                elif base_tokens[i]["value"] == ',':
                    i += 1
                else:
                    DebugError("Error: Unexpected token in base class list")
                    return None
        elif line[2]["type"] == "COLON":
            # Geen basisklassen, maar wel een colon
            pass
        else:
            DebugError("Error: Class definition syntax is invalid (missing colon)")
            return None

        return nodes.ClassNode(name, bases)
    def parse(self, token_lines):
        ast = []
        stack = []
        function_names = set()
        errors = []
        builtin_functions = {"print"}
        # Voor globale scope: alle functies verzamelen
        for line in token_lines:
            if not line:
                continue
            content = remove_indentation_block(line)
            # Import statement
            if content and content[0]["type"] == "IMPORT":
                node = self.parse_import(content)
                if node:
                    ast.append(node)
                continue
            if content and content[0]["type"] == "FUNCTION":
                # Functienaam ophalen
                if content[1]["type"] == "TYPE":
                    name_idx = 2
                else:
                    name_idx = 1
                fname = content[name_idx]["value"]
                if fname in function_names:
                    errors.append(f"Error: Duplicate function name '{fname}'")
                function_names.add(fname)
        function_names |= builtin_functions

        # Helper om per scope variabelen bij te houden
        def check_expr_vars(node, local_vars, known_funcs):
            # Voeg altijd global_vars toe aan de zichtbare variabelen
            visible_vars = set(local_vars) | set(global_vars)
            if isinstance(node, nodes.IdentifierNode):
                # Sta True en False altijd toe als geldige identifiers
                if node.name in ("True", "False"):
                    return
                if node.name not in visible_vars and node.name not in known_funcs:
                    errors.append(f"Error: Unknown variable or function '{node.name}' in expression")
            elif isinstance(node, nodes.FunctionCallNode):
                for arg in node.arguments:
                    check_expr_vars(arg, local_vars, known_funcs)
            elif isinstance(node, nodes.BinaryOpNode):
                check_expr_vars(node.left, local_vars, known_funcs)
                check_expr_vars(node.right, local_vars, known_funcs)

        # Stack van (indent_level, local_vars) voor scopes
        scope_stack = []
        global_vars = set()
        current_vars = set()
        for line in token_lines:
            if not line:
                continue

            indent_level = sum(1 for token in line if token["type"] == "INDENT")
            content = remove_indentation_block(line)

            # Scope management
            while scope_stack and scope_stack[-1][0] >= indent_level:
                scope_stack.pop()
                if scope_stack:
                    current_vars = set(scope_stack[-1][1])
                else:
                    current_vars = set(global_vars)

            # Verzamel globale variabelen in global_vars
            if content and content[0]["type"] == "TYPE":
                node = self.parse_variable_declaration(content)
                if node:
                    if indent_level == 0:
                        global_vars.add(node.identifier)
                        current_vars = set(global_vars)
                        ast.append(node)
                    else:
                        current_vars.add(node.identifier)
                        # Update scope_stack with new local var
                        if scope_stack:
                            scope_stack[-1] = (scope_stack[-1][0], set(scope_stack[-1][1]) | {node.identifier})
                        while len(stack) > indent_level:
                            stack.pop()
                        if not stack:
                            DebugError("Error: Cannot add node to body, stack is empty (indentation error or previous parse error)")
                            continue
                        stack[-1].body.append(node)
                continue

            if content and content[0]["type"] == "THREADING_FUNCTION":
                node = self.parse_threading_function(content)
                if node is None:
                    errors.append("Error: Could not parse function definition")
                    continue
                # Nieuwe scope: argumenten zijn lokale variabelen
                local_vars = set()
                scope_stack.append((indent_level, local_vars))
                current_vars = set(local_vars) | set(global_vars)
                node.body = []
                stack = [node]
                ast.append(node)
                continue
            elif content and content[0]["type"] == "FUNCTION":
                node = self.parse_function(content)
                if node is None:
                    errors.append("Error: Could not parse function definition")
                    continue
                # Nieuwe scope: argumenten zijn lokale variabelen
                local_vars = set(arg[1] for arg in node.arguments)
                scope_stack.append((indent_level, local_vars))
                current_vars = set(local_vars) | set(global_vars)
                node.body = []
                # If inside a class, add to class body, else to top-level AST
                if stack and hasattr(stack[-1], 'type') and stack[-1].type == "CLASS":
                    stack[-1].body.append(node)
                    stack.append(node)
                else:
                    stack = [node]
                    ast.append(node)
                continue
            # ...existing code...
            elif content and content[0]["type"] == "IF_STATEMENT":
                node = self.parse_if(content)
                expr_node = self.expr_transformer.parse_expression(node.expression)
                check_expr_vars(expr_node, current_vars, function_names)
                if indent_level == 0:
                    ast.append(node)
                    stack = [node]
                else:
                    while len(stack) > indent_level:
                        stack.pop()
                    if not stack:
                        DebugError("Error: Cannot add node to body, stack is empty (indentation error or previous parse error)")
                        continue
                    stack[-1].body.append(node)
                    stack.append(node)
            elif (len(content) >= 4 and content[0]["type"] == "IDENTIFIER" and content[1]["type"] == "IDENTIFIER" and content[2]["type"] == "LPAREN"):
                # Try class instantiation first
                node = self.parse_class_instantiation(content)
                if node:
                    if indent_level == 0:
                        ast.append(node)
                    else:
                        while len(stack) > indent_level:
                            stack.pop()
                        if not stack:
                            DebugError("Error: Cannot add node to body, stack is empty (indentation error or previous parse error)")
                            continue
                        stack[-1].body.append(node)
                else:
                    # Fallback to function call
                    node = self.parse_function_call(content)
                    if node:
                        for arg in node.arguments:
                            check_expr_vars(arg, current_vars, function_names)
                        if indent_level == 0:
                            ast.append(node)
                        else:
                            while len(stack) > indent_level:
                                stack.pop()
                            if not stack:
                                DebugError("Error: Cannot add node to body, stack is empty (indentation error or previous parse error)")
                                continue
                            stack[-1].body.append(node)
            elif content and content[1]["type"] == "LPAREN":
                node = self.parse_function_call(content)
                if node:
                    for arg in node.arguments:
                        check_expr_vars(arg, current_vars, function_names)
                    if indent_level == 0:
                        ast.append(node)
                    else:
                        while len(stack) > indent_level:
                            stack.pop()
                        if not stack:
                            DebugError("Error: Cannot add node to body, stack is empty (indentation error or previous parse error)")
                            continue
                        stack[-1].body.append(node)
            elif content and content[0]["type"] == "ELSE_STATEMENT":
                node = self.parse_else(content)
                if indent_level == 0:
                    ast.append(node)
                    stack = [node]
                else:
                    while len(stack) > indent_level:
                        stack.pop()
                    stack[-1].body.append(node)
                    stack.append(node)

            elif content and content[0]["type"] == "FOR":
                node = self.parse_for(content)
                if node:
                    local_vars = set()
                    if hasattr(node, 'iterator') and node.iterator is not None:
                        local_vars.add(node.iterator.name)
                    scope_stack.append((indent_level, local_vars))
                    current_vars = set(local_vars)
                    if indent_level == 0:
                        ast.append(node)
                        stack = [node]
                    else:
                        while len(stack) > indent_level:
                            stack.pop()
                        stack[-1].body.append(node)
                        stack.append(node)
            elif content and content[0]["type"] == "WHILE":

                node = self.parse_while(content)
                expr_node = self.expr_transformer.parse_expression(node.expression)
                check_expr_vars(expr_node, current_vars, function_names)
                if indent_level == 0:
                    ast.append(node)
                    stack = [node]
                else:
                    while len(stack) > indent_level:
                        stack.pop()
                    if not stack:
                        DebugError("Error: Cannot add node to body, stack is empty (indentation error or previous parse error)")
                        continue
                    stack[-1].body.append(node)
                    stack.append(node)
            elif content and content[0]["type"] == "IDENTIFIER" and (
                content[1]["type"] in ("PLUS_ASSIGN", "MINUS_ASSIGN", "MULTIPLY_ASSIGN", "DIVIDE_ASSIGN", "MODULO_ASSIGN") or
                (len(content) > 2 and content[1]["type"] in ("PLUS", "MINUS", "MULTIPLY", "DIVIDE", "MODULO") and content[2]["type"] == "VARIABLE ASSIGNMENT")
            ):
                node = self.parse_compound_assignment(content)
                if indent_level == 0:
                    ast.append(node)
                else:
                    while len(stack) > indent_level:
                        stack.pop()
                    if not stack:
                        DebugError("Error: Cannot add node to body, stack is empty (indentation error or previous parse error)")
                        continue
                    stack[-1].body.append(node)
            elif content and content[0]["type"] == "IDENTIFIER" and content[1]["type"] == "LPAREN":
                # Try function call first
                node = self.parse_function_call(content)
                if node:
                    for arg in node.arguments:
                        check_expr_vars(arg, current_vars, function_names)
                    if indent_level == 0:
                        ast.append(node)
                    else:
                        while len(stack) > indent_level:
                            stack.pop()
                        if not stack:
                            DebugError("Error: Cannot add node to body, stack is empty (indentation error or previous parse error)")
                            continue
                        stack[-1].body.append(node)
                else:
                    # Try class instantiation: Myclass test("hello")
                    node = self.parse_class_instantiation(content)
                    if node:
                        if indent_level == 0:
                            ast.append(node)
                        else:
                            while len(stack) > indent_level:
                                stack.pop()
                            if not stack:
                                DebugError("Error: Cannot add node to body, stack is empty (indentation error or previous parse error)")
                                continue
                            stack[-1].body.append(node)
            elif content and content[1]["type"] == "VARIABLE ASSIGNMENT":
                node = nodes.VariableAssignmentNode(content[0]["value"], self.expr_transformer.parse_expression(content[2:]))
                print("DEBUGGER: ",node)
                if indent_level == 0:
                    ast.append(node)
                else:
                    while len(stack) > indent_level:
                        stack.pop()
                    if not stack:
                        DebugError("Error: Cannot add node to body, stack is empty (indentation error or previous parse error)")
                        continue
                    stack[-1].body.append(node)
            elif content and content[0]["type"] == "CLASS":
                print("PARSING CLASS")
                node = self.parse_class(content)
                if indent_level == 0:
                    ast.append(node)
                    stack = [node]
                else:
                    while len(stack) > indent_level:
                        stack.pop()
                    stack[-1].body.append(node)
                    stack.append(node)
        if errors:
            for err in errors:
                DebugError(err)
            sys.exit(1)
        return ast
