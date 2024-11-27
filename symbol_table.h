// Enum for different variable types in the interpreter
typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_STRING,
} SymbolType;


// Symbol table entry structure
typedef struct SymbolEntry {
    char* name;               // Variable/function name
    SymbolType type;          // Type of the symbol
    void* value;              // Stored value
    struct SymbolEntry* next; // Next entry for hash collision handling
} SymbolEntry;

// Symbol table structure (hash table)
typedef struct {
    int size;                 // Size of the hash table
    int count;                // Number of entries
    SymbolEntry** entries;    // Array of symbol entry pointers
} SymbolTable;
typedef struct SymbolTablesList SymbolTablesList;
typedef struct SymbolTablesList{
    SymbolTable* local_vars;
    SymbolTablesList* prev;
    bool is_new_function;
} SymbolTablesList;

// Scope manager structure
typedef struct {
    SymbolTable* global_vars;   // Global variables
    SymbolTablesList* symbol_tables;  // Previous scopes symbol table
} ScopeManager;

void print_type(SymbolType t);
SymbolTablesList* create_symbol_tables_list(SymbolTable* local_vars);
SymbolTablesList* add_scope(SymbolTablesList* current, SymbolTable* new_symbol_table,bool is_new_function);
SymbolTablesList* return_func_update_symbol_tables_list(SymbolTablesList* current);
SymbolTablesList* exit_compound_stmt(SymbolTablesList* current);
void free_symbol_tables_list(SymbolTablesList* current);
ScopeManager* create_scope_manager();
void return_to_prev_function(ScopeManager* scope_manager);
void call_function(ScopeManager* scope_manager,SymbolTable* arguments);
void enter_new_scope(ScopeManager* scope_manager);
void exit_scope(ScopeManager* scope_manager);
unsigned int hash(const char* key, int table_size);
SymbolTable* create_symbol_table(int size);
bool insert_symbol(ScopeManager* scope_manager, const char* name, SymbolType type, void* value, bool is_global);
SymbolEntry* lookup_symbol(ScopeManager* scope_manager, const char* name);
SymbolEntry* lookup_symbol_in_table(SymbolTable* table, const char* name);
bool remove_symbol(SymbolTable* table, const char* name);
void free_symbol_table(SymbolTable* table);
bool insert_symbol_in_table(SymbolTable* table, const char* name, SymbolType type, void* value);