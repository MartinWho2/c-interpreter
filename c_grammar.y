%{
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"

extern int yylex(void);
extern char* yytext;
extern int column;
void yyerror(const char *s){
	fflush(stdout);
	printf("\n%*s\n%*s\n", column, "^", column, s);
}
void modify_types(ASTNode* node, ASTNode* type){
	if (node == NULL){
		return;
	}
	switch (node->type){
		case NODE_DECLARATOR_LIST:
			modify_types(node->data.arg_list.arg,type);
			modify_types(node->data.arg_list.next,type);
			break;
		case NODE_DECLARATION:
			node->data.declaration.type = type;
			break;
	}
	return;
}
// Root of our AST
ASTNode* root = NULL;
%}

/* Declare the type of values in the grammar */
%union {
    ASTNode* node;
    char* string;
    int token;
	ConstInfo* const_info;
	bin_operator bin_op;
	un_operator unary_op;
	assign_operator assign_op;
	type_t type_type;
}

%token <const_info > CONSTANT STRING_LITERAL
%token <string> IDENTIFIER  
%token <token> INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP AND_OP OR_OP
%token <token> MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token <token> SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token <token> XOR_ASSIGN OR_ASSIGN

%token <string> INT FLOAT VOID

%token <string> IF ELSE WHILE DO FOR CONTINUE BREAK RETURN

%type <node> primary_expression postfix_expression argument_expression_list
%type <node> unary_expression
%type <unary_op> unary_operator
%type <node> multiplicative_expression additive_expression
%type <node> shift_expression relational_expression
%type <node> equality_expression and_expression
%type <node> exclusive_or_expression inclusive_or_expression
%type <node> logical_and_expression logical_or_expression
%type <node> assignment_expression
%type <assign_op> assignment_operator
%type <node> declaration
%type <node> init_declarator init_declarator_list
%type <type_type> type_specifier
%type <node> declarator
%type <token> pointer
%type <node> parameter_list
%type <node> parameter_declaration
%type <node> initializer initializer_list
%type <node> statement compound_statement declaration_list
%type <node> statement_list expression_statement
%type <node> selection_statement iteration_statement jump_statement
%type <node> translation_unit external_declaration
%type <node> function_definition
%type <node> type_name

%start translation_unit
%%

primary_expression
	: IDENTIFIER
        {$$ = create_identifier(yytext);}
	| CONSTANT
        {$$ = create_constant($1);}
	| STRING_LITERAL
        {$$ = create_constant($1);}
	| '(' assignment_expression ')'
    	{$$ = $2;}
	;

postfix_expression
	: primary_expression
		{$$ = $1;}
	| postfix_expression '[' assignment_expression ']'
		{$$ = create_array_access($1, $3);}
	| postfix_expression '(' ')'
		{$$ = create_function_call($1, NULL);}
	| postfix_expression '(' argument_expression_list ')'
		{$$ = create_function_call($1, $3);}
	| postfix_expression INC_OP
		{$$ = create_un_op($1,NODE_INC_POST);}
	| postfix_expression DEC_OP
		{$$ = create_un_op($1,NODE_DEC_POST);}
	;

argument_expression_list
	: logical_or_expression
		{$$ = $1;}
	| logical_or_expression ',' argument_expression_list
		{$$ = create_list($1,$3,NODE_ARG_LIST);}
	;

unary_expression
	: postfix_expression
		{$$ = $1;}
	| INC_OP unary_expression
		{$$ = create_un_op($2,NODE_INC_PRE);}
	| DEC_OP unary_expression
		{$$ = create_un_op($2,NODE_DEC_PRE);}
	| unary_operator unary_expression
		{$$ = create_un_op($2, $1);}
	;

unary_operator
	: '&'
		{$$ = NODE_ADDRESS;}
	| '*'
		{$$ = NODE_DEREF;}
	| '+'
		{$$ = NODE_PLUS;}
	| '-'
		{$$ = NODE_MINUS;}
	| '~'
		{$$ = NODE_BITWISE_NOT;}
	| '!'
		{$$ = NODE_LOGICAL_NOT;}
	;

type_name
	: type_specifier
		{$$ = create_type($1,0);}
	| type_specifier pointer
		{$$ = create_type($1,$2);}
	;

