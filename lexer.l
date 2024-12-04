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
#define ECHO   do { if (DEBUG_MAIN) fwrite(yytext, yyleng, 1, yyout); } while (0);

void count();
Value* parse_num(const char* text);
Value* parse_float(const char* text);
char* parse_string(char* text);
Value* parse_float_or_int(const char* text);
Value* parse_char(const char* text);
extern int DEBUG_MAIN;
char* unescape_string(const char* escaped_str, int original_length, int* out_len);


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


\"(\\.|[^\\"])*\"	{ count(); yylval.string = parse_string(yytext);return(STRING_LITERAL); }

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
	while ((c = input()) != '*' && c != 0){
	    if (DEBUG_MAIN)
		    putchar(c);
    }
	if ((c1 = input()) != '/' && c != 0)
	{   if (DEBUG_MAIN)
		    unput(c1);
		goto loop;
	}

	if (c != 0 && DEBUG_MAIN)
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
char* parse_string(char* s){
	char* out = strdup(s+1);
	int len = strlen(out);
	out[len-1] = '\0';
	int out_len = 0;
	char* unescaped = unescape_string(out,len-1,&out_len);
	free(out);
	return unescaped;
}
char* unescape_string(const char* escaped_str, int original_length, int* out_len) {
    // Allocate memory for the unescaped string (same length or potentially shorter)
    char* unescaped_str = malloc(original_length + 1);
    if (unescaped_str == NULL) {
        return NULL;  // Memory allocation failed
    }
    int final_length = original_length;

    size_t i = 0;  // Index for escaped string
    size_t j = 0;  // Index for unescaped string

    while (original_length > 0) {
        if (escaped_str[i] == '\\') {
            // Handle escape sequences
            switch (escaped_str[i + 1]) {
                case 'n':  // Newline
                    unescaped_str[j++] = '\n';
                    i += 2;
                    break;
                case 'r':  // Carriage return
                    unescaped_str[j++] = '\r';
                    i += 2;
                    break;
                case 't':  // Tab
                    unescaped_str[j++] = '\t';
                    i += 2;
                    break;
                case '0':  // Null terminator
                    unescaped_str[j++] = '\0';
                    i += 2;
                    break;
                case '\\':  // Backslash
                    unescaped_str[j++] = '\\';
                    i += 2;
                    break;
                case '"':  // Double quote
                    unescaped_str[j++] = '"';
                    i += 2;
                    break;
                case '\'':  // Single quote
                    unescaped_str[j++] = '\'';
                    i += 2;
                    break;
                default:
                    // If it's not a recognized escape sequence,
                    // just copy the backslash and the next character
                    unescaped_str[j++] = escaped_str[i++];
                    unescaped_str[j++] = escaped_str[i++];
                    break;
            }
            final_length--;
            original_length -= 2;
        } else {
            // Copy regular characters
            unescaped_str[j++] = escaped_str[i++];
            original_length--;
        }
    }

    // Null-terminate the string
    unescaped_str[j] = '\0';
    *out_len = final_length;
    return unescaped_str;
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