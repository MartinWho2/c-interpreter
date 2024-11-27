#include <stdio.h>
#include <stdbool.h>
#include "ast.h"
#include "y.tab.h"


extern int yyparse(void);
extern FILE *yyin;
extern ASTNode* root; // This will be set by the parser

int main(int argc, char *argv[]) {
    if (argc == 1){
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }
    FILE *file = fopen(argv[1], "r");
    if (!file) {
        fprintf(stderr, "Could not open %s\n", argv[1]);
        return 1;
    }
    yyin = file;
    
    
    int result = yyparse();
    
    if (result == 0) {
        printf("\nParsing successful! Here's the AST:\n");
        print_ast(root, 0);
        
        // Here you can:
        // 1. Generate code from the AST
        // generate_code(root);
        
        // 2. Interpret the AST directly
        // interpret_ast(root);
        
        // 3. Perform optimizations
        // optimize_ast(root);
        
        // Don't forget to clean up
    } else {
        printf("Parsing failed!\n");
    }
    
    return result;
}
void error_wrong_node(NodeType expected, NodeType actual, const char* function_name){
    fprintf(stderr,"%s called with wrong node ( expected ",function_name);
    print_node_type(expected);
    fprintf(stderr, ", actual ");
    print_node_type(actual);
    fprintf(stderr,"\n");
    exit(1);

}
bool is_declaration_array(ASTNode* declaration){
    if (declaration->type != NODE_DECLARATION){
        error_wrong_node(NODE_DECLARATION,declaration->type,"is_declaration_array");
    }
    return declaration->data.declaration.name->type == NODE_ARRAY_ACCESS;
}
int eval_constant(ASTNode* constant_value){
    return 0;
}
int array_size(ASTNode* array){
    if (array->type != NODE_ARRAY_ACCESS){
        error_wrong_node(NODE_ARRAY_ACCESS,array->type,"array_size");
    }
    return 0;
}


int count_num_alloc(ASTNode* node){
    if (node == NULL)
        return 0;
    switch (node->type){
        case NODE_IDENTIFIER:
        case NODE_CONSTANT:
        case NODE_ARRAY_ACCESS:
        case NODE_FUNCTION_CALL:
        case NODE_UNARY_OP:
        case NODE_ID_LIST:
        case NODE_STMT_LIST:
        case NODE_PARAM_LIST:
        case NODE_INIT_LIST:
        case NODE_ARG_LIST:
        case NODE_TOP_LEVEL_LIST:
        case NODE_TYPE:
        case NODE_BINARY_OP:
        case NODE_CONTINUE:
        case NODE_BREAK:
            return 0;
        case NODE_FUNCTION_DEF:
            return count_num_alloc(node->data.function_def.parameters) +
                    count_num_alloc(node->data.function_def.body);
        case NODE_DECLARATION_LIST:
        case NODE_DECLARATOR_LIST:
            return count_num_alloc(node->data.arg_list.arg) +
                    count_num_alloc(node->data.arg_list.arg);
        case NODE_DECLARATION:
            return count_num_alloc(node->data.declaration.value);
    }
}
