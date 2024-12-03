#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "symbol_table.h"

#define DEFAULT_SYMBOL_TABLE_SIZE 4

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
    manager->global_vars = create_symbol_table(DEFAULT_SYMBOL_TABLE_SIZE);
    manager->symbol_tables = NULL;
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
    SymbolTable* new_symbol_table = create_symbol_table(DEFAULT_SYMBOL_TABLE_SIZE);
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

SymbolTable* resize_symbol_table(SymbolTable* old_table) {
    // Choose a new size, roughly doubling the current size
    int new_size = old_table->size * 2;

    // Create a new symbol table with the larger size
    SymbolTable* new_table = malloc(sizeof(SymbolTable));
    new_table->size = new_size;
    new_table->count = 0;
    new_table->entries = calloc(new_size, sizeof(SymbolEntry*));

    // Rehash all existing entries into the new table
    for (int i = 0; i < old_table->size; i++) {
        SymbolEntry* current = old_table->entries[i];
        while (current) {
            // Save the next entry before rehashing
            SymbolEntry* next = current->next;
            unsigned int new_index = hash(current->name, new_size);

            // Prepare this entry for reinsertion
            current->next = new_table->entries[new_index];
            new_table->entries[new_index] = current;
            new_table->count++;

            // Move to next entry in old table
            current = next;
        }
    }

    // Free the old table structure (but not the entries, as they've been moved)
    free(old_table->entries);
    free(old_table);

    return new_table;
}

// update an already existing variable within the scope
bool update_symbol(ScopeManager* scopeManager, const char* name, int address, full_type_t *type){
    SymbolEntry* existing_entry = lookup_symbol(scopeManager,name);
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
bool insert_symbol(ScopeManager *scope_manager, const char *name, full_type_t *type, int address) {
    SymbolTable* table;
    if (scope_manager->symbol_tables == NULL){
        table = scope_manager->global_vars;
    }else{
        table = scope_manager->symbol_tables->local_vars;
    }
    SymbolTable *res = insert_symbol_in_table(table,name,type,address);
    if (res){
        if (scope_manager->symbol_tables == NULL){
            scope_manager->global_vars = res;
            return true;
        }
        scope_manager->symbol_tables->local_vars = res;
    }
    return true;
}
// returns a new symbol table if the previous one was too small
SymbolTable *insert_symbol_in_table(SymbolTable* table, const char* name, full_type_t *type,
                            int address){
    int resized = 0;
    // Check if table is getting too full (optional load factor check)
    if (table->count >= table->size * 0.75) {
        table = resize_symbol_table(table);
        resized = 1;
    }

    unsigned int index = hash(name, table->size);

    // Check for existing symbol
    SymbolEntry* current = table->entries[index];
    while (current) {
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
    return resized ? table : NULL;
}
// return a new symbol table if it was too full
SymbolTable *insert_param_in_symbol_table(ASTNode* curr_param, SymbolTable* table,int* stack_pointer){
    error_on_wrong_node(NODE_PARAM_DECLARATION,curr_param->type,"param of function definition");
    full_type_t * full_type = curr_param->data.param_declaration.type;
    ASTNode * identifier = curr_param->data.param_declaration.name;
    error_on_wrong_node(NODE_IDENTIFIER,identifier->type,"identifier for argument");
    const char* name = identifier->data.identifier.name;

    SymbolTable *result = insert_symbol_in_table(table,name,full_type,*stack_pointer);
    *stack_pointer = (*stack_pointer + type_size(full_type));
    return result;
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
    printf("  ║ 📦 Globals :                  ║\n");
    print_symbol_table(scope_manager->global_vars);
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
