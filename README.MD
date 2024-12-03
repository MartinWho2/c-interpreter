## Compiler Design term project

This is the term project for the compiler design course CS420 at KAIST. The goal is to write an interpreter for a subset of C.

I chose to do it in C, using bison and flex.

# Run the program
To run the program, first compile the parser using the following commands :
```sh
flex lexer.l
bison -d c_grammar.y
gcc lex.yy.c y.tab.c ast.c symbol_table.c memory_manager.c flow_manager.c global_manager.c main.c -O3 -o "exe_cute" -I .
```

Then you can run any C program that is part of my grammar with the following command:

```sh
./exe_cute file.c
```

# Changes from C
The parts removed from C for this interpreter are:
- struct and union
- typedef
- ternary operator
- if/else statement without \{\}
- switch case
- function pointer
- function declaration
- implicit typecasting
- only types are float, int and char (with pointers of course)
# Additions (as it is an interpreter only)
- globals can call any function
- 

# Testing the interpreter
There are some written test programs in the test_programs directory of this repository.
For the basic one, you can try
```sh
./exe_cute test_programs/add.c
```
