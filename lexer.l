D			[0-9]
L			[a-zA-Z_]
H			[a-fA-F0-9]
E			[Ee][+-]?{D}+
FS			(f|F|l|L)

%{
#include <stdio.h>
#include <string.h>
#include "ast.h"
#include "y.tab.h"


void count();
Value* parse_num(const char* text);
Value* parse_float(const char* text);
Value* parse_string(const char* text);
Value* parse_float_or_int(const char* text);
Value* parse_char(const char* text);
%}

%%
"/*"			{ comment(); }
"//" 		 	{ while (input() != '\n'); count(); }

"break"			{ count(); return(BREAK); }
"char"			{ count(); return(CHAR); }
"continue"		{ count(); return(CONTINUE); }
"do"			{ count(); return(DO); }
"else"			{ count(); return(ELSE); }
"float"			{ count(); return(FLOAT); }
"for"			{ count(); return(FOR); }
"if"			{ count(); return(IF); }
"int"			{ count(); return(INT); }
"return"		{ count(); return(RETURN); }
"void"			{ count(); return(VOID); }
"while"			{ count(); return(WHILE); }

{L}({L}|{D})*		{ count(); yylval.string = strdup(yytext);return(IDENTIFIER); }

0[xX]{H}+		{ count(); yylval.value = parse_num(yytext);return(CONSTANT); }
0{D}+		{ count(); yylval.value = parse_num(yytext);return(CONSTANT); }
{D}+		{ count(); yylval.value = parse_num(yytext);return(CONSTANT); }

{D}+{E}{FS}?		{ count(); yylval.value = parse_float_or_int(yytext);return(CONSTANT); }
{D}*"."{D}+({E})?	{ count(); yylval.value = parse_float(yytext);return(CONSTANT); }
{D}+"."{D}*({E})?	{ count();yylval.value = parse_float(yytext); return(CONSTANT); }
'(\\.|[^\\'])'	{ count(); yylval.value = parse_char(yytext); return(CONSTANT); }


\"(\\.|[^\\"])*\"	{ count(); yylval.value = parse_string(yytext);return(STRING_LITERAL); }

">>="			{ count(); return(RIGHT_ASSIGN); }
"<<="			{ count(); return(LEFT_ASSIGN); }
"+="			{ count(); return(ADD_ASSIGN); }
"-="			{ count(); return(SUB_ASSIGN); }
"*="			{ count(); return(MUL_ASSIGN); }
"/="			{ count(); return(DIV_ASSIGN); }
"%="			{ count(); return(MOD_ASSIGN); }
"&="			{ count(); return(AND_ASSIGN); }
"^="			{ count(); return(XOR_ASSIGN); }
"|="			{ count(); return(OR_ASSIGN); }
">>"			{ count(); return(RIGHT_OP); }
"<<"			{ count(); return(LEFT_OP); }
"++"			{ count(); return(INC_OP); }
"--"			{ count(); return(DEC_OP); }
"&&"			{ count(); return(AND_OP); }
"||"			{ count(); return(OR_OP); }
"<="			{ count(); return(LE_OP); }
">="			{ count(); return(GE_OP); }
"=="			{ count(); return(EQ_OP); }
"!="			{ count(); return(NE_OP); }
";"			{ count(); return(';'); }
("{"|"<%")		{ count(); return('{'); }
("}"|"%>")		{ count(); return('}'); }
","			{ count(); return(','); }
":"			{ count(); return(':'); }
"="			{ count(); return('='); }
"("			{ count(); return('('); }
")"			{ count(); return(')'); }
("["|"<:")		{ count(); return('['); }
("]"|":>")		{ count(); return(']'); }
"."			{ count(); return('.'); }
"&"			{ count(); return('&'); }
"!"			{ count(); return('!'); }
"~"			{ count(); return('~'); }
"-"			{ count(); return('-'); }
"+"			{ count(); return('+'); }
"*"			{ count(); return('*'); }
"/"			{ count(); return('/'); }
"%"			{ count(); return('%'); }
"<"			{ count(); return('<'); }
">"			{ count(); return('>'); }
"^"			{ count(); return('^'); }
"|"			{ count(); return('|'); }
"?"			{ count(); return('?'); }

[ \t\v\n\f]		{ count(); }
.			{ /* ignore bad characters */ }

%%

yywrap()
{
	return(1);
}


comment()
{
	char c, c1;

loop:
	while ((c = input()) != '*' && c != 0)
		putchar(c);

	if ((c1 = input()) != '/' && c != 0)
	{
		unput(c1);
		goto loop;
	}

	if (c != 0)
		putchar(c1);
}


int column = 0;

void count()
{
	int i;

	for (i = 0; yytext[i] != '\0'; i++)
		if (yytext[i] == '\n')
			column = 0;
		else if (yytext[i] == '\t')
			column += 8 - (column % 8);
		else
			column++;

	ECHO;
}

/*ConstInfo* parse_hex(const char* s){
	ConstInfo* const_info = malloc(sizeof (ConstInfo))
	const_info->type = ConstType.CONST_INT;
	const_info->value.int_val = strtoll(s,NULL,16);
}
ConstInfo* parse_octal(const char* s){
	ConstInfo* const_info = malloc(sizeof (ConstInfo))
	const_info->type = ConstType.CONST_INT;
	const_info->value.int_val = strtoll(s,NULL,8);
}*/
Value* parse_num(const char* s){
	Value* value = malloc(sizeof (Value));
	full_type_t full_type = {NODE_INT,0};
	value->type = full_type;
	value->value.i = strtol(s,NULL,0);
	value->is_constant = 1;
	return value;
}
Value* parse_float(const char* s){
	Value* value = malloc(sizeof (Value));
	full_type_t full_type = {NODE_FLOAT,0};
	value->type = full_type;
	value->value.f = strtof(s,NULL);
	value->is_constant = 1;
	return value;
}
Value* parse_float_or_int(const char* s){
	if (strchr(s, 'f') != NULL){
		return parse_float(s);
	}
	return parse_num(s);
}
Value* parse_string(const char* s){
	Value* value = malloc(sizeof (Value));
	full_type_t full_type = {NODE_CHAR,1};
	value->type = full_type;
	value->value.ptr = strdup(s);
	value->is_constant = 1;
	return value;
}

Value* parse_char(const char* s){
	Value* value = malloc(sizeof (Value));
	full_type_t full_type = {NODE_CHAR,0};
	value->type = full_type;
	if (s[1] == '\\'){
		switch(s[2]){
			case 'n': value->value.c = '\n'; break;
			case 't': value->value.c = '\t'; break;
		}
	}else{
		value->value.c = s[1];

	}
	return value;
}

int check_type()
{
/*
* pseudo code --- this is what it should check
*
*	if (yytext == type_name)
*		return(TYPE_NAME);
*
*	return(IDENTIFIER);
*/

/*
*	it actually will only return IDENTIFIER
*/

	return(IDENTIFIER);
}