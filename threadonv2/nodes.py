class ClassNode:
    def __init__(self, name, bases):
        self.type = "CLASS"
        self.name = name
        self.bases = bases
        self.body = []
    def __str__(self):
        return f"ClassNode(name={self.name}, bases={self.bases})"
class IFNode:
    def __init__(self,expression):
        self.expression = expression
        self.type = "IF_STATEMENT"
        self.value = ''
        self.body = []
    def __str__(self):
        # Gebruik super().__str__() om de string van de bovenliggende klasse (ASTNode) te krijgen
        return f"IFNode(expression={self.expression})"
class ELIFNode:
    def __init__(self,expression):
        self.expression = expression
        self.type = "ELIF_STATEMENT"
        self.value = ''
        self.body = []
    def __str__(self):
        # Gebruik super().__str__() om de string van de bovenliggende klasse (ASTNode) te krijgen
        return f"ELIFNode(expression={self.expression})"
class ELSENode:
    def __init__(self):
        self.type = "ELSE_STATEMENT"
        self.value = ''
        self.body = []
    def __str__(self):
        # Gebruik super().__str__() om de string van de bovenliggende klasse (ASTNode) te krijgen
        return f"ELSENode()"
class VariableDeclarationNode:
    def __init__(self, var_type, identifier, value):
        self.type = "VARIABLE_DECLARATION"
        self.var_type = var_type
        self.identifier = identifier
        self.value = value
    def __str__(self):
        return f"VariableDeclaration(type={self.var_type}, name={self.identifier}, value={self.value})"
class VariableAssignmentNode:
    def __init__(self, identifier, value):
        self.type = "VARIABLE_ASSIGNMENT"
        self.identifier = identifier
        self.value = value
    def __str__(self):
        return f"VariableAssignment(name={self.identifier}, value={self.value})"
class ReturnNode:
    def __init__(self,value):
        self.type = "RETURN"
        self.value = value
    def __str__(self):
        return f"ReturnNode(value={self.value})"
class FUNCTIONNode:
    def __init__(self, type, name, arguments=None):
        self.type = "FUNCTION"
        self.return_type = type
        self.name = name
        # arguments: lijst van (type, name, default) tuples; default is None als niet opgegeven
        self.arguments = arguments or []
        self.body = []
    def __str__(self):
        return f"FUNCTIONNode(type={self.type},name={self.name},args={self.arguments})"
class THREADFUNCTIONNode:
    def __init__(self, name):
        self.type = "THREADING_FUNCTION"
        self.name = name
        self.body = []
    def __str__(self):
        return f"THREADFUNCTIONNode(name={self.name})"
class FunctionCallNode:
    def __init__(self, name, arguments):
        self.type = "FUNCTION_CALL"
        self.name = name
        self.arguments = arguments  # lijst van argument tokens

    def __str__(self):
        return f"FunctionCall(name={self.name}, args={self.arguments})"
class NumberNode:
    def __init__(self, value):
        self.type = "NUMBER"
        self.value = value
    def __str__(self):
        return f"Number({self.value})"

class IdentifierNode:
    def __init__(self, name):
        self.type = "IDENTIFIER"
        self.name = name
    def __str__(self):
        return f"Identifier({self.name})"
class StringNode:
    def __init__(self, value):
        self.type = "STRING"
        self.value = value
    def __str__(self):
        return f"String({self.value})"

class BinaryOpNode:
    def __init__(self, left, operator, right):
        self.type = "BINARY_OP"
        self.left = left
        self.operator = operator
        self.right = right
    def __str__(self):
        return f"({self.left} {self.operator} {self.right})"
class ForLoopNode:
    def __init__(self,var_type,iterator,iterable):
        self.type = "FOR_LOOP"
        # voor de standaard C++ stijl:
        # voor range-gebaseerde stijl:
        self.var_type = var_type
        self.iterator = iterator    # bijvoorbeeld IdentifierNode
        self.iterable = iterable    # expressie node
        self.body = []

    def __str__(self):
        if self.iterator is not None:
            return f"ForLoopRange(iterator={self.iterator}, iterable={self.iterable})"
        else:
            return f"ForLoop(init={self.init}, cond={self.condition}, inc={self.increment})"
class WHILENode:
    def __init__(self,expression):
        self.expression = expression
        self.type = "WHILE"
        self.value = ''
        self.body = []
    def __str__(self):
        # Gebruik super().__str__() om de string van de bovenliggende klasse (ASTNode) te krijgen
        return f"WHILENode(expression={self.expression})"
class ClassInstantiationNode:
    def __init__(self, class_type, identifier, arguments):
        self.type = "CLASS_INSTANTIATION"
        self.class_type = class_type
        self.identifier = identifier
        self.arguments = arguments
    def __str__(self):
        return f"ClassInstantiationNode(type={self.class_type}, name={self.identifier}, arguments={self.arguments})"