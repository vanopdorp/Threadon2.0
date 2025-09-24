import sys
class Debugger:
    @staticmethod
    def analyze(ast, tokens):
        errors = []
        warnings = []
        function_names = set()
        variable_names = set()
        used_variables = set()
        used_functions = set()
        lines_with_errors = set()

        # Helper: Recursief door het AST lopen
        # Verzamel globale variabelen vóór het walken
        global_vars = set()
        for node in ast:
            if getattr(node, 'type', None) == "VARIABLE_DECLARATION":
                global_vars.add(node.identifier)

        def walk(node, scope_vars, scope_funcs, parent_type=None, current_function=None):
            if node is None:
                return
            ntype = getattr(node, 'type', None)
            # Voeg altijd global_vars toe aan scope_vars
            visible_vars = set(scope_vars) | set(global_vars)
            if ntype == "FUNCTION":
                if node.name in scope_funcs:
                    errors.append(f"Dubbele functie: {node.name}")
                scope_funcs.add(node.name)
                local_vars = set(arg[1] for arg in node.arguments)
                if not node.body:
                    warnings.append(f"Functie '{node.name}' heeft een lege body")
                for child in node.body:
                    walk(child, local_vars, scope_funcs, ntype, current_function=node.name)
            elif ntype == "VARIABLE_DECLARATION":
                # Alleen als deze variabele nog niet in de huidige scope_vars zit (dus niet in global_vars)
                if node.identifier in scope_vars:
                    errors.append(f"Dubbele variabele declaratie: {node.identifier}")
                # Voeg alleen toe aan scope_vars, niet aan visible_vars (global_vars is al meegenomen)
                scope_vars.add(node.identifier)
                if node.value is None or node.value == "":
                    warnings.append(f"Variabele '{node.identifier}' heeft geen waarde")
            elif ntype == "IDENTIFIER":
                if node.name not in visible_vars:
                    errors.append(f"Onbekende variabele: {node.name}")
                used_variables.add(node.name)
            elif ntype == "FUNCTION_CALL":
                used_functions.add(node.name)
                # Recursieve functie-aanroep check (direct)
                if current_function is not None and node.name == current_function:
                    warnings.append(f"Recursive call in function: {node.name}")
                for arg in getattr(node, 'arguments', []):
                    walk(arg, scope_vars, scope_funcs, ntype, current_function=current_function)
            elif ntype == "RETURN":
                walk(node.value, scope_vars, scope_funcs, ntype, current_function=current_function)
            elif ntype == "IF_STATEMENT" or ntype == "ELIF_STATEMENT":
                if not hasattr(node, 'body') or not node.body:
                    warnings.append(f"{ntype} without body")
                for child in getattr(node, 'body', []):
                    walk(child, scope_vars, scope_funcs, ntype, current_function=current_function)
            elif ntype == "ELSE_STATEMENT":
                if not hasattr(node, 'body') or not node.body:
                    warnings.append(f"else without body")
                for child in getattr(node, 'body', []):
                    walk(child, scope_vars, scope_funcs, ntype, current_function=current_function)
            elif ntype == "FOR_LOOP":
                if not hasattr(node, 'body') or not node.body:
                    warnings.append(f"for-loop without body")
                local_vars = set(scope_vars)
                if hasattr(node, 'iterator') and node.iterator:
                    local_vars.add(node.iterator.name)
                for child in getattr(node, 'body', []):
                    walk(child, local_vars, scope_funcs, ntype, current_function=current_function)
            elif ntype == "BINARY_OP":
                walk(node.left, scope_vars, scope_funcs, ntype, current_function=current_function)
                walk(node.right, scope_vars, scope_funcs, ntype, current_function=current_function)
            # Add more node types as needed

        # Start walk
        for node in ast:
            walk(node, set(), set(function_names))

        # Token-level checks (syntax errors, etc)
        for lineno, line in enumerate(tokens, 1):
            for token in line:
                if token["type"] == "SYNTAX_ERROR":
                    errors.append(f"Syntax error op regel {lineno}: '{token['value']}'")
                    lines_with_errors.add(lineno)

        # Warnings voor niet-gebruikte variabelen/functies
        unused_vars = variable_names - used_variables
        unused_funcs = function_names - used_functions
        for v in unused_vars:
            warnings.append(f"Niet-gebruikte variabele: {v}")
        for f in unused_funcs:
            if f != "main":
                warnings.append(f"Niet-gebruikte functie: {f}")

        # Print alles overzichtelijk
        output = []
        output.append("\n==== DEBUGGER REPORT ====")
        if errors:
            output.append("FAULTS:")
            for e in errors:
                output.append(f"  - {e}")
            output.append("==========================\n")
            print("\n".join(output))
            sys.exit(1)
        else:
            output.append("No faults found.")
        if warnings:
            output.append("\nWARNINGS:")
            for w in warnings:
                output.append(f"  - {w}")
        else:
            output.append("No warnings.")
        output.append("==========================\n")
        print("\n".join(output))
def DebugError(message):
    print(f"\033[31m{message}\033[39m")
    sys.exit(1)