#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "shellmemory.h"
#include "shell.h"

#define MAX_PROGRAM_LINES 1000

int load_script (const char *filename, ProgramMemory * pm) {
    FILE *fp = fopen (filename, "r");
    if (!fp) {
        return -1;
    }


    char buffer[MAX_PROGRAM_LINES];

    while (fgets (buffer, sizeof (buffer), fp) != NULL
           && pm->num_lines < MAX_PROGRAM_LINES) {
        buffer[strcspn (buffer, "\r\n")] = '\0';
        int len = strlen (buffer);
        char *line_copy = malloc (len + 1);
        if (!line_copy) {
            fclose (fp);
            return -1;
        }
        strcpy (line_copy, buffer);

        pm->lines[pm->num_lines] = line_copy;
        pm->num_lines++;
    }

    fclose (fp);
    return 0;
}

void cleanup_program_memory (ProgramMemory * pm) {
    for (int i = 0; i < pm->num_lines; i++) {
        free (pm->lines[i]);
        pm->lines[i] = NULL;
    }
    pm->num_lines = 0;
}

ReadyQueue readyQueue;

void init_ready_queue (ReadyQueue * rq) {
    rq->head = NULL;
    rq->tail = NULL;
}

void enqueue_ready_queue (ReadyQueue * rq, PCB * pcb) {
    pcb->next = NULL;
    if (rq->tail == NULL) {
        rq->head = pcb;
        rq->tail = pcb;
    } else {
        rq->tail->next = pcb;
        rq->tail = pcb;
    }
}

PCB *dequeue_ready_queue (ReadyQueue * rq) {
    if (rq->head == NULL)
        return NULL;

    PCB *pcb = rq->head;
    rq->head = rq->head->next;
    if (rq->head == NULL)
        rq->tail = NULL;

    pcb->next = NULL;
    return pcb;
}

void scheduler_run (ProgramMemory * pm) {
    PCB *pcb = dequeue_ready_queue (&readyQueue);

    while (pcb != NULL) {
        while (pcb->pc < pcb->num_lines) {
            char *instruction = pm->lines[pcb->start + pcb->pc];
            parseInput (instruction);
            pcb->pc++;
        }
        free (pcb);
        pcb = dequeue_ready_queue (&readyQueue);
    }
}

void scheduler_run_rr (ProgramMemory * pm) {
    PCB *pcb = dequeue_ready_queue (&readyQueue);
    while (pcb != NULL) {
        int instructionsRun = 0;
        while (pcb->pc < pcb->num_lines && instructionsRun < 2) {
            char *line = pm->lines[pcb->start + pcb->pc];
            parseInput (line);
            pcb->pc++;
            instructionsRun++;
        }
        if (pcb->pc < pcb->num_lines) {
            enqueue_ready_queue (&readyQueue, pcb);
        } else {
            free (pcb);
        }
        pcb = dequeue_ready_queue (&readyQueue);
    }
}

// Helper for scheduler_run_aging
void age_ready_queue (ReadyQueue * rq, int exceptPid) {
    for (PCB * cur = rq->head; cur; cur = cur->next) {
        if (cur->pid != exceptPid && cur->job_length_score > 0) {
            cur->job_length_score--;
        }
    }
}

// Helper for scheduler_run_aging
void sorted_enqueue_by_score (ReadyQueue * rq, PCB * newPCB) {
    newPCB->next = NULL;

    if (rq->head == NULL ||
        newPCB->job_length_score <= rq->head->job_length_score) {

        newPCB->next = rq->head;
        rq->head = newPCB;

        if (rq->tail == NULL) {
            rq->tail = newPCB;
        }
        return;
    }

    PCB *prev = rq->head;
    while (prev->next != NULL &&
           prev->next->job_length_score <= newPCB->job_length_score) {
        prev = prev->next;
    }

    newPCB->next = prev->next;
    prev->next = newPCB;
    if (newPCB->next == NULL) {
        rq->tail = newPCB;
    }
}

void scheduler_run_aging (ProgramMemory * pm) {
    PCB *pcb = dequeue_ready_queue (&readyQueue);

    while (pcb != NULL) {
        char *instruction = pm->lines[pcb->start + pcb->pc];
        parseInput (instruction);
        pcb->pc++;

        if (pcb->pc >= pcb->num_lines) {
            free (pcb);
        } else {
            age_ready_queue (&readyQueue, pcb->pid);

            sorted_enqueue_by_score (&readyQueue, pcb);
        }

        pcb = dequeue_ready_queue (&readyQueue);
    }
}

void scheduler_run_rr30 (ProgramMemory * pm) {
    PCB *pcb = dequeue_ready_queue (&readyQueue);
    while (pcb != NULL) {
        int instructionsRun = 0;
        while (pcb->pc < pcb->num_lines && instructionsRun < 30) {
            char *line = pm->lines[pcb->start + pcb->pc];
            parseInput (line);
            pcb->pc++;
            instructionsRun++;
        }
        if (pcb->pc < pcb->num_lines) {
            enqueue_ready_queue (&readyQueue, pcb);
        } else {
            free (pcb);
        }
        pcb = dequeue_ready_queue (&readyQueue);
    }
}

struct memory_struct {
    char *var;
    char *value;
};

struct memory_struct shellmemory[MEM_SIZE];

// Helper functions
int match (char *model, char *var) {
    int i, len = strlen (var), matchCount = 0;
    for (i = 0; i < len; i++) {
        if (model[i] == var[i])
            matchCount++;
    }
    if (matchCount == len) {
        return 1;
    } else
        return 0;
}

// Shell memory functions

void mem_init () {
    int i;
    for (i = 0; i < MEM_SIZE; i++) {
        shellmemory[i].var = "none";
        shellmemory[i].value = "none";
    }
}

// Set key value pair
void mem_set_value (char *var_in, char *value_in) {
    int i;

    for (i = 0; i < MEM_SIZE; i++) {
        if (strcmp (shellmemory[i].var, var_in) == 0) {
            shellmemory[i].value = strdup (value_in);
            return;
        }
    }

    //Value does not exist, need to find a free spot.
    for (i = 0; i < MEM_SIZE; i++) {
        if (strcmp (shellmemory[i].var, "none") == 0) {
            shellmemory[i].var = strdup (var_in);
            shellmemory[i].value = strdup (value_in);
            return;
        }
    }

    return;
}

//get value based on input key
char *mem_get_value (char *var_in) {
    int i;

    for (i = 0; i < MEM_SIZE; i++) {
        if (strcmp (shellmemory[i].var, var_in) == 0) {
            return strdup (shellmemory[i].value);
        }
    }
    return "Variable does not exist";
}
