#include <stdio.h>
#include <string.h>
#include "ast.h"
#include "y.tab.h"

ASTNode* create_identifier(const char* name) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_IDENTIFIER;
    node->data.identifier.name = strdup(name);
    return node;
}

ASTNode* create_constant(Value* value) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_CONSTANT;
    node->data.constant.value = value;
    return node;
}

ASTNode* create_array_access(ASTNode* array, ASTNode* index) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_ARRAY_ACCESS;
    node->data.array_access.array = array;
    node->data.array_access.index = index;
    return node;
}

ASTNode* create_function_call(ASTNode* function, ASTNode* args) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_FUNCTION_CALL;
    node->data.function_call.function = function;
    node->data.function_call.args = args;
    return node;
}


ASTNode* create_un_op(ASTNode* operand, un_operator operator) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_UNARY_OP;
    node->data.unary.operator = operator;
    node->data.unary.operand = operand;
    return node;
}

ASTNode* create_list(ASTNode* arg, ASTNode* next,NodeType type) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = type;
    node->data.arg_list.arg = arg;
    node->data.arg_list.next = next;
    return node;
}

full_type_t * create_type(type_t t, int n_pointers){
    full_type_t * full_type = malloc(sizeof(full_type_t));
    full_type->type = t;
    full_type->n_pointers = n_pointers;
    return full_type;
}

ASTNode* create_bin_op(ASTNode* left, ASTNode* right,bin_operator operator) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_BINARY_OP;
    node->data.binary.operator = operator;
    node->data.binary.left = left;
    node->data.binary.right = right;
    return node;
}

ASTNode* create_block(ASTNode* block){
    ASTNode * node = malloc(sizeof (ASTNode));
    node->type = NODE_BLOCK;
    node->data.compound_stmt.block = block;
    return node;
}

ASTNode* create_assignment(ASTNode* left, ASTNode* right, assign_operator operator) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_ASSIGNMENT;
    node->data.binary.operator = operator;
    node->data.binary.left = left;
    node->data.binary.right = right;
    return node;
}


ASTNode* create_declaration(ASTNode* name,full_type_t * full_type, ASTNode* value) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_DECLARATION;
    node->data.declaration.name = name;
    node->data.declaration.type = full_type;
    node->data.declaration.value = value;
    return node;
}

ASTNode* create_array_declaration(ASTNode* name, ASTNode* size){
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_ARRAY_DECLARATION;
    node->data.array_declaration.name = name;
    node->data.array_declaration.size = size;
    return node;

}

ASTNode* create_param_declaration(full_type_t * full_type, ASTNode* name){
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_PARAM_DECLARATION;
    node->data.param_declaration.type = full_type;
    node->data.param_declaration.name = name;
    return node;

}


ASTNode* create_if_stmt(ASTNode* condition, ASTNode* if_body, ASTNode* else_body) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_IF;
    node->data.if_stmt.condition = condition;
    node->data.if_stmt.if_body = if_body;
    node->data.if_stmt.else_body = else_body;
    return node;
}

ASTNode* create_while_loop(ASTNode* condition, ASTNode* body) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_WHILE;
    node->data.while_loop.condition = condition;
    node->data.while_loop.body = body;
    return node;
}

ASTNode* create_do_while_loop(ASTNode* condition, ASTNode* body) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_DO_WHILE;
    node->data.while_loop.condition = condition;
    node->data.while_loop.body = body;
    return node;
}

ASTNode* create_for_loop(ASTNode* init, ASTNode* condition, ASTNode* effect, ASTNode* body) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_FOR;
    node->data.for_loop.init = init;
    node->data.for_loop.condition = condition;
    node->data.for_loop.effect = effect;
    node->data.for_loop.body = body;
    return node;
}

ASTNode* create_jump(NodeType type) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = type;
    return node;
}

ASTNode* create_return_stmt(ASTNode* value) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_RETURN;
    node->data.return_stmt.value = value;
    return node;
}


