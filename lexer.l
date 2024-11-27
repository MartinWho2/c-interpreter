D			[0-9]
L			[a-zA-Z_]
H			[a-fA-F0-9]
E			[Ee][+-]?{D}+
FS			(f|F|l|L)

%{
#include <stdio.h>
#include "ast.h"
#include "y.tab.h"


void count();
ConstInfo* parse_num(const char* text);
ConstInfo* parse_float(const char* text);
ConstInfo* parse_string(const char* text);
ConstInfo* parse_float_or_int(const char* text);

%}

%%
"/*"			{ comment(); }
"//" 		 	{ while (input() != '\n'); count(); }

"break"			{ count(); return(BREAK); }
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

0[xX]{H}+		{ count(); yylval.const_info = parse_num(yytext);return(CONSTANT); }
0{D}+		{ count(); yylval.const_info = parse_num(yytext);return(CONSTANT); }
{D}+		{ count(); yylval.const_info = parse_num(yytext);return(CONSTANT); }

{D}+{E}{FS}?		{ count(); yylval.const_info = parse_float_or_int(yytext);return(CONSTANT); }
{D}*"."{D}+({E})?	{ count(); yylval.const_info = parse_float(yytext);return(CONSTANT); }
{D}+"."{D}*({E})?	{ count();yylval.const_info = parse_float(yytext); return(CONSTANT); }

\"(\\.|[^\\"])*\"	{ count(); yylval.const_info = parse_string(yytext);return(STRING_LITERAL); }

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
ConstInfo* parse_num(const char* s){
	ConstInfo* const_info = malloc(sizeof (ConstInfo));
	const_info->type = CONST_INT;
	const_info->value.int_val = strtol(s,NULL,0);
	return const_info;
}
ConstInfo* parse_float(const char* s){
	ConstInfo* const_info = malloc(sizeof (ConstInfo));
	const_info->type = CONST_FLOAT;
	const_info->value.float_val = strtof(s,NULL);
	return const_info;
}
ConstInfo* parse_float_or_int(const char* s){
	if (strchr(s, 'f') != NULL){
		return parse_float(s);
	}
	return parse_num(s);
}
ConstInfo* parse_string(const char* s){
	ConstInfo* const_info = malloc(sizeof (ConstInfo));
	const_info->type = CONST_STRING;
	const_info->value.string_val = strdup(s);
	return const_info;
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