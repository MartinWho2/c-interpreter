#include <stdio.h>
#include "global_manager.h"


int main(){
    full_type_t * int_ptr_type = create_type(NODE_INT,1);
    full_type_t * int_type = create_type(NODE_INT,0);

    full_type_t * float_type = create_type(NODE_FLOAT,0);
    full_type_t * char_type = create_type(NODE_CHAR,0);

    ASTNode *id1 = create_identifier("var1");
    ASTNode *param1 = create_param_declaration(int_ptr_type,id1);

    ASTNode *id2 = create_identifier("var2");
    ASTNode *param2 = create_param_declaration(int_type,id2);

    ASTNode *id3 = create_identifier("var3");
    ASTNode *param3 = create_param_declaration(float_type,id3);

    ASTNode * params = create_list(param2,param3,NODE_PARAM_LIST);
    params = create_list(param1,params,NODE_PARAM_LIST);

    ASTNode * func_def = create_function_def(int_type,"main",params, create_compound_stmt(NULL,NULL));
    GlobalManager *global_manager = create_global_manager(func_def);

    Value c1 = {1,{NODE_INT,1},.value.i=12345};
    ASTNode * arg1 = create_constant(&c1);

    Value c2 = {1,{NODE_INT,0},.value.i=123};
    ASTNode * arg2 = create_constant(&c2);

    Value c3 = {1,*float_type,.value.f=1.23f};
    ASTNode * arg3 = create_constant(&c3);

    ASTNode *arg_list = create_list(arg2,arg3,NODE_ARG_LIST);
    arg_list = create_list(arg1,arg_list,NODE_ARG_LIST);

    ASTNode* func_call = create_function_call(create_identifier("main"),arg_list);
    call_function_total(global_manager,func_def,func_call);
    declare_new_variable(global_manager,"x",(Value){1,{NODE_INT,1},.value.i=763});

    ASTNode * x = create_identifier("x");
    ASTNode * x_plus_plus = create_un_op(x,NODE_DEC_POST);
    ASTNode *minus_id2 = create_un_op(id2,NODE_MINUS);
    ASTNode* arg_list2 = create_list(minus_id2,id3,NODE_ARG_LIST);
    arg_list2 = create_list(x_plus_plus,arg_list2,NODE_ARG_LIST);
    //arg_list2 = create_list(x_plus_plus,arg_list2,NODE_ARG_LIST);
    ASTNode* func_call2 = create_function_call(create_identifier("main"),arg_list2);
    /*SymbolTable * args2 = construct_symbol_table_for_function_call(func_def,global_manager->memory_manager);
    insert_arguments_from_func_call(global_manager,args2,func_def,func_call2);
    call_function(global_manager->scope_manager,args2);*/
    call_function_total(global_manager,func_def,func_call2);
    printf("\n%d\n", lookup_symbol(global_manager->scope_manager,"var1")->address);
    printf("\n%d\n", lookup_symbol(global_manager->scope_manager,"var2")->address);
    printf("\n%d\n", lookup_symbol(global_manager->scope_manager,"var3")->address);
    printf("var1 = ");
    print_value(get_value(global_manager,"var1"));
    printf("var2 = ");
    print_value(get_value(global_manager,"var2"));
    printf("var3 = ");
    print_value(get_value(global_manager,"var3"));
    print_global_manager_state(global_manager);
    ValueOrAddress v_ret = return_function_total(global_manager,id2,(full_type_t){NODE_INT,0});
    print_global_manager_state(global_manager);
    printf("var1 = ");
    print_value(get_value(global_manager,"var1"));
    printf("var2 = ");
    print_value(get_value(global_manager,"var2"));
    printf("var3 = ");
    print_value(get_value(global_manager,"var3"));
}