multiplicative_expression
	: unary_expression
		{$$ = $1;}
	| multiplicative_expression '*' unary_expression
		{$$ = create_bin_op($1, $3, NODE_MUL);}
	| multiplicative_expression '/' unary_expression
		{$$ = create_bin_op($1, $3, NODE_DIV);}
	| multiplicative_expression '%' unary_expression
		{$$ = create_bin_op($1, $3, NODE_MOD);}
	;

additive_expression
	: multiplicative_expression
		{$$ = $1;}
	| additive_expression '+' multiplicative_expression
		{$$ = create_bin_op($1, $3, NODE_ADD);}
	| additive_expression '-' multiplicative_expression
		{$$ = create_bin_op($1, $3, NODE_SUB);}
	;

shift_expression
	: additive_expression
		{$$ = $1;}
	| shift_expression LEFT_OP additive_expression
		{$$ = create_bin_op($1, $3, NODE_SHIFT_LEFT);}
	| shift_expression RIGHT_OP additive_expression
		{$$ = create_bin_op($1, $3, NODE_SHIFT_RIGHT);}
	;

relational_expression
	: shift_expression
		{$$ = $1;}
	| relational_expression '<' shift_expression
		{$$ = create_bin_op($1, $3, NODE_LT);}
	| relational_expression '>' shift_expression
		{$$ = create_bin_op($1, $3, NODE_GT);}
	| relational_expression LE_OP shift_expression
		{$$ = create_bin_op($1, $3, NODE_LEQ);}
	| relational_expression GE_OP shift_expression
		{$$ = create_bin_op($1, $3, NODE_GEQ);}
	;

equality_expression
	: relational_expression
		{$$ = $1;}
	| equality_expression EQ_OP relational_expression
		{$$ = create_bin_op($1, $3, NODE_IS_EQ);}
	| equality_expression NE_OP relational_expression
		{$$ = create_bin_op($1, $3, NODE_NOT_EQ);}
	;

and_expression
	: equality_expression
		{$$ = $1;}
	| and_expression '&' equality_expression
		{$$ = create_bin_op($1, $3, NODE_BITWISE_AND);}
	;

exclusive_or_expression
	: and_expression
		{$$ = $1;}
	| exclusive_or_expression '^' and_expression
		{$$ = create_bin_op($1, $3, NODE_BITWISE_XOR);}
	;

inclusive_or_expression
	: exclusive_or_expression
		{$$ = $1;}
	| inclusive_or_expression '|' exclusive_or_expression
		{$$ = create_bin_op($1, $3, NODE_BITWISE_OR);}
	;

logical_and_expression
	: inclusive_or_expression
		{$$ = $1;}
	| logical_and_expression AND_OP inclusive_or_expression
		{$$ = create_bin_op($1, $3, NODE_LOGICAL_AND);}
	;

logical_or_expression
	: logical_and_expression
		{$$ = $1;}
	| logical_or_expression OR_OP logical_and_expression
		{$$ = create_bin_op($1, $3, NODE_LOGICAL_OR);}
	;

assignment_expression
	: logical_or_expression
		{$$ = $1;}
	| unary_expression assignment_operator assignment_expression
		{$$ = create_assignment($1, $3, $2);}
	;

assignment_operator
	: '='
		{$$ = NODE_ASSIGN;}
	| MUL_ASSIGN
		{$$ = NODE_MUL_ASSIGN;}
	| DIV_ASSIGN
		{$$ = NODE_DIV_ASSIGN;}
	| MOD_ASSIGN
		{$$ = NODE_MOD_ASSIGN;}
	| ADD_ASSIGN
		{$$ = NODE_ADD_ASSIGN;}
	| SUB_ASSIGN
		{$$ = NODE_SUB_ASSIGN;}
	| LEFT_ASSIGN
		{$$ = NODE_SHIFT_LEFT_ASSIGN;}
	| RIGHT_ASSIGN
		{$$ = NODE_SHIFT_RIGHT_ASSIGN;}
	| AND_ASSIGN
		{$$ = NODE_AND_ASSIGN;}
	| XOR_ASSIGN
		{$$ = NODE_XOR_ASSIGN;}
	| OR_ASSIGN
		{$$ = NODE_OR_ASSIGN;}
	;

