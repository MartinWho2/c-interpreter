#include <stdbool.h>

#include "memory_manager.h"
#include "symbol_table.h"

typedef struct GlobalManager {
    MemoryManager * memory_manager;
    ScopeManager * scope_manager;
    ASTNode * root;
} GlobalManager;


GlobalManager * create_global_manager(ASTNode* root);
ValueOrAddress eval_expr(GlobalManager * global_manager, ASTNode* expr);
SymbolTable * construct_symbol_table_for_function_call(ASTNode* function_def, MemoryManager* memory_manager);