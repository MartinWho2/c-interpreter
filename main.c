#include <stdio.h>
#include <stdbool.h>
#include "global_manager.h"
#include "ast.h"
#include "y.tab.h"


extern int yyparse(void);

extern FILE *yyin;
extern ASTNode *root; // This will be set by the parser

int main(int argc, char *argv[]) {
    if (argc == 1) {
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
        //printf("\nParsing successful! Here's the AST:\n");
        //print_ast(root, 0);
        GlobalManager * global_manager = create_global_manager(root);
        execute_code(global_manager);
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


bool is_declaration_array(ASTNode *declaration) {
    error_on_wrong_node(NODE_DECLARATION, declaration->type, "is_declaration_array");

    return declaration->data.declaration.name->type == NODE_ARRAY_ACCESS;
}
