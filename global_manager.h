#include <stdbool.h>

#include "memory_manager.h"
#include "symbol_table.h"
#include "flow_manager.h"

typedef struct functions_t {
    ASTNode * func_def;
    char* name;
}functions_t;

typedef struct GlobalManager {
    MemoryManager * memory_manager;
    ScopeManager * scope_manager;
    FlowManager * flow_manager;
    ASTNode * root;
    functions_t** function_defs;
    int n_funcs;
} GlobalManager;

void destroy_global_manager(GlobalManager* globalManager);
GlobalManager * create_global_manager(ASTNode* root);
ValueOrAddress eval_expr(GlobalManager * global_manager, ASTNode* expr);
SymbolTable * construct_symbol_table_for_function_call(ASTNode* function_def, MemoryManager* memory_manager);
void insert_arguments_from_func_call(GlobalManager * global_manager,SymbolTable* table, ASTNode* func_def, ASTNode* func_call);
Value get_value(GlobalManager* global_manager,const char* name);
void declare_new_variable(GlobalManager* global_manager, const char* name, Value value);
void declare_new_variable_without_value(GlobalManager* global_manager, const char* name, full_type_t type);
void call_function_total(GlobalManager *global_manager,ASTNode* fun_def, ASTNode* fun_call);
ValueOrAddress return_function_total(GlobalManager* global_manager,ASTNode* return_value, full_type_t return_type);
void register_all_functions(GlobalManager* global_manager);
void execute_code(GlobalManager* globalManager, int debug, int show_mem, int print_func_call);
static inline void declare(GlobalManager* globalManager, ASTNode* curr_instr);
void declare_new_array_without_value(GlobalManager* globalManager, const char* name, full_type_t type, int n_elems);
void declare_new_array_with_constant_value(GlobalManager* globalManager, const char* name, full_type_t type, int n_elems, Value value);
void declare_new_array_with_some_values(GlobalManager* globalManager,const char* name, full_type_t type, int buf_n_elems, Value* values, int n_values);
Value *get_array_init_res(GlobalManager *globalManager,ASTNode *array_init, size_t* out_len);

int handle_printf_call(GlobalManager *globalManager,ValueOrAddress* args, int num_args);

void print_global_manager_state(GlobalManager* global_manager);
void print_memory(GlobalManager* global_manager);