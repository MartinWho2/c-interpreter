#ifndef AST_H
#define AST_H

#include <stdlib.h>

// Node types for our AST
typedef enum {
    NODE_IDENTIFIER,
    NODE_CONSTANT,
    NODE_STRING_LITERAL,
    NODE_ARRAY_ACCESS,
    NODE_FUNCTION_CALL,
    NODE_UNARY_OP,

    NODE_PARAM_LIST,
    NODE_DECLARATOR_LIST,
    NODE_INIT_LIST,
    NODE_ARG_LIST,
    NODE_TOP_LEVEL_LIST,
    NODE_INSTRUCTION_LIST,

    NODE_BLOCK,
    NODE_ARRAY_INIT,
    NODE_CAST,
    NODE_BINARY_OP,
    NODE_ASSIGNMENT,
    NODE_DECLARATION,
    NODE_ARRAY_DECLARATION,
    NODE_PARAM_DECLARATION,
    NODE_IF,
    NODE_WHILE,
    NODE_DO_WHILE,
    NODE_FOR,

    NODE_CONTINUE,
    NODE_BREAK,

    NODE_RETURN,
    NODE_FUNCTION_DEF
    
} NodeType;

// Binary operators
typedef enum {
    NODE_ADD,
    NODE_SUB,
    NODE_MUL,
    NODE_MOD,
    NODE_DIV,
    NODE_SHIFT_RIGHT,
    NODE_SHIFT_LEFT,
    NODE_BITWISE_AND,
    NODE_BITWISE_OR,
    NODE_BITWISE_XOR,
    NODE_LOGICAL_AND,
    NODE_LOGICAL_OR,
    NODE_IS_EQ,
    NODE_NOT_EQ,
    NODE_GT,
    NODE_LT,
    NODE_GEQ,
    NODE_LEQ
} bin_operator;

// Unary operators
typedef enum {
    NODE_ADDRESS,
    NODE_DEREF,
    NODE_PLUS,
    NODE_MINUS,
    NODE_INC_POST,
    NODE_DEC_POST,
    NODE_INC_PRE,
    NODE_DEC_PRE,
    NODE_BITWISE_NOT,
    NODE_LOGICAL_NOT
} un_operator;

// Type for casts
typedef enum {
    NODE_INT,
    NODE_FLOAT,
    NODE_VOID,
    NODE_CHAR
} type_t;

// Full type for declarations
typedef struct full_type_t{
    type_t type;
    int n_pointers;
} full_type_t;

// Type that represents a number with its type
typedef struct Value {
    int is_constant;
    full_type_t type;
    union {int i; float f; char c;} value;
} Value;

typedef struct ValueOrAddress {
    Value value;
    int has_address;
    int address;
} ValueOrAddress;

// Type for assignment
typedef enum {
    NODE_ASSIGN,
    NODE_MUL_ASSIGN,
    NODE_DIV_ASSIGN,
    NODE_MOD_ASSIGN,
    NODE_ADD_ASSIGN,
    NODE_SUB_ASSIGN,
    NODE_SHIFT_LEFT_ASSIGN,
    NODE_SHIFT_RIGHT_ASSIGN,
    NODE_AND_ASSIGN,
    NODE_OR_ASSIGN,
    NODE_XOR_ASSIGN
} assign_operator;



