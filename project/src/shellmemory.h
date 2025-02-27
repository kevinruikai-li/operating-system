#ifndef SHELLMEMORY_H
#   define SHELLMEMORY_H

#   define MEM_SIZE 1000
#   define MAX_PROGRAM_LINES 1000

typedef struct {
    char *lines[MAX_PROGRAM_LINES];
    int num_lines;
} ProgramMemory;

typedef struct PCB {
    int pid;
    int start;
    int num_lines;
    int pc;
    int job_length_score;
    struct PCB *next;
} PCB;

typedef struct ReadyQueue {
    PCB *head;
    PCB *tail;
} ReadyQueue;

extern ReadyQueue readyQueue;

void init_ready_queue (ReadyQueue * rq);
void enqueue_ready_queue (ReadyQueue * rq, PCB * pcb);
PCB *dequeue_ready_queue (ReadyQueue * rq);

void age_ready_queue (ReadyQueue * rq, int exceptPid);
void sorted_enqueue_by_score (ReadyQueue * rq, PCB * newPCB);

void scheduler_run (ProgramMemory * pm);
void scheduler_run_rr (ProgramMemory * pm);
void scheduler_run_aging (ProgramMemory * pm);
void scheduler_run_rr30 (ProgramMemory * pm);
void mem_init ();
void mem_set_value (char *var, char *value);
char *mem_get_value (char *var);
int load_script (const char *filename, ProgramMemory * pm);
void cleanup_program_memory (ProgramMemory * pm);

#endif
