## Compiler Design term project

This is the term project for the compiler design course CS420 at KAIST. The goal is to write an interpreter for a subset of C.

I chose to do it in C, using bison and flex.

# Run the program
To run the program, first compile the parser using the following commands :
```sh
flex lexer.l
yacc -d c_grammar.y
gcc lex.yy.c y.tab.c ast.c main.c -o interpreter -I .
```

Then you can run any C program that is part of my grammar with the following command:

```sh
./interpreter file.c
```

