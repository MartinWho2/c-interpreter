#include "symbol_table.h"
#include "memory_manager.h"



int main(){
    MemoryManager * memory_manager = create_memory_manager();
    ScopeManager * scope_manager = create_scope_manager();
    full_type_t * int_type = create_type(NODE_INT,0);
    full_type_t * float_type = create_type(NODE_FLOAT,0);

    ASTNode *id1 = create_identifier("var1");
    ASTNode *param1 = create_param_declaration(int_type,id1);

    ASTNode *id2 = create_identifier("var2");
    ASTNode *param2 = create_param_declaration(int_type,id2);

    ASTNode *id3 = create_identifier("var3");
    ASTNode *param3 = create_param_declaration(float_type,id3);

    ASTNode * params = create_list(param2,param3,NODE_PARAM_LIST);
    params = create_list(param1,params,NODE_PARAM_LIST);

    ASTNode * func_def = create_function_def(int_type,"main",params, create_compound_stmt(NULL,NULL));

    SymbolTable * args = construct_symbol_table_for_function_call(func_def,memory_manager);
    call_function(scope_manager,args);



}