ASTNode* create_function_def(full_type_t* full_type, char* name, ASTNode* parameters, ASTNode* body){
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_FUNCTION_DEF;
    node->data.function_def.type = full_type;
    node->data.function_def.name = name;
    node->data.function_def.parameters = parameters;
    node->data.function_def.body = body;
    return node;
}

// Helper function to print indentation
static void print_indent(int indent) {
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
}
void print_type(full_type_t full_type, int indent){
    print_indent(indent);
    switch (full_type.type) {
        case NODE_INT:   printf("int"); break;
        case NODE_FLOAT: printf("float"); break;
        case NODE_VOID:  printf("void"); break;
        case NODE_CHAR:  printf("char");break;
    }
    printf(" (pointers: %d)\n", full_type.n_pointers);
}
// Recursive function to print the AST
void print_ast(ASTNode* node, int indent) {
    if (node == NULL) {
        print_indent(indent);
        printf("(NULL)\n");
        return;
    }

    switch (node->type) {
        case NODE_IDENTIFIER:
            print_indent(indent);
            printf("Identifier: %s\n", node->data.identifier.name);
            break;

        case NODE_CONSTANT:
            print_indent(indent);
            Value* v = node->data.constant.value;
            if (v->type.n_pointers > 0){
                printf("Constant (string): %s\n", (char*)v->value.ptr);
            }else{
                switch (v->type.type) {
                    case NODE_INT:
                        printf("Constant (int): %d\n", v->value.i);
                        break;
                    case NODE_FLOAT:
                        printf("Constant (float): %f\n", v->value.f);
                        break;
                    case NODE_VOID:
                        break;
                    case NODE_CHAR:
                        printf("Constant (char): %c\n",v->value.c);
                        break;
                }
            }
            break;
        case NODE_ARRAY_ACCESS:
            print_indent(indent);
            printf("Array Access:\n");
            print_indent(indent + 1);
            printf("Array:\n");
            print_ast(node->data.array_access.array, indent + 2);
            print_indent(indent + 1);
            printf("Index:\n");
            print_ast(node->data.array_access.index, indent + 2);
            break;
        
        case NODE_FUNCTION_CALL:
            print_indent(indent);
            printf("Function Call:\n");
            print_indent(indent + 1);
            printf("Function:\n");
            print_ast(node->data.function_call.function, indent + 2);
            print_indent(indent + 1);
            printf("Arguments:\n");
            print_ast(node->data.function_call.args, indent + 2);
            break;
        
        case NODE_UNARY_OP:
            print_indent(indent);
            printf("Unary Operation:\n");
            print_indent(indent + 1);
            printf("Operator: ");
            switch (node->data.unary.operator) {
                case NODE_ADDRESS: printf("ADDRESS\n"); break;
                case NODE_DEREF: printf("DEREFERENCE\n"); break;
                case NODE_PLUS: printf("PLUS\n"); break;
                case NODE_MINUS: printf("MINUS\n"); break;
                case NODE_INC_POST: printf("INCREMENT POSTFIX\n"); break;
                case NODE_DEC_POST: printf("DECREMENT POSTFIX\n"); break;
                case NODE_INC_PRE: printf("INCREMENT PREFIX\n"); break;
                case NODE_DEC_PRE: printf("DECREMENT PREFIX\n"); break;
                case NODE_BITWISE_NOT: printf("BITWISE NOT\n"); break;
                case NODE_LOGICAL_NOT: printf("LOGICAL NOT\n"); break;

                // Add other unary operators as needed
                default: printf("UNKNOWN\n"); break;
            }
            print_indent(indent + 1);
            printf("Operand:\n");
            print_ast(node->data.unary.operand, indent + 2);
            break;
        case NODE_BLOCK:
            print_indent(indent);
            printf("Block:\n");
            print_ast(node->data.compound_stmt.block,indent+1);
            break;
        case NODE_PARAM_LIST:
        case NODE_DECLARATOR_LIST:
        case NODE_INIT_LIST:
        case NODE_ARG_LIST:
        case NODE_TOP_LEVEL_LIST:
        case NODE_INSTRUCTION_LIST:
            print_indent(indent);
            printf("List:\n");
            print_ast(node->data.arg_list.arg, indent + 1);
            print_ast(node->data.arg_list.next, indent);
            break;


        case NODE_BINARY_OP:
            print_indent(indent);
            printf("Binary Operation:\n");
            print_indent(indent + 1);
            printf("Operator: ");
            switch (node->data.binary.operator) {
                case NODE_ADD: printf("ADD\n"); break;
                case NODE_SUB: printf("SUBTRACT\n"); break;
                case NODE_MUL: printf("MULTIPLY\n"); break;
                case NODE_DIV: printf("DIVIDE\n"); break;
                case NODE_MOD: printf("MODULO\n"); break;
                case NODE_SHIFT_RIGHT: printf("SHIFT RIGHT\n"); break;
                case NODE_SHIFT_LEFT: printf("SHIFT LEFT\n"); break;
                case NODE_BITWISE_AND: printf("BITWISE AND\n"); break;
                case NODE_BITWISE_OR: printf("BITWISE OR\n"); break;
                case NODE_BITWISE_XOR: printf("BITWISE XOR\n"); break;
                case NODE_LOGICAL_AND: printf("LOGICAL AND\n"); break;
                case NODE_LOGICAL_OR: printf("LOGICAL OR\n"); break;
                case NODE_IS_EQ: printf("IS EQUAL\n"); break;
                case NODE_NOT_EQ: printf("NOT EQUAL\n"); break;
                case NODE_GT: printf("GREATER THAN\n"); break;
                case NODE_GEQ: printf("GREATER OR EQUAL THAN\n");break;
                case NODE_LEQ:printf("LESS OR EQUAL THAN\n");break;
                case NODE_LT:printf("LESS THAN\n");break;
                    // Add other binary operators as needed
                default: printf("UNKNOWN\n"); break;
            }
            print_indent(indent + 1);
            printf("Left:\n");
            print_ast(node->data.binary.left, indent + 2);
            print_indent(indent + 1);
            printf("Right:\n");
            print_ast(node->data.binary.right, indent + 2);
            break;
        
        case NODE_ASSIGNMENT:
            print_indent(indent);
            printf("Assignment:\n");
            print_indent(indent + 1);
            printf("Operator: ");
            switch (node->data.binary.operator) {
                case NODE_ASSIGN: printf("ASSIGN\n"); break;
                case NODE_MUL_ASSIGN: printf("MULTIPLY ASSIGN\n"); break;
                case NODE_DIV_ASSIGN: printf("DIVIDE ASSIGN\n"); break;
                case NODE_MOD_ASSIGN: printf("MODULO ASSIGN\n"); break;
                case NODE_ADD_ASSIGN: printf("ADD ASSIGN\n"); break;
                case NODE_SUB_ASSIGN: printf("SUBTRACT ASSIGN\n"); break;
                case NODE_SHIFT_LEFT_ASSIGN: printf("SHIFT LEFT ASSIGN\n"); break;
                case NODE_SHIFT_RIGHT_ASSIGN: printf("SHIFT RIGHT ASSIGN\n"); break;
                case NODE_AND_ASSIGN: printf("AND ASSIGN\n"); break;
                case NODE_OR_ASSIGN: printf("OR ASSIGN\n"); break;
                case NODE_XOR_ASSIGN: printf("XOR ASSIGN\n"); break;
                default: printf("UNKNOWN\n"); break;
            }
            print_indent(indent + 1);
            printf("Left:\n");
            print_ast(node->data.binary.left, indent + 2);
            print_indent(indent + 1);
            printf("Right:\n");
            print_ast(node->data.binary.right, indent + 2);
            break;

        case NODE_DECLARATION:
            print_indent(indent);
            printf("Declaration:\n");
            print_indent(indent + 1);
            printf("Name:\n");
            print_ast(node->data.declaration.name, indent + 2);
            print_indent(indent + 1);
            printf("Type:\n");
            print_type(*(node->data.declaration.type), indent + 2);
            if (node->data.declaration.value) {
                print_indent(indent + 1);
                printf("Value:\n");
                print_ast(node->data.declaration.value, indent + 2);
            }
            break;

        case NODE_ARRAY_DECLARATION:
            print_indent(indent);
            printf("Array Declaration:\n");
            print_indent(indent + 1);
            printf("Name:\n");
            print_ast(node->data.array_declaration.name, indent + 2);
            print_indent(indent + 1);
            printf("Size:\n");
            print_ast(node->data.array_declaration.size, indent + 2);
            break;


        case NODE_PARAM_DECLARATION:
            print_indent(indent);
            printf("Parameter Declaration:\n");
            print_indent(indent + 1);
            printf("Type:\n");
            print_type(*(node->data.param_declaration.type), indent + 2);
            print_indent(indent + 1);
            printf("Name:\n");
            print_ast(node->data.param_declaration.name, indent + 2);
            break;


        case NODE_IF:
            print_indent(indent);
            printf("If Statement:\n");
            print_indent(indent + 1);
            printf("Condition:\n");
            print_ast(node->data.if_stmt.condition, indent + 2);
            print_indent(indent + 1);
            printf("If Body:\n");
            print_ast(node->data.if_stmt.if_body, indent + 2);
            if (node->data.if_stmt.else_body) {
                print_indent(indent + 1);
                printf("Else Body:\n");
                print_ast(node->data.if_stmt.else_body, indent + 2);
            }
            break;
        case NODE_DO_WHILE:
            print_indent(indent);
            printf("Do ");
        case NODE_WHILE:
            print_indent(indent);
            printf("While Loop:\n");
            print_indent(indent + 1);
            printf("Condition:\n");
            print_ast(node->data.while_loop.condition, indent + 2);
            print_indent(indent + 1);
            printf("Body:\n");
            print_ast(node->data.while_loop.body, indent + 2);
            break;
        case NODE_FOR:
            print_indent(indent);
            printf("For Loop:\n");
            print_indent(indent + 1);
            printf("Init:\n");
            print_ast(node->data.for_loop.init, indent + 2);
            print_indent(indent + 1);
            printf("Condition:\n");
            print_ast(node->data.for_loop.condition, indent + 2);
            print_indent(indent + 1);
            printf("Effect:\n");
            print_ast(node->data.for_loop.effect, indent + 2);
            print_indent(indent + 1);
            printf("Body:\n");
            print_ast(node->data.for_loop.body, indent + 2);
            break;
        case NODE_CONTINUE:
            print_indent(indent);
            printf("Continue\n");
            break;
        
        case NODE_BREAK:
            print_indent(indent);
            printf("Break\n");
            break;
        
        case NODE_RETURN:
            print_indent(indent);
            printf("Return Statement:\n");
            if (node->data.return_stmt.value) {
                print_indent(indent + 1);
                printf("Value:\n");
                print_ast(node->data.return_stmt.value, indent + 2);
            }
            break;

        case NODE_FUNCTION_DEF:
            print_indent(indent);
            printf("Function Definition:\n");
            print_indent(indent + 1);
            printf("Name: %s\n", node->data.function_def.name);
            print_indent(indent + 1);
            printf("Return Type:\n");
            print_type(*(node->data.function_def.type), indent + 2);
            if (node->data.function_def.parameters) {
                print_indent(indent + 1);
                printf("Parameters:\n");
                print_ast(node->data.function_def.parameters, indent + 2);
            }
            print_indent(indent + 1);
            printf("Body:\n");
            print_ast(node->data.function_def.body, indent + 2);
            break;

        default:
            print_indent(indent);
            printf("Unhandled Node Type: %d\n", node->type);
    }
}

