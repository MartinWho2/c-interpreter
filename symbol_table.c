#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "symbol_table.h"

// Creates a symbolTables linked list by passing the new symbol table
SymbolTablesList* create_symbol_tables_list(SymbolTable* local_vars){
    SymbolTablesList* s = malloc(sizeof(SymbolTablesList));
    s->local_vars = local_vars;
    s->prev = NULL;
    s->is_new_function = true;
    return s;
}

// Adds a symbol table to the linked list and returns the new symbol table list (called when calling a function or compound statement)
SymbolTablesList* add_scope(SymbolTablesList* current, SymbolTable* new_symbol_table,bool is_new_function){
    SymbolTablesList* new_list = create_symbol_tables_list(new_symbol_table);
    new_list->prev = current;
    new_list->is_new_function = is_new_function;
    return new_list;
}
// Removes the most recent symbol table and returns the updated SymbolTablesList (when returning from a function)
SymbolTablesList* return_func_update_symbol_tables_list(SymbolTablesList* current){
    if (current == NULL){
        fprintf(stderr,"ERROR tried to return from function but was already at global scope\n");
        exit(1);
    }
    while (!(current->is_new_function)){
        current = exit_compound_stmt(current);
    }
    SymbolTablesList* prev = current->prev;
    free_symbol_table(current->local_vars);
    free(current);
    return prev;

}
SymbolTablesList* exit_compound_stmt(SymbolTablesList* current){
    if (current->is_new_function){
        fprintf(stderr,"ERROR tried to escape compound statement but was outer scope of function\n");
        exit(1);
    }
    SymbolTablesList* prev = current->prev;
    free_symbol_table(current->local_vars);
    free(current);
    return prev;
}

void free_symbol_tables_list(SymbolTablesList* current){
    while (current != NULL){
        current = return_func_update_symbol_tables_list(current);
    }
}

ScopeManager* create_scope_manager(){
    ScopeManager* manager = malloc(sizeof(ScopeManager));
    manager->global_vars = create_symbol_table(100);
    return manager;
}
void destroy_scope_manager(ScopeManager* scope_manager){
    free_symbol_table(scope_manager->global_vars);
    free_symbol_tables_list(scope_manager->symbol_tables);
    free(scope_manager);
}

// Returns to previous function
void return_to_prev_function(ScopeManager* scope_manager){
    SymbolTablesList* new_symbol_tables_list = return_func_update_symbol_tables_list(scope_manager->symbol_tables);
    scope_manager->symbol_tables = new_symbol_tables_list;
}
// Call function
void call_function(ScopeManager* scope_manager, SymbolTable* arguments){
    SymbolTablesList* new_symbol_tables_list = add_scope(scope_manager->symbol_tables,arguments,true);
    scope_manager->symbol_tables = new_symbol_tables_list;
}
// Enter a new scope (i.e. a compound statement)
void enter_new_scope_scope_manager(ScopeManager* scope_manager){
    SymbolTable* new_symbol_table = create_symbol_table(100);
    SymbolTablesList* new_symbol_tables_list = add_scope(scope_manager->symbol_tables,new_symbol_table,false);
    scope_manager->symbol_tables = new_symbol_tables_list;
}

void exit_scope_scope_manager(ScopeManager* scope_manager){
    SymbolTablesList* prev_scope = exit_compound_stmt(scope_manager->symbol_tables);
    scope_manager->symbol_tables = prev_scope;
}

// Hash function
unsigned int hash(const char* key, int table_size) {
    unsigned int hash_value = 0;
    while (*key) {
        hash_value = (hash_value * 31) + *key++;
    }
    return hash_value % table_size;
}

// Create a new symbol table
SymbolTable* create_symbol_table(int size) {
    SymbolTable* table = malloc(sizeof(SymbolTable));
    table->size = size;
    table->count = 0;
    table->entries = calloc(size, sizeof(SymbolEntry*));
    return table;
}
// update an already existing variable within the scope
bool update_symbol(ScopeManager* scopeManager, const char* name, int address, full_type_t *type){
    SymbolEntry* existing_entry = lookup_symbol(scopeManager,name);
    if (existing_entry == NULL){
        fprintf(stderr,"ERROR Tried to update symbol %s which does not exist\n",name);
        return false;
    }
    if (existing_entry->type.type != type->type || existing_entry->type.n_pointers != type->n_pointers){
        fprintf(stderr, "ERROR Tried to assign type ");
        print_type(*type,1);
        fprintf(stderr, " to %s which is of type ",name);
        print_type((existing_entry->type),1);
        exit(1);
    }
    existing_entry->address = address;
    return true;
}

