#include <stdio.h>
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
    insert_arguments_from_func_call(global_manager,from_args,fun_def,fun_call);
    call_function(scope_manager,from_args);
}
ValueOrAddress return_function_total(GlobalManager* global_manager,ASTNode* return_value, full_type_t return_type){
    MemoryManager * memory_manager = global_manager->memory_manager;
    ScopeManager * scope_manager = global_manager->scope_manager;
    set_stack_pointer_to_curr_frame_pointer(global_manager->memory_manager);
    memory_manager->frame_list = return_to_last_frame(memory_manager->frame_list);
    ValueOrAddress ret_value;
    if (return_value == NULL){
        ret_value = (ValueOrAddress){{1,{NODE_VOID,0},0},0,-1};
    }else{
        ret_value = eval_expr(global_manager,return_value);
    }
    if (ret_value.value.type.type != return_type.type || ret_value.value.type.n_pointers != return_type.n_pointers){
        error_out(return_value,"Returning wrong type from function\n");
        print_type(ret_value.value.type,1);
        printf("instead of ");
        print_type(return_type,1);
        exit(1);
    }
    return_to_prev_function(scope_manager);
    return ret_value;
}



void add_correct_value_type_to_v(full_type_t full_type, void* var_address, Value* v){
    // Pointers are represented as integers as well
    if (full_type.n_pointers > 0){
        v->value.i = *(int*)var_address;
        return;
    }
    switch (full_type.type){
        case NODE_CHAR:
            v->value.c = *(char*)var_address; return;
        case NODE_INT:
            v->value.i = *(int*)var_address; return;
        case NODE_FLOAT:
            v->value.f = *(float*)var_address; return;
        case NODE_VOID:
            fprintf(stderr,"[ERROR] Tried to evaluate a value of type void ??\n");
            exit(1);
    }
}