// Generic AST node structure
typedef struct ASTNode {
    NodeType type;
    union {
        // For identifiers
        struct {
            char *name;
        } identifier;

        // For literals/constants
        struct {
            Value* value;
        } constant;

        // For array access
        struct {
            struct ASTNode *array;
            struct ASTNode *index;
        } array_access;

        // For function calls
        struct {
            struct ASTNode *function;
            struct ASTNode *args;
        } function_call;

        // For unary operations
        struct {
            struct ASTNode *operand;
            un_operator operator;
        } unary;

        // For argument lists
        struct {
            struct ASTNode *arg;
            struct ASTNode *next;
        } arg_list;

        struct {
            struct full_type_t *cast_type;
            struct ASTNode* operand;
        } cast;

        // For binary operations
        struct {
            struct ASTNode *left;
            struct ASTNode *right;
            bin_operator operator;  // Stores the token type of the operator
        } binary;
        
        // For assignments
        struct {
            struct ASTNode *left;
            struct ASTNode *right;
            assign_operator operator;
        } assignment;

        // For declarations
        struct {
            struct ASTNode *name;
            struct full_type_t *type;
            struct ASTNode *value;
        } declaration;

        // For array declarations
        struct {
            struct ASTNode *name;
            struct ASTNode *size;
        } array_declaration;

        // For parameter declarations
        struct {
            struct full_type_t *type;
            struct ASTNode *name;
        } param_declaration;

        // For compound statements
        struct {
            struct ASTNode *block;
        } compound_stmt;
        
        // For if statements
        struct {
            struct ASTNode *condition;
            struct ASTNode *if_body;
            struct ASTNode *else_body;
        } if_stmt;
        
        // For while loops and do-while loops
        struct {
            struct ASTNode *condition;
            struct ASTNode *body;
        } while_loop;

        // For for loops
        struct {
            struct ASTNode *init;
            struct ASTNode *condition;
            struct ASTNode *effect;
            struct ASTNode *body;
        } for_loop;
        
        // For continue and break statements
        struct {} jump;
        
        // For return statements
        struct {
            struct ASTNode *value;
        } return_stmt;

        struct{
            char* value;
        } string;

        // For function def
        struct {
            struct full_type_t *type;
            char* name;
            struct ASTNode *parameters;
            struct ASTNode *body;
        } function_def;

        // For array init
        struct{
            struct ASTNode *array;
        } array_init;

    } data;
    
    // Source location for error reporting
    int line_number;
    int column_number;
} ASTNode;

// Function declarations for AST operations
ASTNode* create_identifier(const char* name);
ASTNode* create_constant(Value* value);
ASTNode* create_string_literal(char* string);
ASTNode* create_array_access(ASTNode* array, ASTNode* index);
ASTNode* create_function_call(ASTNode* function, ASTNode* args);
ASTNode* create_un_op(ASTNode* operand, un_operator operator);
ASTNode* create_list(ASTNode* arg, ASTNode* next, NodeType type);
full_type_t * create_type(type_t type, int n_pointers);
ASTNode *create_cast(ASTNode *value, full_type_t* type);
ASTNode* create_bin_op(ASTNode* left, ASTNode* right,bin_operator operator);
ASTNode* create_assignment(ASTNode* left, ASTNode* right, assign_operator operator);
ASTNode* create_declaration(ASTNode* name, full_type_t * full_type, ASTNode* value);
ASTNode* create_array_declaration(ASTNode* name, ASTNode* size);
ASTNode* create_param_declaration(full_type_t * full_type, ASTNode* name);
ASTNode* create_block(ASTNode* block);
ASTNode* create_if_stmt(ASTNode* condition, ASTNode* if_body, ASTNode* else_body);
ASTNode* create_while_loop(ASTNode* condition, ASTNode* body);
ASTNode* create_do_while_loop(ASTNode* condition, ASTNode* body);
ASTNode* create_for_loop(ASTNode* init, ASTNode* condition, ASTNode* effect, ASTNode* body);
ASTNode* create_jump(NodeType type);
ASTNode* create_return_stmt(ASTNode* value);
ASTNode* create_function_def(full_type_t* full_type, char* name, ASTNode* parameters, ASTNode* body);
ASTNode* create_array_init_list(ASTNode* list_init);

void free_ast(ASTNode* node);
void print_ast(ASTNode* node, int indent);
void print_node_type(NodeType node_type);
void print_type(full_type_t full_type, int indent);
void print_val_or_addr(ValueOrAddress v);
void print_value(Value v);

int type_size(full_type_t* full_type);
int is_void_value(Value* v);
int is_zero(ValueOrAddress* v);
size_t list_size(ASTNode* list);

void error_on_wrong_node(NodeType expected, NodeType actual, const char *function_name);
void error_out(ASTNode* current_node, const char* error_message);
#endif