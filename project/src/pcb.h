#pragma once
#include <stddef.h>
#include <stdio.h>              // FILE
#include "queue.h"

typedef size_t pid;


struct frame_user {
    struct PCB *pcb;
    size_t virtual_page;
};

extern struct frame_user **frame_users;
extern int *num_users_per_frame;

// A process info struct.
struct PCB {
    pid pid;
    // Store the process name, which we can use to detect that multiple
    // processes with the same name are being scheduled.
    // We set it to the empty string if the process name is not known,
    // which should only happen when the process is the 'shell input'
    // background process.
    // Note: on Linux, knowing the filenames is not enough to guarantee
    // that the files are actually different. Surprise!
    // There are a few reasons for this, e.g. `a/b` is the same file as
    // `a/../a/b` but also so-called "hardlinks." There's a way to detect
    // if two open files have the same "inode number," which is what we
    // would want to do in a real OS.
    // But for our purposes, we're going to stick to doing comparison by name.
    char *name;
    // Base+count form a base-bounds pair for the "memory allocated"
    // to this process. Seeing it this way helps see how we can replace
    // it with pseudo-paging later. Spoilers!
    size_t line_base;
    size_t line_count;

    size_t *page_table;
    size_t page_count;

    // This field is used for SJF and aging, and should initially have
    // the same value as line_count.
    size_t duration;

    // pc is the number of the instruction next to execute.
    // For example, it is initially 0, **regardless** of the value of
    // line_base. (Think of it as the "virtual address" of the next insn.)
    size_t pc;

    // As far as we're concerned, the only purpose of PCBs is to manage
    // scheduling; our "multiprocessing" structure simply isn't complicated
    // enough to warrant a factored-out PCB struct that can be used
    // in many different contexts. So we entangle it with the queue by just
    // including the next pointer here directly. The alternative would be
    // to invent a queue structure that has a pointer to a PCB,
    // and manage it separately.
    // If this PCB is the tail of the queue, next is NULL.
    struct PCB *next;

    char *backing_file;
};

// Returns non-zero iff there are more instructions to execute.
int pcb_has_next_instruction (struct PCB *pcb);
// Get the shellmemory index of the next instruction, and increment pc.
size_t pcb_next_instruction (struct PCB *pcb);
// Create a new process from the given filename:
//   1. Allocates a new PCB
//   2. Loads the code from the script file into shellmemory
//   3. Does NOT enqueue the PCB to any scheduling queue (next is NULL)
struct PCB *create_process (const char *filename, struct queue *q,
                            int process_exists);
// Like create_process, but takes a FILE* directly.
// Ownership of the FILE* is taken and it will be closed.
struct PCB *create_process_from_FILE (FILE * script, const char *filename,
                                      struct queue *q, int process_exists);
// Cleanup a process:
//   1. Free all shellmemory used by the process code
//   2. Free the PCB
void free_pcb (struct PCB *pcb);

struct PCB *find_existing_process (struct queue *q, const char *filename);

void register_frame_user (size_t frame, struct PCB *pcb, size_t page);
void invalidate_frame_users (size_t frame);
size_t select_victim_frame (void);
void load_page (struct PCB *pcb, size_t page_num, size_t frame);