// Insert a symbol into the table via the scope manager (new variable AND NOT AN EXISTING ONE)
bool insert_symbol(ScopeManager* scope_manager, const char* name, full_type_t *type,
                   int address, bool is_global) {
    SymbolTable* table;
    if (is_global){
        table = scope_manager->global_vars;
    }else{
        table = scope_manager->symbol_tables->local_vars;
    }
    return insert_symbol_in_table(table,name,type,address);
}

bool insert_symbol_in_table(SymbolTable* table, const char* name, full_type_t *type,
                            int address){
    // Check if table is getting too full (optional load factor check)
    if (table->count >= table->size * 0.75) {
        // TODO: Implement resizing mechanism
        fprintf(stderr,"warning table is too full, please change size\n");
    }

    unsigned int index = hash(name, table->size);

    // Check for existing symbol
    SymbolEntry* current = table->entries[index];
    while (current) {
        fprintf(stderr,"Collision !!\n");
        if (strcmp(current->name, name) == 0) {
            printf("ERROR, symbol %s already exists. IT SHOULD NOT BE THE CASE.\n", name);
            exit(1);
        }
        current = current->next;
    }

    // Create new symbol entry
    SymbolEntry* new_entry = malloc(sizeof(SymbolEntry));
    new_entry->name = strdup(name);
    new_entry->type = *type;
    new_entry->address = address;

    // Handle hash collision with separate chaining
    new_entry->next = table->entries[index];
    table->entries[index] = new_entry;

    table->count++;
    return true;
}

bool insert_param_in_symbol_table(ASTNode* curr_param, SymbolTable* table,int* stack_pointer){
    error_on_wrong_node(NODE_PARAM_DECLARATION,curr_param->type,"param of function definition");
    full_type_t * full_type = curr_param->data.param_declaration.type;
    ASTNode * identifier = curr_param->data.param_declaration.name;
    error_on_wrong_node(NODE_IDENTIFIER,identifier->type,"identifier for argument");
    const char* name = identifier->data.identifier.name;
    bool result = insert_symbol_in_table(table,name,full_type,*stack_pointer);
    if (!result){
        exit(1);
    }
    *stack_pointer = (*stack_pointer + type_size(full_type));
    return true;
}

// Lookup a symbol in entire scope
SymbolEntry* lookup_symbol(ScopeManager* scope_manager, const char* name){
    SymbolTablesList* symbol_tables = scope_manager->symbol_tables;
    // Iterate in reverse while there are symbol tables
    while (symbol_tables != NULL){
        SymbolEntry* entry = lookup_symbol_in_table(symbol_tables->local_vars,name);
        // If we found an entry in scope, return it
        if (entry != NULL){
            return entry;
        }
        // If not, and if it was the last scope of a function, stop looking for local variables
        if (symbol_tables->is_new_function){
            break;
        }
        // Otherwise go to previous scope
        symbol_tables = symbol_tables->prev;
    }
    // Since we didn't find it in local variables, look into globals
    SymbolEntry * last_try = lookup_symbol_in_table(scope_manager->global_vars,name);
    if (last_try != NULL) return last_try;
    fprintf(stderr, "[ERROR] Variable %s not found in scope.",name);
    exit(1);
}


// Lookup a symbol in the table
SymbolEntry* lookup_symbol_in_table(SymbolTable* table, const char* name) {
    unsigned int index = hash(name, table->size);
    
    SymbolEntry* current = table->entries[index];
    while (current) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    
    return NULL; // Symbol not found
}

// Remove a symbol from the table
bool remove_symbol(SymbolTable* table, const char* name) {
    unsigned int index = hash(name, table->size);
    
    SymbolEntry* current = table->entries[index];
    SymbolEntry* prev = NULL;
    
    while (current) {
        if (strcmp(current->name, name) == 0) {
            // Remove the entry
            if (prev) {
                prev->next = current->next;
            } else {
                table->entries[index] = current->next;
            }
            
            // Free memory
            free(current->name);
            free(current);
            table->count--;
            return true;
        }
        
        prev = current;
        current = current->next;
    }
    
    return false; // Symbol not found
}

