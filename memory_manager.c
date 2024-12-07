#include "memory_manager.h"
#include <stdlib.h>
#include <stdio.h>

#define STACK_SIZE 100000

FrameList * create_frame_list(int new_frame_ptr){
    FrameList * frame_list = malloc(sizeof(FrameList));
    frame_list->current_frame_pointer = new_frame_ptr;
    frame_list->last_frames = NULL;
    return frame_list;
}
FrameList * add_new_frame_list(FrameList* frame_list, int new_frame_ptr){
    FrameList * new_frame_list = create_frame_list(new_frame_ptr);
    new_frame_list->last_frames = frame_list;
    return new_frame_list;
}

// Frees the last frame and returns the current frameslist
FrameList * return_to_last_frame(FrameList* frame_list){
    FrameList * new_frame = frame_list->last_frames;
    free(frame_list);
    return new_frame;
}
void destroy_frame_list(FrameList* frame_list){
    while (frame_list != NULL){
        frame_list = return_to_last_frame(frame_list);
    }
}

MemoryManager * create_memory_manager(){
    MemoryManager * memory_manager = malloc(sizeof(MemoryManager));
    memory_manager->stack_pointer = 0;
    memory_manager->frame_list = create_frame_list(0);
    void* stack_memory = malloc(STACK_SIZE);
    memory_manager->base_of_stack = stack_memory;
    memory_manager->size_memory = STACK_SIZE;
    return memory_manager;
}
void destroy_memory_manager(MemoryManager* memory_manager){
    destroy_frame_list(memory_manager->frame_list);
    // Free all stack memory of interpreted program
    free(memory_manager->base_of_stack);
    free(memory_manager);
}

void increase_memory(MemoryManager* memory_manager, int scale_factor){
    memory_manager->size_memory *= scale_factor;
    void* new_base = realloc(memory_manager->base_of_stack,memory_manager->size_memory);
    if (new_base == NULL){
        fprintf(stderr,"[Error] Not enough memory...");
        exit(1);
    }
    memory_manager->base_of_stack = new_base;

}
// BE CAREFUL THAT BASE OF STACK MIGHT BE CHANGED AFTER THIS CALL, SO DON'T SAVE IT BEFORE
void increase_memory_if_needed(MemoryManager* memoryManager){
    if (memoryManager->stack_pointer >= 0.9 * memoryManager->size_memory)
        increase_memory(memoryManager,
                        memoryManager->stack_pointer / memoryManager->size_memory + 1);
}

int create_buffer(MemoryManager *memory_manager,full_type_t* full_type, int n_elems){
    int size = type_size(full_type) * n_elems;
    if (size < 0 || n_elems < 0){
        fprintf(stderr,"[ERROR] Buffer wayyy too big, OVERFLOW...");
        exit(1);
    }
    int buffer_address = memory_manager->stack_pointer;
    memory_manager->stack_pointer += size;
    if (memory_manager->stack_pointer >= memory_manager->size_memory){
        increase_memory(memory_manager,memory_manager->stack_pointer / memory_manager->size_memory + 1);
    }
    return buffer_address;
}

int declare_new_variable_in_memory(MemoryManager * memory_manager, full_type_t* full_type){
    int size = type_size(full_type);
    int address_for_variable = memory_manager->stack_pointer;
    memory_manager->stack_pointer += size;
    if (memory_manager->stack_pointer >= memory_manager->size_memory){
        increase_memory(memory_manager,2);
        address_for_variable = memory_manager->stack_pointer - size;
    }
    return address_for_variable;
}


void set_stack_pointer_to_curr_frame_pointer(MemoryManager * memory_manager){
    memory_manager->stack_pointer = memory_manager->frame_list->current_frame_pointer;
}


void* get_raw_ptr_for_var(MemoryManager * memoryManager, int variable_address){
    if (variable_address > memoryManager->size_memory || variable_address < 0){
        fprintf(stderr,"[ERROR] Tried to get the memory of a value out of stack memory ? (%d)",variable_address);
        exit(1);
    }
    return (variable_address) + memoryManager->base_of_stack;
}