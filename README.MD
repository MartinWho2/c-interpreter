## Compiler Design term project

This is the term project for the compiler design course CS420 at KAIST. The goal is to write an interpreter for a subset of C.

I chose to do it in C, using bison and flex.

# Run the program
To run the program, first compile the parser using the following commands :
```sh
lex lexer.l
yacc -d c_grammar.y -v
gcc lex.yy.c y.tab.c ast.c symbol_table.c memory_manager.c flow_manager.c global_manager.c main.c -O3 -o "exe_cute" -I .
```

Then you can run any C program that is part of my grammar with the following command:

```sh
./exe_cute file.c [--debug] [--print-func-call] [--no-mem]
```
# How to use it
If you want to run it, just like a normal C program, just run `./exe_cute file.c` and you will be fine.

Now if you want to use debug features, you can use `--debug` which will cause the debug menu to pop after each instruction.

If you are using big buffers, set the --no-mem flag while debugging, otherwise all the memory will be printed at every print of the debug menu.

Finally if you want to look at the function calls with arguments, add the --print-func-call flag to print at each function call what it looks like.

# Debug Menu
The debug menu pops at each instruction if you used --debug or every time the debug() function is called in the code.
The debug menu lets you either:
- p : print the global context of the program
- g <var_name>: print the value of a variable name
- c: resume execution
- n: execute line and debug at next line

# Implicit functions
- void debug(): will open debug menu
- int printf(): same semantic as from <stdio.h>, but format string only %d,%f,%c,%p
- void sleep(int x): sleep for x seconds
- int input(): scans stdin for an int and return it

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
- some implicit functions defined above

# Testing the interpreter
There are some written test programs in the test_programs directory of this repository.
For the basic one, you can try
```sh
./exe_cute test_programs/add.c
```

# Details about implementation
Please go to https://docs.google.com/presentation/d/1IRMmgP0Qw0TMl4HkLuNDYypUyu6dY5fv2E0NAlvzycQ to read more details about how the interpreter works