ValueOrAddress eval_unary_op(GlobalManager *global_manager, ASTNode* unary_op){
    error_on_wrong_node(NODE_UNARY_OP,unary_op->type,"eval_unary_op");
    ValueOrAddress res;
    Value new_value;
    switch (unary_op->data.unary.operator) {
        case NODE_ADDRESS:
            res = eval_expr(global_manager,unary_op->data.unary.operand);
            if (!res.has_address){
                error_out(unary_op,"Tried to take the address of a non-addressed value");
            }
             new_value = (Value){res.value.is_constant,{res.value.type.type,res.value.type.n_pointers+1},res.value.value};
            return (ValueOrAddress){new_value,0,-1};
        case NODE_DEREF:
            res = eval_expr(global_manager,unary_op->data.unary.operand);
            if (res.value.type.n_pointers == 0){
                error_out(unary_op,"Tried to dereference a non-pointer value");
            }
            if (res.value.is_constant){
                error_out(unary_op,"Tried to dereference a constant pointer, not possible...");
            }
            int ptr = res.value.value.i;
            void* raw_ptr = get_raw_ptr_for_var(global_manager->memory_manager,ptr);
            new_value = (Value){0,{res.value.type.type,res.value.type.n_pointers-1},0};
            add_correct_value_type_to_v(new_value.type,raw_ptr,&new_value);
            return (ValueOrAddress){new_value,1,ptr};
        // This node is useless ??
        case NODE_PLUS:
            return eval_expr(global_manager,unary_op->data.unary.operand);
        case NODE_MINUS:
            res = eval_expr(global_manager,unary_op->data.unary.operand);
            fprintf(stderr,"DEBUG");
            print_val_or_addr(res);
            new_value = (Value){res.value.is_constant,res.value.type,0};
            if (res.value.type.n_pointers > 0){
                error_out(unary_op, "Cannot take the negative value of a pointer");
            }
            switch (res.value.type.type) {
                case NODE_INT:new_value.value.i = -res.value.value.i;break;
                case NODE_FLOAT:new_value.value.f = -res.value.value.f;break;
                case NODE_CHAR:new_value.value.c = (char)-(res.value.value.c);break;
                case NODE_VOID:error_out(unary_op,"Tried to take the negative of void ??");break;
            }
            return (ValueOrAddress) {new_value,0,-1};
        case NODE_INC_POST:
            res = eval_expr(global_manager,unary_op->data.unary.operand);
            if (!res.has_address) error_out(unary_op,"Operand requires the value to be addressable");
            raw_ptr = get_raw_ptr_for_var(global_manager->memory_manager,res.address);
            new_value = (Value){0,res.value.type,res.value.value};
            if (res.value.type.n_pointers> 0){
                *(int*)raw_ptr += 1;
                return (ValueOrAddress){new_value,0,-1};
            }
            switch (res.value.type.type) {
                case NODE_INT:*(int*)(raw_ptr) += 1;break;
                case NODE_FLOAT:*(float*)(raw_ptr) += 1.0f;break;
                case NODE_VOID:error_out(unary_op,"Tried to increment void ??");break;
                case NODE_CHAR:*(char*)raw_ptr += (char)1;break;
            }
            return (ValueOrAddress){new_value,0,-1};
        case NODE_DEC_POST:
            res = eval_expr(global_manager,unary_op->data.unary.operand);
            if (!res.has_address) error_out(unary_op,"Operand requires the value to be addressable");
            raw_ptr = get_raw_ptr_for_var(global_manager->memory_manager,res.address);
            new_value = (Value){0,res.value.type,res.value.value};
            if (res.value.type.n_pointers> 0){
                *(int*)raw_ptr -= 1;
                return (ValueOrAddress){new_value,0,-1};
            }
            switch (res.value.type.type) {
                case NODE_INT:*(int*)(raw_ptr) -= 1;break;
                case NODE_FLOAT:*(float*)(raw_ptr) -= 1.0f;break;
                case NODE_VOID:error_out(unary_op,"Tried to decrement void ??");break;
                case NODE_CHAR:*(char*)raw_ptr -= (char)1;break;
            }
            return (ValueOrAddress){new_value,0,-1};
        case NODE_INC_PRE:
            res = eval_expr(global_manager,unary_op->data.unary.operand);
            if (!res.has_address) error_out(unary_op,"Operand requires the value to be addressable");
            raw_ptr = get_raw_ptr_for_var(global_manager->memory_manager,res.address);
            new_value = (Value){0,res.value.type,res.value.value};
            if (res.value.type.n_pointers> 0){
                *(int*)raw_ptr += 1;
                new_value.value.i += 1;
                return (ValueOrAddress){new_value,0,-1};
            }
            switch (res.value.type.type) {
                case NODE_INT:*(int*)(raw_ptr) += 1;new_value.value.i += 1;break;
                case NODE_FLOAT:*(float*)(raw_ptr) += 1.0f;new_value.value.f += 1.0f;break;
                case NODE_VOID:error_out(unary_op,"Tried to increment void ??");break;
                case NODE_CHAR:*(char*)raw_ptr += (char)1;new_value.value.c += (char)1;break;
            }
            return (ValueOrAddress){new_value,0,-1};
        case NODE_DEC_PRE:
            res = eval_expr(global_manager,unary_op->data.unary.operand);
            if (!res.has_address) error_out(unary_op,"Operand requires the value to be addressable");
            raw_ptr = get_raw_ptr_for_var(global_manager->memory_manager,res.address);
            new_value = (Value){0,res.value.type,res.value.value};
            if (res.value.type.n_pointers> 0){
                *(int*)raw_ptr -= 1;
                new_value.value.i -= 1;
                return (ValueOrAddress){new_value,0,-1};
            }
            switch (res.value.type.type) {
                case NODE_INT:*(int*)(raw_ptr) -= 1;new_value.value.i -= 1;break;
                case NODE_FLOAT:*(float*)(raw_ptr) -= 1.0f;new_value.value.f -= 1.0f;break;
                case NODE_VOID:error_out(unary_op,"Tried to decrement void ??");break;
                case NODE_CHAR:*(char*)raw_ptr -= (char)1;new_value.value.c -= (char)1;break;
            }
            return (ValueOrAddress){new_value,0,-1};
        case NODE_BITWISE_NOT:
            res = eval_expr(global_manager,unary_op->data.unary.operand);
            if (res.value.type.n_pointers > 0) error_out(unary_op,"Bitwise not of a pointer ??");
            if (res.value.type.type == NODE_FLOAT || res.value.type.type == NODE_VOID) error_out(unary_op,"bitwise not on float or void ??");
            // since ony for int and char, we can just invert all bits of biggest field
            new_value = (Value) {0,res.value.type,~res.value.value.i};
            return (ValueOrAddress){new_value,0,-1};
        case NODE_LOGICAL_NOT:
            res = eval_expr(global_manager,unary_op->data.unary.operand);
            if (res.value.type.n_pointers > 0) error_out(unary_op,"Logical not of a pointer ??");
            new_value = (Value){0,res.value.type,0};
            switch (res.value.type.type) {
                case NODE_INT:
                    new_value.value.i = res.value.value.i == 0 ? 1 : 0;
                    break;
                case NODE_CHAR:
                    new_value.value.c = new_value.value.c == (char)0 ? (char)1 : (char)0;
                    break;
                case NODE_FLOAT:
                    new_value.value.f = new_value.value.f == 0.0f ? 1.0f : 0.0f;
                    break;
                case NODE_VOID:
                    error_out(unary_op,"logical not on void ??");
            }
            return (ValueOrAddress){new_value,0,-1};
    }
}
ValueOrAddress eval_binary_op(GlobalManager* global_manager,ASTNode* binary_op){

}

