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
    // TODO put values of arguments into the function call
    call_function(scope_manager,from_args);
}
void add_correct_value_type_to_v(full_type_t * full_type, void* var_address, Value* v){
    // Pointers are represented as integers as well
    if (full_type->n_pointers > 0){
        v->value.i = *(int*)var_address;
        return;
    }
    switch (full_type->type){
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
    switch (unary_op->data.unary.operator) {
        case NODE_ADDRESS:
            ValueOrAddress res = eval_expr(global_manager,unary_op->data.unary.operand);
            if (!res.has_address){
                fprintf(stderr,"[ERROR] Tried to take the address of a non-addressed value (l. %d)",unary_op->line_number);
                exit(1);
            }
            Value new_value = {res.value.is_constant,{res.value.type.type,res.value.type.n_pointers+1},res.value.value};
            return (ValueOrAddress){new_value,0,-1};
        case NODE_DEREF:
            ValueOrAddress res2 = eval_expr(global_manager,unary_op->data.unary.operand);
            if (res2.value.type.n_pointers == 0){
                fprintf(stderr,"[ERROR] Tried to dereference a non-pointer value (l. %d)",unary_op->line_number);
                exit(1);
            }
            if (res2.value.is_constant){
                fprintf(stderr,"[ERROR] Tried to dereference a constant pointer, not possible...");
                exit(1);
            }
            int ptr = res2.value.value.i;
            void* raw_ptr = get_raw_ptr_for_var(global_manager->memory_manager,ptr);
            new_value = (Value){0,{res2.value.type.type,res2.value.type.n_pointers-1},0};
            add_correct_value_type_to_v(&new_value.type,raw_ptr,&new_value);
            return (ValueOrAddress){new_value,1,ptr};
        // This node is useless ??
        case NODE_PLUS:
            return eval_expr(global_manager,unary_op->data.unary.operand);
        case NODE_MINUS:
            ValueOrAddress res3 = eval_expr(global_manager,unary_op->data.unary.operand);
            // TODO continue here
            new_value = (Value){res3.value.is_constant,res3.value.type,0};
            if (res3.value.type.n_pointers > 0){
                error_out(unary_op, "Cannot take the negative value of a pointer");
            }
            switch (res3.value.type.type) {
                case NODE_INT:res3.value.value.i *= -1;break;
                case NODE_FLOAT:res3.value.value.f *= -1.0f;break;
                case NODE_CHAR:res3.value.value.c *= (char)(-1);break;
                case NODE_VOID:error_out(unary_op,"Tried to take the negative of void ??");break;
            }
            return (ValueOrAddress) {new_value,0,-1};
        case NODE_INC_POST:
            ValueOrAddress res4 = eval_expr(global_manager,unary_op->data.unary.operand);
            if (!res4.has_address) error_out(unary_op,"Operand requires the value to be addressable");
            raw_ptr = get_raw_ptr_for_var(global_manager->memory_manager,res4.address);
            switch (res4.value.type.type) {
                case NODE_INT:*(int*)(raw_ptr) += 1;break;
                case NODE_FLOAT:*(float*)(raw_ptr) += 1.0f;break;
                case NODE_VOID:error_out(unary_op,"Tried to increment void ??");break;
                case NODE_CHAR:*(char*)raw_ptr += (char)1;break;
            }
            new_value = (Value){0,res4.value.type,0};

            break;
        case NODE_DEC_POST:
            break;
        case NODE_INC_PRE:
            break;
        case NODE_DEC_PRE:
            break;
        case NODE_BITWISE_NOT:
            break;
        case NODE_LOGICAL_NOT:
            break;
    }
    return (ValueOrAddress) {0,{},0,0};
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
            Value v = {0,*(a->type),0};
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
            add_correct_value_type_to_v(&new_type,element_ptr,&result);
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