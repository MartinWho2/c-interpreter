#include "ast.h"

// Symbol table entry structure
typedef struct SymbolEntry {
    char* name;               // Variable/function name
    full_type_t type;          // Type of the symbol
    int address;              // Stored value
    struct SymbolEntry* next; // Next entry for hash collision handling
} SymbolEntry;

// Symbol table structure (hash table)
typedef struct {
    int size;                 // Size of the hash table
    int count;                // Number of entries
    SymbolEntry** entries;    // Array of symbol entry pointers
} SymbolTable;
typedef struct SymbolTablesList{
    SymbolTable* local_vars;
    struct SymbolTablesList* prev;
    bool is_new_function;
} SymbolTablesList;

// Scope manager structure
typedef struct {
    SymbolTable* global_vars;   // Global variables
    SymbolTablesList* symbol_tables;  // Previous scopes symbol table
} ScopeManager;

void print_symbol_type(full_type_t *t);
void print_symbol_tables(ScopeManager* scope_manager);
SymbolTablesList* create_symbol_tables_list(SymbolTable* local_vars);
SymbolTablesList* add_scope(SymbolTablesList* current, SymbolTable* new_symbol_table,bool is_new_function);
SymbolTablesList* return_func_update_symbol_tables_list(SymbolTablesList* current);
SymbolTablesList* exit_compound_stmt(SymbolTablesList* current);
void free_symbol_tables_list(SymbolTablesList* current);
ScopeManager* create_scope_manager();
void destroy_scope_manager(ScopeManager* scope_manager);
void return_to_prev_function(ScopeManager* scope_manager);
void call_function(ScopeManager* scope_manager,SymbolTable* arguments);
void enter_new_scope_scope_manager(ScopeManager* scope_manager);
void exit_scope_scope_manager(ScopeManager* scope_manager);
unsigned int hash(const char* key, int table_size);
SymbolTable* create_symbol_table(int size);

bool insert_symbol(ScopeManager *scope_manager, const char *name, full_type_t *type, int address);
SymbolEntry* lookup_symbol(ScopeManager* scope_manager, const char* name);
SymbolEntry* lookup_symbol_in_table(SymbolTable* table, const char* name);
bool remove_symbol(SymbolTable* table, const char* name);
bool update_symbol(ScopeManager* scopeManager, const char* name, int address, full_type_t *type);
void free_symbol_table(SymbolTable* table);
SymbolTable *insert_symbol_in_table(SymbolTable* table, const char* name, full_type_t *type, int address);
SymbolTable *insert_param_in_symbol_table(ASTNode* curr_param, SymbolTable* table,int* stack_pointer);
int num_symbol_tables(ScopeManager* scope_manager);
