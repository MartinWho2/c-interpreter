#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "global_manager.h"
#define VOID_VALUE (Value){0,{NODE_VOID,0},0}

#define DEBUG 1

ASTNode RETURN_VOID =  (ASTNode){NODE_RETURN,.data.return_stmt.value=NULL};

GlobalManager * create_global_manager(ASTNode* root){
    MemoryManager * memory_manager = create_memory_manager();
    ScopeManager * scope_manager = create_scope_manager();
    GlobalManager * global_manager = malloc(sizeof (GlobalManager));
    FlowManager *flow_manager = create_flow_manager();

    global_manager->scope_manager = scope_manager;
    global_manager->memory_manager = memory_manager;
    global_manager->flow_manager = flow_manager;
    global_manager->root = root;
    register_all_functions(global_manager);
    return global_manager;
}
void destroy_global_manager(GlobalManager* globalManager){
    destroy_memory_manager(globalManager->memory_manager);
    destroy_scope_manager(globalManager->scope_manager);
    destroy_flow_manager(globalManager->flow_manager);
    free(globalManager);
}

int count_function_defs(GlobalManager* global_manager){
    ASTNode *root = global_manager->root;
    ASTNode *curr;
    int count = 0;
    while (root->type == NODE_TOP_LEVEL_LIST){
        curr = root->data.arg_list.arg;
        if (curr->type == NODE_FUNCTION_DEF)
            count++;
        root = root->data.arg_list.next;
    }
    return count + (root->type == NODE_FUNCTION_DEF ? 1 : 0);
}

void register_all_functions(GlobalManager* global_manager) {
    ASTNode *root = global_manager->root;
    int num_functions = count_function_defs(global_manager);
    global_manager->n_funcs = num_functions;
    global_manager->function_defs = calloc(sizeof(functions_t *),num_functions);
    int counter = 0;
    ASTNode *curr;
    while (root->type == NODE_TOP_LEVEL_LIST){
        curr = root->data.arg_list.arg;
        if (curr->type == NODE_FUNCTION_DEF){
            functions_t *f = malloc(sizeof (functions_t ));
            f->func_def = curr;
            f->name = curr->data.function_def.name;
            global_manager->function_defs[counter++] = f;
        }
        root = root->data.arg_list.next;
    }
    if (root->type == NODE_FUNCTION_DEF) {
        functions_t *f = malloc(sizeof(functions_t));
        f->func_def = root;
        f->name = root->data.function_def.name;
        global_manager->function_defs[counter++] = f;
    }
    if (counter != num_functions){
        printf("WTFFFFFF %d != %d",counter,num_functions);
        exit(1);
    }
}


void enter_new_scope(GlobalManager* global_manager, ASTNode* instruction, ASTNode* next_instruction){
    MemoryManager * memory_manager = global_manager->memory_manager;
    enter_new_scope_scope_manager(global_manager->scope_manager);
    FrameList *new_frame_list = add_new_frame_list(memory_manager->frame_list,memory_manager->stack_pointer);
    memory_manager->frame_list = new_frame_list;
    push_node(global_manager->flow_manager,instruction,next_instruction);
}

void call_function_total(GlobalManager *global_manager,ASTNode* fun_def, ASTNode* fun_call){
    MemoryManager * memory_manager = global_manager->memory_manager;
    ScopeManager * scope_manager = global_manager->scope_manager;
    enter_new_scope(global_manager,fun_call,NULL);
    SymbolTable * from_args = construct_symbol_table_for_function_call(fun_def,memory_manager);
    insert_arguments_from_func_call(global_manager,from_args,fun_def,fun_call);
    call_function(scope_manager,from_args);
}

// Warning: This function also frees the last flow element so if you need it, save its values before
// Also this function should not be used to return from function calls
void exit_scope(GlobalManager *global_manager) {
    MemoryManager *memory_manager = global_manager->memory_manager;
    set_stack_pointer_to_curr_frame_pointer(memory_manager);
    memory_manager->frame_list = return_to_last_frame(memory_manager->frame_list);
    pop_last(global_manager->flow_manager);
    exit_scope_scope_manager(global_manager->scope_manager);
}

void exit_scope_from_func_call(GlobalManager* global_manager) {
    MemoryManager *memory_manager = global_manager->memory_manager;
    set_stack_pointer_to_curr_frame_pointer(memory_manager);
    memory_manager->frame_list = return_to_last_frame(memory_manager->frame_list);
    pop_flow_until_last_func_call(global_manager->flow_manager);
    return_to_prev_function(global_manager->scope_manager);
}

