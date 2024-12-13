#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "global_manager.h"
#include "ast.h"
#include "y.tab.h"


extern int yyparse(void);
extern void yyrestart(FILE *input_file);
extern void yylex_destroy(void);

extern FILE *yyin;
extern ASTNode *root; // This will be set by the parser
int DEBUG_MAIN = 0;

int main(int argc, char *argv[]) {
    if (argc == 1) {
        printf("Usage: %s <filename> [--no-mem] [--debug] [--print-func-call]\n", argv[0]);
        return 1;
    }
    FILE *file = fopen(argv[1], "r");
    if (!file) {
        fprintf(stderr, "Could not open %s\n", argv[1]);
        return 1;
    }
    yyin = file;

    int show_mem = 1;
    int print_func_call = 0;
    for (int i = 2; i < argc; ++i) {
        if (strcmp(argv[i],"--no-mem") == 0){
            show_mem = 0;
        }else if(strcmp(argv[i],"--debug") == 0){
            DEBUG_MAIN = 1;
        }else if(strcmp(argv[i],"--print-func-call") == 0){
            print_func_call = 1;
        }
        else if (strcmp(argv[i],"--help") == 0){
            printf("Usage: %s <filename> [--no-mem] [--interactive] [--print-func-call]\n", argv[0]);
            printf("--debug: Opens the debug window before every instruction\n");
            printf("--no-mem: Do not print memory state while debugging (set this if you have big buffers)\n");
            printf("--print-func-call: Print function calls \n");
            return 1;
        }else{
            printf("Unknown argument %s\n",argv[i]);
            return 1;
        }
    }
    if (DEBUG_MAIN)
        printf("\nINPUT PROGRAM\n================================\n");

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
        execute_code(global_manager,DEBUG_MAIN, show_mem,print_func_call);
        destroy_global_manager(global_manager);
    } else {
        if (DEBUG_MAIN)
            printf("Parsing failed!\n");
        fclose(file);
        // Parse again with debug to show the error
        if (!DEBUG_MAIN){
            FILE *file = fopen(argv[1], "r");
            if (!file) {
                fprintf(stderr, "Could not open %s\n", argv[1]);
                return 1;
            }

            yyrestart(yyin);
            yylex_destroy();
            yyin = file;

            DEBUG_MAIN = 1;
            yyparse();
        }
    }

    return result;
}