declaration
	: type_name init_declarator_list ';'
		{
			modify_types($2,$1);
			$$ = $2;}
	;
init_declarator_list
	: init_declarator
	| init_declarator ',' init_declarator_list
		{$$ = create_list($1,$3,NODE_DECLARATOR_LIST);}
	;
init_declarator
	: declarator
		{$$ = create_declaration($1,NULL,NULL);}
	| declarator '=' initializer
		{$$ = create_declaration($1,NULL,$3);}
	;
type_specifier
	: VOID
		{$$ = NODE_VOID;}
	| INT
		{$$ = NODE_INT;}
	| FLOAT
		{$$ = NODE_FLOAT;}
	;

declarator
	: IDENTIFIER
		{$$ = create_identifier($1);}
	| declarator '[' logical_or_expression ']'
		{$$ = create_array_declaration($1, $3);}
	| declarator '[' ']'
		{$$ = create_array_declaration($1, NULL);}
	;

pointer
	: '*'
		{$$ = 1;}
	| '*' pointer
		{$$ = $2 + 1;}
	;


parameter_list
	: parameter_declaration
	| parameter_declaration ',' parameter_list
		{$$ = create_list($1,$3,NODE_PARAM_LIST);}
	;

parameter_declaration
	: type_name declarator
		{$$ = create_param_declaration($1,$2);}
	;

initializer
	: assignment_expression
	| '{' initializer_list '}'
	{$$ = $2;}
	;

initializer_list
	: initializer
	| initializer ',' initializer_list
		{$$ = create_list($1,$3,NODE_INIT_LIST);}
	;

statement
	: expression_statement
	| selection_statement
	| iteration_statement
	| jump_statement
	;

compound_statement
	: '{' '}'
	 	{$$ = create_compound_stmt(NULL,NULL);}
	| '{' statement_list '}'
		{$$ = create_compound_stmt(NULL,$2);}
	| '{' declaration_list '}'
		{$$ = create_compound_stmt($2,NULL);}
	| '{' declaration_list statement_list '}'
		{$$ = create_compound_stmt($2,$3);}
	;

declaration_list
	: declaration
		{$$ = $1;}
	| declaration declaration_list
		{$$ = create_list($1,$2,NODE_DECLARATION_LIST);}
	;

statement_list
	: statement
		{$$ = $1;}
	| statement statement_list
		{$$ = create_list($1,$2,NODE_STMT_LIST);}

	;

expression_statement
	: assignment_expression ';'
		{$$ = $1;}
	;

selection_statement
	: IF '(' assignment_expression ')' compound_statement
		{$$ = create_if_stmt($3,$5,NULL);}
	| IF '(' assignment_expression ')' compound_statement ELSE compound_statement
		{$$ = create_if_stmt($3,$5,$7);}
	;

iteration_statement
	: WHILE '(' assignment_expression ')' compound_statement
		{$$ = create_while_loop($3,$5);}
	| DO compound_statement WHILE '(' assignment_expression ')' ';'
		{$$ = create_do_while_loop($5,$2);}
	| FOR '(' expression_statement expression_statement assignment_expression ')' compound_statement
		{$$ = create_for_loop($3,$4,$5,$7);}
	| FOR '(' declaration expression_statement assignment_expression ')' compound_statement
		{$$ = create_for_loop($3,$4,$5,$7);}
	;

jump_statement
	: CONTINUE ';'
		{$$ = create_jump(NODE_CONTINUE);}
	| BREAK ';'
		{$$ = create_jump(NODE_BREAK);}
	| RETURN ';'
		{$$ = create_return_stmt(NULL);}
	| RETURN assignment_expression ';'
		{$$ = create_return_stmt($2);}
	;

translation_unit
	: external_declaration
		{$$ = $1;
		root = $$;}
	| external_declaration translation_unit
		{$$ = create_list($1,$2,NODE_TOP_LEVEL_LIST);
		root = $$;		
		}

	;

external_declaration
	: function_definition
	| declaration
	;
function_definition
	: type_name IDENTIFIER '(' parameter_list ')' compound_statement
		{$$ = create_function_def($1,$2,$4,$6);}
	| type_name IDENTIFIER '(' ')' compound_statement
		{$$ = create_function_def($1,$2,NULL,$5);}
	;
