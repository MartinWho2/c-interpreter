#include "global_manager.h"
GlobalManager * create_global_manager(ASTNode* root){
    MemoryManager * memory_manager = create_memory_manager();
    ScopeManager * scope_manager = create_scope_manager();
    GlobalManager * global_manager = malloc(sizeof (GlobalManager));
    global_manager->scope_manager = scope_manager;
    global_manager->memory_manager = memory_manager;
    global_manager->root = root;
    return global_manager;
}
void destroy_global_manager(GlobalManager* globalManager){
    destroy_memory_manager(globalManager->memory_manager);
    destroy_scope_manager(globalManager->scope_manager);
    free(globalManager);
}

void call_function_total(GlobalManager *global_manager,ASTNode* fun_def, ASTNode* fun_call){
    MemoryManager * memory_manager = global_manager->memory_manager;
    ScopeManager * scope_manager = global_manager->scope_manager;
    FrameList * new_frame_list = add_new_frame_list(memory_manager->frame_list,memory_manager->stack_pointer);
    memory_manager->frame_list = new_frame_list;
    SymbolTable * from_args = construct_symbol_table_for_function_call(fun_def,memory_manager);
    // TODO put values of arguments into the function call
    call_function(scope_manager,from_args);
}

int eval_expr(GlobalManager *global_manager,ASTNode* ast_node){
    switch (ast_node->type) {
        case NODE_IDENTIFIER:
            SymbolEntry* a =  lookup_symbol(global_manager->scope_manager, ast_node->data.identifier.name);
            void* var = get_raw_ptr_for_var(global_manager->memory_manager,a->address);
            break;
        case NODE_CONSTANT:
            break;
        case NODE_ARRAY_ACCESS:
            break;
        case NODE_FUNCTION_CALL:
            break;
        case NODE_UNARY_OP:
            break;
        case NODE_ID_LIST:
            break;
        case NODE_PARAM_LIST:
            break;
        case NODE_STMT_LIST:
            break;
        case NODE_DECLARATION_LIST:
            break;
        case NODE_DECLARATOR_LIST:
            break;
        case NODE_INIT_LIST:
            break;
        case NODE_ARG_LIST:
            break;
        case NODE_TOP_LEVEL_LIST:
            break;
        case NODE_TYPE:
            break;
        case NODE_BINARY_OP:
            break;
        case NODE_ASSIGNMENT:
            break;
        case NODE_DECLARATION:
            break;
        case NODE_ARRAY_DECLARATION:
            break;
        case NODE_PARAM_DECLARATION:
            break;
        case NODE_COMPOUND_STMT:
            break;
        case NODE_IF:
            break;
        case NODE_WHILE:
            break;
        case NODE_DO_WHILE:
            break;
        case NODE_FOR:
            break;
        case NODE_CONTINUE:
            break;
        case NODE_BREAK:
            break;
        case NODE_RETURN:
            break;
        case NODE_FUNCTION_DEF:
            break;
    }
    return 0;
}

SymbolTable * construct_symbol_table_for_function_call(ASTNode* function_def,MemoryManager* memory_manager){
    error_on_wrong_node(NODE_FUNCTION_DEF,function_def->type,"create_symbol_table_for_func_call");
    SymbolTable * symbol_table = create_symbol_table(100);
    ASTNode * params = function_def->data.function_def.parameters;
    if (!params) return symbol_table;
    while (params->type == NODE_PARAM_LIST){
        ASTNode * curr_param = params->data.arg_list.arg;
        insert_param_in_symbol_table(curr_param,symbol_table,&(memory_manager->stack_pointer));
        params = params->data.arg_list.next;
    }
    insert_param_in_symbol_table(params,symbol_table,&(memory_manager->stack_pointer));
    return symbol_table;
}