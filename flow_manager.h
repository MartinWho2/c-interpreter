#include "ast.h"

#define FLOW_STACK_SIZE 10000

typedef enum {
    FLOW_IF,
    FLOW_IF_ELSE,
    FLOW_WHILE,
    FLOW_DO_WHILE,
    FLOW_FOR,
    FLOW_FUNC_CALL,
    FLOW_BLOCK
} FlowEnum;

typedef struct FlowElement{
    FlowEnum flow_type;
    ASTNode *ast_node;
    ASTNode *next_node;
}FlowElement;

typedef struct FlowManager{
    FlowElement** flow_element_stack;
    int stack_pointer;
} FlowManager;

FlowManager *create_flow_manager();
void destroy_flow_manager(FlowManager* flowManager);

void push_node(FlowManager *flow_manager,ASTNode* node, ASTNode *next_node);
ASTNode *get_last_loop(FlowManager* flow_manager);
void pop_flow_until_last_func_call(FlowManager* flow_manager);
FlowElement *pop_until_last_loop(FlowManager* flow_manager);
void pop_last(FlowManager* flow_manager);
FlowElement *get_last(FlowManager* flow_manager);
int is_loop(FlowEnum flow_enum);

void print_current_flow(FlowManager* flow_manager);