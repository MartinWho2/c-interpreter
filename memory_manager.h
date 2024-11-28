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

int declare_new_variable(MemoryManager * memory_manager, full_type_t* full_type);
void* get_raw_ptr_for_var(MemoryManager * memoryManager, int variable_address);