void print_node_type(NodeType node_type){
    switch (node_type){
        case NODE_IDENTIFIER:
            printf("Identifier\n");
            break;
        case NODE_CONSTANT:
            printf("Constant\n");
            break;
        case NODE_ARRAY_ACCESS:
            printf("Array Access\n");
            break;
        case NODE_FUNCTION_CALL:
            printf("Function Call\n");
            break;
        case NODE_UNARY_OP:
            printf("Unary Operation\n");
            break;
        case NODE_PARAM_LIST:
            printf("Param List\n");
            break;
        case NODE_DECLARATOR_LIST:
            printf("Declarator List\n");
            break;
        case NODE_BLOCK:
            printf("Declaration List\n");
            break;
        case NODE_INIT_LIST:
            printf("Init List\n");
            break;
        case NODE_ARG_LIST:
            printf("Arg List\n");
            break;
        case NODE_TOP_LEVEL_LIST:
            printf("Top Level List\n");
            break;
        case NODE_BINARY_OP:
            printf("Binary Operation\n");
            break;
        case NODE_ASSIGNMENT:
            printf("Assignment\n");
            break;
        case NODE_DECLARATION:
            printf("Declaration\n");
            break;
        case NODE_ARRAY_DECLARATION:
            printf("Array Declaration\n");
            break;
        case NODE_PARAM_DECLARATION:
            printf("Param Declaration\n");
            break;
        case NODE_IF:
            printf("If\n");
            break;
        case NODE_WHILE:
            printf("While\n");
            break;
        case NODE_DO_WHILE:
            printf("Do While\n");
            break;
        case NODE_FOR:
            printf("For\n");
            break;
        case NODE_CONTINUE:
            printf("Continue\n");
            break;
        case NODE_BREAK:
            printf("Break\n");
            break;
        case NODE_RETURN:
            printf("Return\n");
            break;
        case NODE_FUNCTION_DEF:
            printf("Function Definition\n");
            break;
        case NODE_INSTRUCTION_LIST:
            printf("Instruction list\n");
            break;
    }
}