ValueOrAddress return_function_total(GlobalManager* global_manager,ASTNode* return_value, full_type_t return_type){
    ValueOrAddress ret_value;
    if (return_value == NULL){
        ret_value = (ValueOrAddress){{1,{NODE_VOID,0},0},0,-1};
    }else{
        ret_value = eval_expr(global_manager,return_value);
    }
    exit_scope_from_func_call(global_manager);
    if (ret_value.value.type.type != return_type.type || ret_value.value.type.n_pointers != return_type.n_pointers){
        error_out(return_value,"Returning wrong type from function\n");
        print_type(ret_value.value.type,1);
        printf("instead of ");
        print_type(return_type,1);
        exit(1);
    }
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
             new_value = (Value){0,{res.value.type.type,res.value.type.n_pointers+1},res.address};
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

int check_type_compatibility(full_type_t left, full_type_t right, const char* op_name) {
    if (left.n_pointers > 0 || right.n_pointers > 0) {
        // Specific pointer operation rules
        if (strcmp(op_name, "SUB") == 0) {
            if (left.n_pointers > 0 && right.n_pointers > 0) {
                // Subtraction of two pointers of same type is allowed
                return left.type == right.type;
            }
            if (left.n_pointers > 0 && right.type == NODE_INT) {
                // Pointer - integer is allowed
                return 1;
            }
        }
        return 0;
    }
    return left.type == right.type;
}

ValueOrAddress eval_binary_op(GlobalManager* global_manager,ASTNode* binary_op) {
    error_on_wrong_node(NODE_BINARY_OP, binary_op->type, "eval_binary_op");
    ASTNode *left_operand = binary_op->data.binary.left;
    ASTNode *right_operand = binary_op->data.binary.right;
    ValueOrAddress val_left = eval_expr(global_manager, left_operand);
    ValueOrAddress val_right = eval_expr(global_manager, right_operand);
    full_type_t type_left = val_left.value.type, type_right = val_right.value.type;
    Value ret_value;
    if ((type_right.type == NODE_VOID && type_right.n_pointers == 0) ||
        (type_left.type == NODE_VOID && type_left.n_pointers == 0))
        error_out(binary_op, "Cannot do a binary operation with a void value.");
    switch (binary_op->data.binary.operator) {
        case NODE_ADD:
            if (type_left.n_pointers > 0 && type_right.n_pointers > 0)
                error_out(binary_op, "Cannot add two pointers");
            if ((type_left.n_pointers > 0 && type_right.type != NODE_INT) ||
                (type_right.n_pointers > 0 && type_left.type != NODE_INT))
                error_out(binary_op, "Cannot add a pointer with a non-integer value");
            if (type_left.n_pointers > 0) {
                int size = type_size(&type_left);
                int new_addr = size * val_right.value.value.i + val_left.value.value.i;
                ret_value = (Value) {0, type_left, .value.i=new_addr};
            } else if (type_right.n_pointers > 0) {
                int size = type_size(&type_right);
                int new_addr = size * val_right.value.value.i + val_left.value.value.i;
                ret_value = (Value) {0, type_left, .value.i=new_addr};
            } else {
                if (type_left.type == type_right.type) {
                    ret_value = (Value) {0, type_left, 0};
                    switch (type_left.type) {
                        case NODE_INT:
                            ret_value.value.i = val_left.value.value.i + val_right.value.value.i;
                            break;
                        case NODE_FLOAT:
                            ret_value.value.f = val_left.value.value.f + val_right.value.value.f;
                            break;
                        case NODE_CHAR:
                            ret_value.value.c = (char) (val_left.value.value.c + val_right.value.value.c);
                            break;
                    }
                } else {
                    error_out(binary_op, "cannot add two values of different types");
                }
            }
            return (ValueOrAddress) {ret_value, 0, -1};
        case NODE_SUB:
            if (!check_type_compatibility(type_left, type_right, "SUB"))
                error_out(binary_op, "Invalid types for subtraction");

            if (type_left.n_pointers > 0) {
                if (type_right.n_pointers > 0) {
                    // Pointer - Pointer (should return integer difference)
                    ret_value = (Value) {0, (full_type_t) {NODE_INT, 0}, 0};
                    ret_value.value.i = val_left.value.value.i - val_right.value.value.i;
                } else {
                    // Pointer - Integer
                    int size = type_size(&type_left);
                    int new_addr = val_left.value.value.i - size * val_right.value.value.i;
                    ret_value = (Value) {0, type_left, .value.i = new_addr};
                }
            } else {
                ret_value = (Value) {0, type_left, 0};
                switch (type_left.type) {
                    case NODE_INT:
                        ret_value.value.i = val_left.value.value.i - val_right.value.value.i;
                        break;
                    case NODE_FLOAT:
                        ret_value.value.f = val_left.value.value.f - val_right.value.value.f;
                        break;
                    case NODE_CHAR:
                        ret_value.value.c = (char) (val_left.value.value.c - val_right.value.value.c);
                        break;
                    default:
                        error_out(binary_op, "Unsupported type for subtraction");
                }
            }
            return (ValueOrAddress) {ret_value, 0, -1};

        case NODE_MUL:
            if (!check_type_compatibility(type_left, type_right, "MUL"))
                error_out(binary_op, "Cannot multiply values of different types");

            if (type_left.n_pointers > 0 || type_right.n_pointers > 0)
                error_out(binary_op, "Cannot multiply pointers");

            ret_value = (Value) {0, type_left, 0};
            switch (type_left.type) {
                case NODE_INT:
                    ret_value.value.i = val_left.value.value.i * val_right.value.value.i;
                    break;
                case NODE_FLOAT:
                    ret_value.value.f = val_left.value.value.f * val_right.value.value.f;
                    break;
                case NODE_CHAR:
                    ret_value.value.c = (char) (val_left.value.value.c * val_right.value.value.c);
                    break;
                default:
                    error_out(binary_op, "Unsupported type for multiplication");
            }
            return (ValueOrAddress) {ret_value, 0, -1};

        case NODE_DIV:
            if (!check_type_compatibility(type_left, type_right, "DIV"))
                error_out(binary_op, "Cannot divide values of different types");

            if (type_left.n_pointers > 0 || type_right.n_pointers > 0)
                error_out(binary_op, "Cannot divide pointers");

            // Check for division by zero
            switch (type_left.type) {
                case NODE_INT:
                    if (val_right.value.value.i == 0)
                        error_out(binary_op, "Division by zero");
                    ret_value = (Value) {0, type_left, .value.i = val_left.value.value.i / val_right.value.value.i};
                    break;
                case NODE_FLOAT:
                    if (val_right.value.value.f == 0.0)
                        error_out(binary_op, "Division by zero");
                    ret_value = (Value) {0, type_left, .value.f = val_left.value.value.f / val_right.value.value.f};
                    break;
                case NODE_CHAR:
                    if (val_right.value.value.c == 0)
                        error_out(binary_op, "Division by zero");
                    ret_value = (Value) {0, type_left, .value.c = (char) (val_left.value.value.c /
                                                                          val_right.value.value.c)};
                    break;
                default:
                    error_out(binary_op, "Unsupported type for division");
            }
            return (ValueOrAddress) {ret_value, 0, -1};

        case NODE_MOD:
            if (type_left.n_pointers > 0 || type_right.n_pointers > 0)
                error_out(binary_op, "Cannot use modulo with pointers");

            if (type_left.type != NODE_INT || type_right.type != NODE_INT)
                error_out(binary_op, "Modulo operation only supported for integers");

            if (val_right.value.value.i == 0)
                error_out(binary_op, "Modulo by zero");

            ret_value = (Value) {0, type_left, .value.i = val_left.value.value.i % val_right.value.value.i};
            return (ValueOrAddress) {ret_value, 0, -1};

        case NODE_SHIFT_LEFT:
        case NODE_SHIFT_RIGHT:
            if (type_left.n_pointers > 0 || type_right.n_pointers > 0)
                error_out(binary_op, "Cannot use shift operations with pointers");

            if (type_left.type != NODE_INT || type_right.type != NODE_INT)
                error_out(binary_op, "Shift operations only supported for integers");

            ret_value = (Value) {0, type_left, 0};
            if (binary_op->data.binary.operator == NODE_SHIFT_LEFT)
                ret_value.value.i = val_left.value.value.i << val_right.value.value.i;
            else
                ret_value.value.i = val_left.value.value.i >> val_right.value.value.i;
            return (ValueOrAddress) {ret_value, 0, -1};

        case NODE_BITWISE_AND:
        case NODE_BITWISE_OR:
        case NODE_BITWISE_XOR:
            if (type_left.n_pointers > 0 || type_right.n_pointers > 0)
                error_out(binary_op, "Cannot use bitwise operations with pointers");

            if (type_left.type != NODE_INT || type_right.type != NODE_INT)
                error_out(binary_op, "Bitwise operations only supported for integers");

            ret_value = (Value) {0, type_left, 0};
            switch (binary_op->data.binary.operator) {
                case NODE_BITWISE_AND:
                    ret_value.value.i = val_left.value.value.i & val_right.value.value.i;
                    break;
                case NODE_BITWISE_OR:
                    ret_value.value.i = val_left.value.value.i | val_right.value.value.i;
                    break;
                case NODE_BITWISE_XOR:
                    ret_value.value.i = val_left.value.value.i ^ val_right.value.value.i;
                    break;
            }
            return (ValueOrAddress) {ret_value, 0, -1};

        case NODE_LOGICAL_AND:
        case NODE_LOGICAL_OR:
            // Convert values to boolean first
            int left_bool = val_left.value.value.i != 0;
            int right_bool = val_right.value.value.i != 0;

            ret_value = (Value) {0, (full_type_t) {NODE_INT, 0}, 0};
            if (binary_op->data.binary.operator == NODE_LOGICAL_AND)
                ret_value.value.i = left_bool && right_bool;
            else
                ret_value.value.i = left_bool || right_bool;
            return (ValueOrAddress) {ret_value, 0, -1};

        case NODE_IS_EQ:
        case NODE_NOT_EQ:
            ret_value = (Value) {0, (full_type_t) {NODE_INT, 0}, 0};
            if (type_left.n_pointers != type_right.n_pointers || type_left.type != type_right.type)
                ret_value.value.i = 0;
            else {
                if (type_left.n_pointers > 0) {
                    ret_value.value.i = (val_left.value.value.i == val_right.value.value.i);
                }
                switch (type_left.type) {
                    case NODE_INT:
                        ret_value.value.i = (val_left.value.value.i == val_right.value.value.i);
                        break;
                    case NODE_FLOAT:
                        ret_value.value.i = (val_left.value.value.f == val_right.value.value.f);
                        break;
                    case NODE_CHAR:
                        ret_value.value.i = (val_left.value.value.c == val_right.value.value.c);
                        break;
                    default:
                        ret_value.value.i = 0;
                }
            }
            if (binary_op->data.binary.operator == NODE_NOT_EQ)
                ret_value.value.i = !ret_value.value.i;
            return (ValueOrAddress) {ret_value, 0, -1};

        case NODE_GT:
        case NODE_LT:
        case NODE_GEQ:
        case NODE_LEQ:
            if (type_left.n_pointers > 0 || type_right.n_pointers > 0)
                error_out(binary_op, "Cannot compare pointers with these operators");

            ret_value = (Value) {0, (full_type_t) {NODE_INT, 0}, 0};
            switch (type_left.type) {
                case NODE_INT:
                    switch (binary_op->data.binary.operator) {
                        case NODE_GT:
                            ret_value.value.i = val_left.value.value.i > val_right.value.value.i;
                            break;
                        case NODE_LT:
                            ret_value.value.i = val_left.value.value.i < val_right.value.value.i;
                            break;
                        case NODE_GEQ:
                            ret_value.value.i = val_left.value.value.i >= val_right.value.value.i;
                            break;
                        case NODE_LEQ:
                            ret_value.value.i = val_left.value.value.i <= val_right.value.value.i;
                            break;
                    }
                    break;
                case NODE_FLOAT:
                    switch (binary_op->data.binary.operator) {
                        case NODE_GT:
                            ret_value.value.i = val_left.value.value.f > val_right.value.value.f;
                            break;
                        case NODE_LT:
                            ret_value.value.i = val_left.value.value.f < val_right.value.value.f;
                            break;
                        case NODE_GEQ:
                            ret_value.value.i = val_left.value.value.f < val_right.value.value.f;
                            break;
                        case NODE_LEQ:
                            ret_value.value.i = val_left.value.value.f <= val_right.value.value.f;
                            break;
                    }
                case NODE_CHAR:
                    switch (binary_op->data.binary.operator) {
                        case NODE_GT:
                            ret_value.value.i = val_left.value.value.f > val_right.value.value.f;
                            break;
                        case NODE_LT:
                            ret_value.value.i = val_left.value.value.f < val_right.value.value.f;
                            break;
                        case NODE_GEQ:
                            ret_value.value.i = val_left.value.value.f < val_right.value.value.f;
                            break;
                        case NODE_LEQ:
                            ret_value.value.i = val_left.value.value.f <= val_right.value.value.f;
                            break;
                    }
            }
            return (ValueOrAddress){ret_value,0,-1};
    }
}

ValueOrAddress eval_assignment(GlobalManager * global_manager,ASTNode * ast_node) {
    error_on_wrong_node(NODE_ASSIGNMENT,ast_node->type,"eval_assignment");
    ValueOrAddress left = eval_expr(global_manager,ast_node->data.assignment.left);
    if (!left.has_address) error_out(ast_node,"cannot assign to non-addressable value");
    assign_operator assign = ast_node->data.assignment.operator;
    ValueOrAddress right = eval_expr(global_manager,ast_node->data.assignment.right);
    if (left.value.type.type != right.value.type.type ||
        left.value.type.n_pointers != right.value.type.n_pointers)
        error_out(ast_node,"Cannot assign a value to another of different type");
    Value ret_value = {0};
    void *raw_ptr = get_raw_ptr_for_var(global_manager->memory_manager,left.address);
    ret_value.type = left.value.type;
    // Helper macro for basic assignment types to reduce repetition
#define HANDLE_NUMERIC_ASSIGNMENT(type, current_val, right_val) \
        do { \
            type* ptr = (type*)raw_ptr; \
            switch (assign) { \
                case NODE_ASSIGN:       *ptr = right_val; break; \
                case NODE_ADD_ASSIGN:   *ptr += right_val; break; \
                case NODE_SUB_ASSIGN:   *ptr -= right_val; break; \
                case NODE_MUL_ASSIGN:   *ptr *= right_val; break; \
                case NODE_DIV_ASSIGN:   \
                    if (right_val == 0) error_out(ast_node, "Division by zero"); \
                    *ptr /= right_val; \
                    break; \
                case NODE_MOD_ASSIGN:   \
                    if (right_val == 0) error_out(ast_node, "Modulo by zero"); \
                    *ptr %= right_val; \
                    break; \
                case NODE_SHIFT_LEFT_ASSIGN:  *ptr <<= right_val; break; \
                case NODE_SHIFT_RIGHT_ASSIGN: *ptr >>= right_val; break; \
                case NODE_AND_ASSIGN:   *ptr &= right_val; break; \
                case NODE_OR_ASSIGN:    *ptr |= right_val; break; \
                case NODE_XOR_ASSIGN:   *ptr ^= right_val; break; \
                default: error_out(ast_node, "Unsupported assignment operator"); \
            } \
            ret_value.value.current_val = *ptr; \
        } while(0)

// Helper macro for float assignments (more restricted)
#define HANDLE_FLOAT_ASSIGNMENT(type, current_val, right_val) \
        do { \
            type* ptr = (type*)raw_ptr; \
            switch (assign) { \
                case NODE_ASSIGN:       *ptr = right_val; break; \
                case NODE_ADD_ASSIGN:   *ptr += right_val; break; \
                case NODE_SUB_ASSIGN:   *ptr -= right_val; break; \
                case NODE_MUL_ASSIGN:   *ptr *= right_val; break; \
                case NODE_DIV_ASSIGN:   \
                    if (right_val == 0.0f) error_out(ast_node, "Division by zero"); \
                    *ptr /= right_val; \
                    break; \
                case NODE_MOD_ASSIGN: \
                case NODE_SHIFT_LEFT_ASSIGN: \
                case NODE_SHIFT_RIGHT_ASSIGN: \
                case NODE_AND_ASSIGN: \
                case NODE_OR_ASSIGN: \
                case NODE_XOR_ASSIGN: \
                    error_out(ast_node, "Bitwise and shift operations not supported for float"); \
                default: error_out(ast_node, "Unsupported assignment operator for float"); \
            } \
            ret_value.value.current_val = *ptr; \
        } while(0)
    // Pointer assignments are special
    if (left.value.type.n_pointers > 0) {
        // For pointers, only basic assignment is allowed
        if (assign != NODE_ASSIGN)
            error_out(ast_node, "Only basic assignment is allowed for pointers");

        *(int*)raw_ptr = right.value.value.i;
        ret_value.value.i = right.value.value.i;
        return (ValueOrAddress){ret_value, 0, -1};
    }

    // Handle different numeric types
    switch (left.value.type.type) {
        case NODE_INT:
            HANDLE_NUMERIC_ASSIGNMENT(int, i, right.value.value.i);
            break;
        case NODE_FLOAT:
            HANDLE_FLOAT_ASSIGNMENT(float, f, right.value.value.f);
            break;
        case NODE_CHAR:
            HANDLE_NUMERIC_ASSIGNMENT(char, c, right.value.value.c);
            break;
        default:
            error_out(ast_node, "Unsupported type for assignment");
    }

    return (ValueOrAddress){ret_value, 0, -1};
}
ASTNode * find_function(GlobalManager* global_manager, const char* func_name){
    for (int i = 0; i < global_manager->n_funcs; ++i) {
        if (strcmp(func_name,global_manager->function_defs[i]->name) == 0){
            return global_manager->function_defs[i]->func_def;
        }
    }
    fprintf(stderr,"[ERROR] Function %s not defined\n",func_name);
    exit(1);
}

ASTNode *next_instruction(ASTNode* curr_instr_list){
    ASTNode *next_instr;
    if (curr_instr_list->type == NODE_INSTRUCTION_LIST){
        next_instr = curr_instr_list->data.arg_list.next;
    }else{
        next_instr = NULL;
    }
    return next_instr;
}

ASTNode* return_void(ASTNode* fun_def){
    full_type_t func_ret_type = *fun_def->data.function_def.type;

    if (func_ret_type.type != NODE_VOID || func_ret_type.n_pointers != 0){
        char s[300] = {};
        snprintf(s,299,"Function %s misses a return with non-void return type",fun_def->data.function_def.name);
        error_out(fun_def, s);
    }
    return &RETURN_VOID;
}
// return 1 if should return void, 0 otherwise
int handle_end_of_block(GlobalManager* globalManager, ASTNode** curr_instr_list_ptr){
    FlowElement *last_flow_elem = NULL;

    last_flow_elem = get_last(globalManager->flow_manager);
    ASTNode *next = last_flow_elem->next_node;
    ASTNode *current = last_flow_elem->ast_node;
    FlowEnum current_flow_type = last_flow_elem->flow_type;
    // If a function was called for the block, just return void since there was no return value
    if (current_flow_type == FLOW_FUNC_CALL){
        // just make sure that we are allowed to return void
        return 1;
    }
    // we can exit scope since we saved all information
    exit_scope(globalManager);

    // We reached the end of out block
    while (next == NULL || is_loop(current_flow_type)) {
        int already_exited = 0;
        if (is_loop(current_flow_type)) {
            // exit scope to evaluate value with only outer for / while
            switch (current_flow_type) {
                case FLOW_WHILE:
                case FLOW_DO_WHILE:
                    ValueOrAddress condition = eval_expr(globalManager, current->data.while_loop.condition);
                    if (is_zero(&condition)) {
                        if (next != NULL){
                            *curr_instr_list_ptr = next;
                            return 0;
                        }
                        already_exited = 1;
                        break;
                    }
                    *curr_instr_list_ptr = current->data.while_loop.body->data.compound_stmt.block;
                    enter_new_scope(globalManager,current,next);
                    return 0;
                case FLOW_FOR:
                    // evaluate effect (i.e. i++ 99% of time)
                    ValueOrAddress r = eval_expr(globalManager,current->data.for_loop.effect);
                    // recompute condition
                    condition = eval_expr(globalManager,current->data.for_loop.condition);
                    if (is_zero(&condition)){
                        // exit 2nd scope of for loop
                        exit_scope(globalManager);
                        if (next != NULL){
                            *curr_instr_list_ptr = next;
                            return 0;
                        }
                        already_exited = 1;
                        break;
                    }
                    *curr_instr_list_ptr = current->data.for_loop.body->data.compound_stmt.block;
                    enter_new_scope(globalManager,current,next);
                    return 0;
                default:
                    error_out(*curr_instr_list_ptr, "impossible error");
            }
        }else{
            last_flow_elem = get_last(globalManager->flow_manager);
            current_flow_type = last_flow_elem->flow_type;
            current = last_flow_elem->ast_node;
            next = last_flow_elem->next_node;
            // If a function was called for the block, just return void since there was no return value
            if (!already_exited){
                if (current_flow_type == FLOW_FUNC_CALL){return 1;}
                exit_scope(globalManager);
            }
        }
    }
    *curr_instr_list_ptr = next;
    return 0;
}


ASTNode* execute_function(GlobalManager *globalManager,ASTNode *fun_def){
    error_on_wrong_node(NODE_FUNCTION_DEF,fun_def->type,"execute_function");
    ASTNode* body = fun_def->data.function_def.body->data.compound_stmt.block;
    if (body == NULL){
        return return_void(fun_def);
    }
    ASTNode *curr_instr_list = body;
    ASTNode *curr_instr = NULL;
    while (1){
        // curr_instr_list can be NULL if body of compound_stmt is NULL
        // This is when you reach the end of a block
        if (curr_instr == curr_instr_list || curr_instr_list == NULL){
            int should_return = handle_end_of_block(globalManager,&curr_instr_list);
            if (should_return){
                return return_void(fun_def);
            }
        }
        curr_instr =  curr_instr_list->type == NODE_INSTRUCTION_LIST? curr_instr_list->data.arg_list.arg: curr_instr_list;
        if (DEBUG){
            printf("\n\nCURR INSTR \n=============\n");
            print_ast(curr_instr,0);
            printf("?=============\n");
            print_global_manager_state(globalManager);
            printf("--------------------\n\n");
        }
        //sleep(1);


        switch (curr_instr->type){
            case NODE_IDENTIFIER:
            case NODE_CONSTANT:
            case NODE_ARRAY_ACCESS:
            case NODE_FUNCTION_CALL:
            case NODE_UNARY_OP:
            case NODE_BINARY_OP:
            case NODE_ASSIGNMENT:
                eval_expr(globalManager,curr_instr);
                if (curr_instr_list->type == NODE_INSTRUCTION_LIST){
                    curr_instr_list = curr_instr_list->data.arg_list.next;
                }
                break;
            case NODE_BLOCK:
                ASTNode *next_instr = next_instruction(curr_instr_list);
                curr_instr_list = curr_instr->data.compound_stmt.block;
                enter_new_scope(globalManager,curr_instr,next_instr);
                break;
            case NODE_PARAM_LIST:
            case NODE_INSTRUCTION_LIST:
            case NODE_TOP_LEVEL_LIST:
            case NODE_DECLARATOR_LIST:
            case NODE_INIT_LIST:
            case NODE_ARG_LIST:
            case NODE_PARAM_DECLARATION:
            case NODE_ARRAY_DECLARATION:
            case NODE_FUNCTION_DEF:
                error_out(curr_instr,"Should not be here");
                break;
            case NODE_DECLARATION:
                ValueOrAddress v = eval_expr(globalManager,curr_instr->data.declaration.value);
                declare_new_variable(globalManager,curr_instr->data.declaration.name->data.identifier.name,v.value);
                if (curr_instr_list->type == NODE_INSTRUCTION_LIST)
                    curr_instr_list = curr_instr_list->data.arg_list.next;
                break;
            case NODE_IF:
                ValueOrAddress val = eval_expr(globalManager,curr_instr->data.if_stmt.condition);
                if (!is_zero(&val)){
                    // Set next instruction for control flow
                    next_instr = next_instruction(curr_instr_list);
                    curr_instr_list = curr_instr->data.if_stmt.if_body->data.compound_stmt.block;
                    enter_new_scope(globalManager, curr_instr,next_instr);
                }else {
                    ASTNode* else_node = curr_instr->data.if_stmt.else_body;
                    while (else_node != NULL && else_node->type == NODE_IF){
                        val = eval_expr(globalManager,else_node->data.if_stmt.condition);
                        if(!is_zero(&val)){
                            next_instr = next_instruction(curr_instr_list);
                            enter_new_scope(globalManager,curr_instr,next_instr);
                            curr_instr_list = curr_instr->data.if_stmt.if_body->data.compound_stmt.block;
                            break;
                        }else{
                            else_node = else_node->data.if_stmt.else_body;
                        }
                    }
                    if (else_node == NULL || else_node->type != NODE_IF){
                        if (else_node){
                            next_instr = next_instruction(curr_instr_list);
                            curr_instr_list = else_node->data.compound_stmt.block;
                            enter_new_scope(globalManager,curr_instr,next_instr);
                        }else{
                            // no condition was true and no empty-else => skip if
                            if (curr_instr_list->type == NODE_INSTRUCTION_LIST)
                                curr_instr_list = curr_instr_list->data.arg_list.next;
                        }
                    }
                }
                break;
            case NODE_WHILE:
                ValueOrAddress condition = eval_expr(globalManager,curr_instr->data.while_loop.condition);
                if (is_zero(&condition)) { break; }
                // no break because same
            case NODE_DO_WHILE:
                next_instr = next_instruction(curr_instr_list);
                curr_instr_list = curr_instr->data.while_loop.body->data.compound_stmt.block;
                enter_new_scope(globalManager,curr_instr,next_instr);
                break;
            case NODE_FOR:
                // scope for variable declaration of init
                ASTNode block_of_init = (ASTNode){NODE_BLOCK,.data.compound_stmt.block=curr_instr->data.for_loop.init,0,0};
                enter_new_scope(globalManager,&block_of_init,NULL);
                ASTNode *init = curr_instr->data.for_loop.init;
                // the init declares a variable
                if (init->type == NODE_DECLARATOR_LIST || init->type == NODE_DECLARATION){
                    ASTNode *decls = init;
                    ASTNode *curr_decl;
                    while (decls->type == NODE_DECLARATOR_LIST){
                        curr_decl = decls->data.arg_list.arg;
                        ValueOrAddress v = eval_expr(globalManager,curr_decl->data.declaration.value);
                        declare_new_variable(globalManager,curr_decl->data.declaration.name->data.identifier.name,v.value);
                        decls = decls->data.arg_list.next;
                    }
                    ValueOrAddress v = eval_expr(globalManager,decls->data.declaration.value);
                    declare_new_variable(globalManager,decls->data.declaration.name->data.identifier.name,v.value);
                }else{
                    eval_expr(globalManager,init);
                }
                condition = eval_expr(globalManager,curr_instr->data.for_loop.condition);
                if (is_zero(&condition)) { exit_scope(globalManager);break; }
                next_instr = next_instruction(curr_instr_list);
                enter_new_scope(globalManager,curr_instr,next_instr);
                curr_instr_list = curr_instr->data.for_loop.body->data.compound_stmt.block;
                break;
            case NODE_CONTINUE:
                curr_instr_list = NULL;
                continue;
            case NODE_BREAK:
                FlowElement *flow_elem = pop_until_last_loop(globalManager->flow_manager);
                curr_instr_list = flow_elem->next_node;
                // exit the scope from the init of the for loop
                if (flow_elem->flow_type == FLOW_FOR)
                    exit_scope(globalManager);
                // avoid UAF
                flow_elem = NULL;
                exit_scope(globalManager);
                break;
            case NODE_RETURN:
                return curr_instr->data.return_stmt.value;
        }
    }
}
void execute_code(GlobalManager* globalManager){
    ASTNode *main = find_function(globalManager,"main");
    ASTNode *func_call = create_function_call(create_identifier("main"),NULL);
    call_function_total(globalManager,main,func_call);
    print_global_manager_state(globalManager);
    ASTNode * ret = execute_function(globalManager,main);
    ValueOrAddress final_result = return_function_total(globalManager,ret,*main->data.function_def.type);
    printf("Process returned with value : \n");
    print_value(final_result.value);
}
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
                error_out(ast_node,"Tried to access an array on a non-pointer variable");
            }
            ValueOrAddress index = eval_expr(global_manager,ast_node->data.array_access.index);
            if ((index.value.type.type != NODE_INT && index.value.type.type != NODE_CHAR)||
                index.value.type.n_pointers > 0){
                fprintf(stderr,"[ERROR] Array index must be an integer or char (l. %d)\n",ast_node->line_number);
                exit(1);
            }
            int arr_index = index.value.type.type == NODE_CHAR ? index.value.value.c: index.value.value.i;
            full_type_t new_type = {array.value.type.type,array.value.type.n_pointers - 1};
            int final_ptr = array.value.value.i + arr_index * type_size(&new_type);
            void* element_ptr = get_raw_ptr_for_var(global_manager->memory_manager,final_ptr);
            Value result = {0,new_type,0};
            add_correct_value_type_to_v(new_type,element_ptr,&result);
            ValueOrAddress ret = {result,1,final_ptr};
            return ret;
        case NODE_FUNCTION_CALL:
            char* fun_name = ast_node->data.function_call.function->data.identifier.name;
            ASTNode * func_def = find_function(global_manager,fun_name);
            call_function_total(global_manager,func_def,ast_node);
            ASTNode* ret_call = execute_function(global_manager,func_def);
            ValueOrAddress func_call_result = return_function_total(global_manager,ret_call,*func_def->data.function_def.type);
            return func_call_result;
        case NODE_UNARY_OP:
            return eval_unary_op(global_manager,ast_node);
        case NODE_INIT_LIST:
            fprintf(stderr,"init list not implemented");
            exit(1);
            // TODO init the list and returns the pointer
            break;
        case NODE_BINARY_OP:
            return eval_binary_op(global_manager,ast_node);
        case NODE_ASSIGNMENT:
            return eval_assignment(global_manager,ast_node);
        case NODE_PARAM_LIST:
        case NODE_BLOCK:
        case NODE_ARG_LIST:
        case NODE_DECLARATOR_LIST:
        case NODE_DECLARATION:
        case NODE_TOP_LEVEL_LIST:
        case NODE_ARRAY_DECLARATION:
        case NODE_PARAM_DECLARATION:
        case NODE_IF:
        case NODE_WHILE:
        case NODE_DO_WHILE:
        case NODE_FOR:
        case NODE_CONTINUE:
        case NODE_BREAK:
        case NODE_RETURN:
        case NODE_FUNCTION_DEF:
        case NODE_INSTRUCTION_LIST:
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
    if(!args) return;
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

