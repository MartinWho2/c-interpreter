#include "ast.h"
typedef struct FrameList {
    int current_frame_pointer;
    struct FrameList* last_frames;
} FrameList;

typedef struct MemoryManager {
    int stack_pointer;
    FrameList* frame_list;
    void* base_of_stack;
    int size_memory;
} MemoryManager;

FrameList * create_frame_list(int new_frame_ptr);
FrameList * add_new_frame_list(FrameList* frame_list, int new_frame_ptr);
FrameList * return_to_last_frame(FrameList* frame_list);
void destroy_frame_list(FrameList* frame_list);
MemoryManager * create_memory_manager();
void destroy_memory_manager(MemoryManager* memory_manager);
void set_stack_pointer_to_curr_frame_pointer(MemoryManager * memory_manager);
int create_buffer(MemoryManager *memory_manager,full_type_t* full_type, int n_elems);
void increase_memory_if_needed(MemoryManager* memoryManager);

int declare_new_variable_in_memory(MemoryManager * memory_manager, full_type_t* full_type);
void* get_raw_ptr_for_var(MemoryManager * memoryManager, int variable_address);
