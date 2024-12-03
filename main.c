#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "global_manager.h"
#include "ast.h"
#include "y.tab.h"


extern int yyparse(void);

extern FILE *yyin;
extern ASTNode *root; // This will be set by the parser
int DEBUG_MAIN = 0;

int main(int argc, char *argv[]) {
    if (argc == 1) {
        printf("Usage: %s <filename> [DEBUG]\n", argv[0]);
        return 1;
    }
    FILE *file = fopen(argv[1], "r");
    if (!file) {
        fprintf(stderr, "Could not open %s\n", argv[1]);
        return 1;
    }
    yyin = file;

    if (argc >= 3 && strcmp(argv[2],"DEBUG") == 0){
        DEBUG_MAIN = 1;
        printf("\nINPUT PROGRAM\n================================\n");
    }
    int result = yyparse();
    if (DEBUG_MAIN)
        printf("\n=======================\n");


    if (result == 0) {
        //printf("\nParsing successful! Here's the AST:\n");
        if (DEBUG_MAIN){
            printf("\nABSTRACT SYNTAX TREE\n\n===================\n\n");
            print_ast(root, 0);
            printf("\n=====================\n\n");
        }
        GlobalManager * global_manager = create_global_manager(root);
        execute_code(global_manager,DEBUG_MAIN);
        destroy_global_manager(global_manager);
    } else {
        printf("Parsing failed!\n");
    }

    return result;
}


bool is_declaration_array(ASTNode *declaration) {
    error_on_wrong_node(NODE_DECLARATION, declaration->type, "is_declaration_array");

    return declaration->data.declaration.name->type == NODE_ARRAY_ACCESS;
}