void print_memory(GlobalManager* global_manager) {
    if (global_manager == NULL || global_manager->memory_manager == NULL) return;

    printf("  ║ 💾 Memory Dump                ║\n");
    printf("  ╠═══════════════════════════════╣\n");

    int stack_pointer = global_manager->memory_manager->stack_pointer;
    unsigned char* base_of_stack = (unsigned char*)(global_manager->memory_manager->base_of_stack);

    if (stack_pointer == 0) {
        printf("  ║   🔷 No Memory Allocated      ║\n");
        return;
    }

    printf("  ║   🔢 Bytes: %d                 ║\n", stack_pointer);
    printf("  ║   📊 Memory Contents:         ║\n");

    printf("  ║   ");
    for (int i = 0; i < stack_pointer; ++i) {
        if (i > 0 && i % 16 == 0) {
            printf("\n  ║   ");
        }
        printf("%02x ", base_of_stack[i]);
    }
    printf("\n");
}

void print_global_manager_state(GlobalManager* global_manager){
    fflush(stdout);
    printf("\n\n");
    printf("  ╔═══════════════════════════════╗\n");
    printf("  ║  🌐 Global Manager State 🌐   ║\n");
    printf("  ╠═══════════════════════════════╣\n");
    printf("  ║ 📊 Stack Pointer    : %6d  ║\n", global_manager->memory_manager->stack_pointer);
    printf("  ║ 🖼️  Frame Pointer    : %6d  ║\n", global_manager->memory_manager->frame_list->current_frame_pointer);
    printf("  ║ 💾 Memory Size      : %6d  ║\n", global_manager->memory_manager->size_memory);
    printf("  ║ 📚 Num Tables    : %6d     ║\n", num_symbol_tables(global_manager->scope_manager));
    printf("  ╠═══════════════════════════════╣\n");
    print_symbol_tables(global_manager->scope_manager);
    print_current_flow(global_manager->flow_manager);
    print_memory(global_manager);
    printf("  ╚══════════════════════════════╝\n");
    fflush(stdout);
}