void print_value(Value v){
    printf("Value :\n  is_const=%d\n",v.is_constant);
    print_type(v.type,1);
    print_indent(1);
    if(v.type.n_pointers > 0){
        printf("val=%d\n",v.value.i);
        return;
    }
    switch (v.type.type) {
        case NODE_INT:printf("val=%d\n",v.value.i);break;
        case NODE_FLOAT:printf("val=%f\n",v.value.f);break;
        case NODE_VOID:printf("val=None\n");break;
        case NODE_CHAR:printf("val='%c'\n",v.value.c);break;
    }
}

void print_val_or_addr(ValueOrAddress v){
    print_value(v.value);
    if (v.has_address){
        printf("  addr=%d\n",v.address);
    }else{
        printf("  value is not addressed\n");
    }
}
int type_size(full_type_t* full_type){
    if (full_type->n_pointers > 0){
        return 4;
    }
    switch (full_type->type) {
        case NODE_INT:
            return 4;
        case NODE_CHAR:
            return 1;
        case NODE_FLOAT:
            return 4;
        case NODE_VOID:
            return 0;
    }
}

int is_void_value(Value* v){
    return v->type.type == NODE_VOID && v->type.n_pointers == 0;
}

int is_zero(ValueOrAddress* v){
    if (v->value.type.n_pointers > 0) {
        fprintf(stderr,"a pointer cannot be zero value, it's a fk pointer");
        exit(1);
    }
    switch (v->value.type.type) {
        case NODE_INT:
            return v->value.value.i == 0;
        case NODE_FLOAT:
            return v->value.value.f == 0.0f;
        case NODE_VOID:
            fprintf(stderr,"Void cannot be zero...");
            exit(1);
        case NODE_CHAR:
            return v->value.value.c == 0;
    }
}

void error_on_wrong_node(NodeType expected, NodeType actual, const char *function_name) {
    if (expected == actual) return;
    fprintf(stderr, "%s called with wrong node ( expected ", function_name);
    print_node_type(expected);
    fprintf(stderr, ", actual ");
    print_node_type(actual);
    fprintf(stderr, "\n");
    exit(1);
}

void error_out(ASTNode* current_node,const char* error_message){
    fprintf(stderr,"[ERROR] %s (l. %d)\n",error_message, current_node->line_number);
    exit(1);
}