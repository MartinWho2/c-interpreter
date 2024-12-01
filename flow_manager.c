#include <stdio.h>
#include "flow_manager.h"

FlowManager *create_flow_manager(){
    FlowManager *flow_manager = malloc(sizeof(FlowManager) );
    flow_manager->flow_element_stack = calloc(sizeof(FlowManager),FLOW_STACK_SIZE);
    flow_manager->stack_pointer = 0;
    return flow_manager;
}
void destroy_flow_manager(FlowManager* flowManager){
    for (int i = (flowManager->stack_pointer)-1; i >= 0; --i) {
        free(flowManager->flow_element_stack[i]);
    }
    free(flowManager->flow_element_stack);
    free(flowManager);
}

// Called to add 
void push_node(FlowManager *flow_manager,ASTNode* node, ASTNode* next_node){
    FlowElement *flow_element = malloc(sizeof(FlowElement));
    flow_element->ast_node = node;
    flow_element->next_node = next_node;
    switch (node->type) {
        case NODE_IF:
            if (node->data.if_stmt.else_body)
                flow_element->flow_type = FLOW_IF_ELSE;
            else
                flow_element->flow_type = FLOW_IF;
            break;
        case NODE_WHILE:
            flow_element->flow_type = FLOW_WHILE;
            break;
        case NODE_DO_WHILE:
            flow_element->flow_type = FLOW_DO_WHILE;
            break;
        case NODE_FOR:
            flow_element->flow_type = FLOW_FOR;
            break;
        case NODE_FUNCTION_CALL:
            flow_element->flow_type = FLOW_FUNC_CALL;
            break;
        case NODE_BLOCK:
            flow_element->flow_type = FLOW_BLOCK;
            break;
        default:
            error_out(node,"Expected a node that involves flow control");
    }
    flow_manager->flow_element_stack[(flow_manager->stack_pointer)++] = flow_element;
}

// Called when return
void pop_flow_until_last_func_call(FlowManager* flow_manager){
    int stack_pointer = flow_manager->stack_pointer-1;
    if (stack_pointer < 0){
        fprintf(stderr,"Popping all control flow without finding function call ??");
        exit(1);
    }
    FlowElement * flow_element = flow_manager->flow_element_stack[stack_pointer];
    while (flow_element->flow_type != FLOW_FUNC_CALL){
        if (stack_pointer == 0){
            fprintf(stderr,"Popping all control flow without finding function call ??");
            exit(1);
        }
        free(flow_element);
        flow_element = flow_manager->flow_element_stack[stack_pointer--];
    }
    flow_manager->stack_pointer = stack_pointer--;
    free(flow_element);

}

// Called when reaching the end of a block which involves looping
ASTNode *get_last_loop(FlowManager* flow_manager){
    int stack_pointer = flow_manager->stack_pointer-1;
    FlowElement *flow_element = flow_manager->flow_element_stack[stack_pointer];
    while (flow_element->flow_type == FLOW_IF ||
    flow_element->flow_type == FLOW_IF_ELSE ||
    flow_element->flow_type == FLOW_BLOCK){
        if (stack_pointer == 0) {
            fprintf(stderr, "Popping all control flow without finding loop ??");
            exit(1);
        }
        stack_pointer--;
        flow_element = flow_manager->flow_element_stack[stack_pointer];
    }
    if (flow_element->flow_type != FLOW_WHILE &&
        flow_element->flow_type != FLOW_DO_WHILE &&
        flow_element->flow_type != FLOW_FOR){
        fprintf(stderr,"Cannot break or continue if there was no 'for' and no 'while'");
        exit(1);
    }
    return flow_element->ast_node;
}

//
FlowElement *pop_until_last_loop(FlowManager* flow_manager){
    int stack_pointer = flow_manager->stack_pointer-1;
    FlowElement *flow_element = flow_manager->flow_element_stack[stack_pointer];
    while (flow_element->flow_type == FLOW_IF ||
           flow_element->flow_type == FLOW_IF_ELSE ||
           flow_element->flow_type == FLOW_BLOCK){
        if (stack_pointer == 0){
            fprintf(stderr,"Popping all control flow without finding loop ??");
            exit(1);
        }
        stack_pointer--;
        free(flow_element);
        flow_element = flow_manager->flow_element_stack[stack_pointer];
    }
    if (flow_element->flow_type != FLOW_WHILE &&
        flow_element->flow_type != FLOW_DO_WHILE &&
        flow_element->flow_type != FLOW_FOR){
        fprintf(stderr,"Cannot break or continue if there was no 'for' and no 'while'");
        exit(1);
    }
    flow_manager->stack_pointer = stack_pointer;
    return flow_element;

}

FlowElement *get_last(FlowManager* flow_manager){
    return flow_manager->flow_element_stack[flow_manager->stack_pointer-1];
}

void pop_last(FlowManager* flow_manager){
    free(flow_manager->flow_element_stack[--flow_manager->stack_pointer]);
}

int is_loop(FlowEnum flowEnum){
    switch (flowEnum) {
        case FLOW_IF:
        case FLOW_IF_ELSE:
        case FLOW_FUNC_CALL:
        case FLOW_BLOCK:
            return 0;
        default:
            return 1;
    }
}

void print_flow_enum(FlowEnum f){
    switch (f) {
        case FLOW_IF:
            printf("IF\n");
            break;
        case FLOW_IF_ELSE:
            printf("IF ELSE\n");
            break;
        case FLOW_WHILE:
            printf("WHILE\n");
            break;
        case FLOW_DO_WHILE:
            printf("DO WHILE\n");
            break;
        case FLOW_FOR:
            printf("FOR\n");
            break;
        case FLOW_FUNC_CALL:
            printf("FUNC CALL\n");
            break;
        case FLOW_BLOCK:
            printf("BLOCK\n");
            break;
    }
}
void print_current_flow(FlowManager* flow_manager){
    printf("  Flow manager:\n");
    for (int i = 0; i < flow_manager->stack_pointer; ++i) {
        printf("    ");
        print_flow_enum(flow_manager->flow_element_stack[i]->flow_type);
        if (flow_manager->flow_element_stack[i]->next_node){
            printf("    next node type: ");
            print_node_type(flow_manager->flow_element_stack[i]->next_node->type);
        }else{
            printf("    no next node for block\n");
        }
    }
}