// a = 12;
// *(&p) = &b;
// p--[128] = 17;
// f()[12] = 12;
// *(&a) = 12
// (p+p2)[12]
// *(p+p2)

// This has side effects such as evaluating x in a[x], where x could be "b=12", so b we will be assigned the value 12
// BE CAREFUL TO NEVER CALL TWICE ON SAME NODE !!
ValueOrAddress eval_expr(GlobalManager *global_manager,ASTNode* ast_node){
    switch (ast_node->type) {
        case NODE_IDENTIFIER:
            SymbolEntry* a = lookup_symbol(global_manager->scope_manager, ast_node->data.identifier.name);
            void* var = get_raw_ptr_for_var(global_manager->memory_manager,a->address);
            Value v = {0,a->type,0};
            add_correct_value_type_to_v(a->type,var,&v);
            ValueOrAddress value_or_address = {v,1,a->address};
            return value_or_address;
        case NODE_CONSTANT:
            v = *(ast_node->data.constant.value);
            return (ValueOrAddress) {v,0,-1};
        case NODE_ARRAY_ACCESS:
            ValueOrAddress array = eval_expr(global_manager,ast_node->data.array_access.array);
            if (array.value.type.n_pointers == 0){
                fprintf(stderr,"[ERROR] Tried to access an array on a non-pointer variable (l. %d)\n",ast_node->line_number);
                exit(1);
            }
            ValueOrAddress index = eval_expr(global_manager,ast_node->data.array_access.index);
            if ((index.value.type.type != NODE_INT && index.value.type.type != NODE_CHAR)||
                index.value.type.n_pointers > 0){
                fprintf(stderr,"[ERROR] Array index must be an integer or char (l. %d)\n",ast_node->line_number);
                exit(1);
            }
            int arr_index = index.value.type.type == NODE_CHAR ? index.value.value.c: index.value.value.i;
            full_type_t new_type = {v.type.type,v.type.n_pointers - 1};
            int final_ptr = v.value.i + arr_index * type_size(&new_type);
            void* element_ptr = get_raw_ptr_for_var(global_manager->memory_manager,final_ptr);
            Value result = {0,new_type,0};
            add_correct_value_type_to_v(new_type,element_ptr,&result);
            ValueOrAddress ret = {result,1,v.value.i};
            return ret;
        case NODE_FUNCTION_CALL:

            // TODO call function
            break;
        case NODE_UNARY_OP:
            return eval_unary_op(global_manager,ast_node);
        case NODE_INIT_LIST:
            // TODO init the list and returns the pointer
            break;
        case NODE_BINARY_OP:
            return eval_binary_op(global_manager,ast_node);
        case NODE_ASSIGNMENT:
            ValueOrAddress left = eval_expr(global_manager,ast_node->data.assignment.left);
            assign_operator assign = ast_node->data.assignment.operator;
            ValueOrAddress right = eval_expr(global_manager,ast_node->data.assignment.right);
            return left;
        case NODE_PARAM_LIST:
        case NODE_STMT_LIST:
        case NODE_DECLARATION_LIST:
        case NODE_ARG_LIST:
        case NODE_DECLARATOR_LIST:
        case NODE_DECLARATION:
        case NODE_TOP_LEVEL_LIST:
        case NODE_ARRAY_DECLARATION:
        case NODE_PARAM_DECLARATION:
        case NODE_COMPOUND_STMT:
        case NODE_IF:
        case NODE_WHILE:
        case NODE_DO_WHILE:
        case NODE_FOR:
        case NODE_CONTINUE:
        case NODE_BREAK:
        case NODE_RETURN:
        case NODE_FUNCTION_DEF:
            fprintf(stderr,"[ERROR] Tried to evaluate the expression of something which is not an expression");
            print_node_type(ast_node->type);
            exit(1);
    }
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


void insert_arguments_from_func_call(GlobalManager * global_manager,SymbolTable* table, ASTNode* func_def, ASTNode* func_call){
    error_on_wrong_node(NODE_FUNCTION_DEF,func_def->type,"insert_arg_from_func_call");
    error_on_wrong_node(NODE_FUNCTION_CALL,func_call->type,"insert_arg_from_func_call");
    ASTNode* params = func_def->data.function_def.parameters;
    ASTNode* args = func_call->data.function_call.args;
    ASTNode *curr_param, *curr_arg;
    int finished = 0;
    while (!finished){
        if (params->type == NODE_PARAM_DECLARATION && args->type == NODE_ARG_LIST){
            error_out(func_call,"Too many arguments for function call");
        }
        if (params->type == NODE_PARAM_LIST && args->type != NODE_ARG_LIST){
            error_out(func_call,"Not enough arguments for function call");
        }
        if (args->type == NODE_ARG_LIST) {
            curr_param = params->data.arg_list.arg;
            curr_arg = args->data.arg_list.arg;
        }else {
            curr_param = params;
            curr_arg = args;
            finished = 1;
        }
        ValueOrAddress arg_evaluated = eval_expr(global_manager,curr_arg);
        full_type_t param_type = *curr_param->data.param_declaration.type;
        if ( param_type.type != arg_evaluated.value.type.type || param_type.n_pointers != arg_evaluated.value.type.n_pointers ){
            error_out(func_call, "Function called with wrong type of parameter");
        }
        SymbolEntry *entry = lookup_symbol_in_table(table,curr_param->data.param_declaration.name->data.identifier.name);
        void* raw_ptr = get_raw_ptr_for_var(global_manager->memory_manager,entry->address);
        if (param_type.n_pointers > 0){
            *(int*)raw_ptr = arg_evaluated.value.value.i;
        }else{
            switch (param_type.type){
                case NODE_INT:*(int*)raw_ptr = arg_evaluated.value.value.i;break;
                case NODE_FLOAT:*(float*)raw_ptr = arg_evaluated.value.value.f;break;
                case NODE_VOID:fprintf(stderr,"Error parameter of type void??");exit(1);
                case NODE_CHAR:*(char*)raw_ptr = arg_evaluated.value.value.c;break;
            }
        }
        params = params->data.arg_list.next;
        args = args->data.arg_list.next;
    }
}

Value get_value(GlobalManager* global_manager,const char* name){
    SymbolEntry *entry = lookup_symbol(global_manager->scope_manager,name);
    void* raw_ptr = get_raw_ptr_for_var(global_manager->memory_manager,entry->address);
    full_type_t value_type = entry->type;
    Value value = {0,value_type,0};
    if (value_type.n_pointers > 0){
        value.value.i = *(int*)raw_ptr;
        return value;
    }
    switch (value_type.type) {
        case NODE_INT:value.value.i = *(int*)raw_ptr;return value;
        case NODE_FLOAT:value.value.f = *(float*)raw_ptr;return value;
        case NODE_VOID:
            fprintf(stderr,"Tried to get the value of a void value ??");exit(1);
        case NODE_CHAR:value.value.c = *(char*)raw_ptr;return value;
    }
}
// TODO GLOBAL VARIABLES SHOULD NOT BE STORED IN THE STACK !!!!! Create a new memory for just globals
void declare_new_variable(GlobalManager* global_manager, const char* name, Value value){
    print_type(value.type,0);
    int address = declare_new_variable_in_memory(global_manager->memory_manager,&value.type);
    insert_symbol(global_manager->scope_manager,name,&value.type,address,false);
    void *var_ptr = get_raw_ptr_for_var(global_manager->memory_manager,address);
    if (value.type.n_pointers> 0){
        *(int*)var_ptr = value.value.i;
    }
    switch (value.type.type) {
        case NODE_INT:*(int*)var_ptr = value.value.i;break;
        case NODE_FLOAT:*(float*)var_ptr = value.value.f;break;
        case NODE_VOID:fprintf(stderr,"Tried to declare a variable of type void ??");exit(1);
        case NODE_CHAR:*(char*)var_ptr = value.value.c;break;
    }
}

void print_memory(GlobalManager* global_manager){
    printf("\n\nMEMORY\n");
    for (int i = 0; i < global_manager->memory_manager->stack_pointer; ++i) {
        printf("%02x|",(( unsigned char*)(global_manager->memory_manager->base_of_stack))[i]);
    }
    printf("\n\n");
}

void print_global_manager_state(GlobalManager* global_manager){
    printf("\n\nGlobal's manager state:\n");
    printf("  SP = %d\n",global_manager->memory_manager->stack_pointer);
    printf("  FP = %d\n",global_manager->memory_manager->frame_list->current_frame_pointer);
    printf("  Memory size = %d\n",global_manager->memory_manager->size_memory);
    printf("  Functions calls = %d", num_fun_calls(global_manager->scope_manager));
    print_memory(global_manager);

}