int num_symbol_tables(ScopeManager* scope_manager){
    SymbolTablesList* symbolTablesList = scope_manager->symbol_tables;
    int calls = 0;
    while (symbolTablesList != NULL){
        calls++;
        symbolTablesList = symbolTablesList->prev;
    }
    return calls;
}

void print_symbol_table(SymbolTable* table){
    if (table == NULL) return;

    printf("  ║  📋 Symbol Table:             ║\n");
    for (int i = 0; i < table->size; ++i) {
        SymbolEntry * entry = table->entries[i];
        while (entry != NULL){
            printf("  ║   🔍 %-10s : %10d  ║\n", entry->name, entry->address);
            entry = entry->next;
        }
    }
}

void print_symbol_tables(ScopeManager* scope_manager){
    if (scope_manager == NULL) return;

    printf("  ║ 📚 Symbol Tables              ║\n");
    printf("  ╠═══════════════════════════════╣\n");

    SymbolTablesList* symbolTablesList = scope_manager->symbol_tables;
    int scope_count = 0;
    while (symbolTablesList != NULL){
        printf("  ║ 📦 Scope %2d:                  ║\n", scope_count++);
        print_symbol_table(symbolTablesList->local_vars);
        symbolTablesList = symbolTablesList->prev;
    }
    printf("  ╠═══════════════════════════════╣\n");
}

// Free the entire symbol table
void free_symbol_table(SymbolTable* table) {
    for (int i = 0; i < table->size; i++) {
        SymbolEntry* current = table->entries[i];
        while (current) {
            SymbolEntry* temp = current;
            current = current->next;
            // Free name and then cell
            free(temp->name);
            free(temp);
        }
    }
    
    free(table->entries);
    free(table);
}
















/*int fake_main(){
    ScopeManager* scope_manager = create_scope_manager();
    call_function(scope_manager, create_symbol_table(100));

    char* hello = malloc(10);
    strcpy(hello,"hello");
    int hello_var = 17;
    full_type_t * int_type = create_type(NODE_INT,0);
    full_type_t * int2_type = create_type(NODE_INT,0);
    full_type_t * float_type = create_type(NODE_FLOAT,0);

    insert_symbol(scope_manager,hello,int_type,hello_var,true);

    int hello_var2 = 18;
    char* hello2 = malloc(10);
    strcpy(hello2,"hello2");
    insert_symbol(scope_manager,hello2,int2_type,hello_var2,false);

    enter_new_scope(scope_manager);
    char* hello3 = malloc(10);
    strcpy(hello3,"hello4");
    int hello_var3 = 19;
    insert_symbol(scope_manager,hello3,int_type,hello_var3,false);

    SymbolTable *args = create_symbol_table(100);
    float hello_arg = 12.03f;
    insert_symbol_in_table(args,hello,float_type,hello_arg);
    insert_symbol_in_table(args,hello2,int2_type,hello_var2);
    call_function(scope_manager,args);

    SymbolEntry * res = lookup_symbol(scope_manager,hello);
    printf("Function call\n");

    res = lookup_symbol(scope_manager,hello2);

    res = lookup_symbol(scope_manager,hello3);
    printf("%p\n",res);

    return_to_prev_function(scope_manager);
    printf("Now back to main scope\n");
    res = lookup_symbol(scope_manager,hello);
    printf("%s = %d\n",res->name,*(int*)(res->value));

    res = lookup_symbol(scope_manager,hello2);
    printf("%s = %d\n",res->name,*(int*)(res->value));

    res = lookup_symbol(scope_manager,hello3);
    printf("%s = %d\n",res->name,*(int*)(res->value));
    update_symbol(scope_manager,hello ,hello_var3,int_type);
    printf("Now hello should be = %d\n",hello_var3);
    res = lookup_symbol(scope_manager,hello);
    printf("%s = %d\n",res->name,*(int*)(res->value));

    res = lookup_symbol(scope_manager,hello2);
    printf("%s = %d\n",res->name,*(int*)(res->value));

    res = lookup_symbol(scope_manager,hello3);
    printf("%s = %d\n",res->name,*(int*)(res->value));

    return_to_prev_function(scope_manager);
    printf("%p\n",scope_manager->symbol_tables);
    res = lookup_symbol(scope_manager,hello2);
    printf("%p\n",res);

    destroy_scope_manager(scope_manager);
}*/