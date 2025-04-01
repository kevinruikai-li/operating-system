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

void sorted_enqueue_by_score (ReadyQueue * rq, PCB * pcb) {
    pcb->next = NULL;
    if (rq->head == NULL) {
        rq->head = pcb;
        rq->tail = pcb;
        return;
    }
    PCB *prev = NULL;
    PCB *curr = rq->head;
    while (curr != NULL && curr->job_length_score <= pcb->job_length_score) {
        prev = curr;
        curr = curr->next;
    }
    if (prev == NULL) {
        pcb->next = rq->head;
        rq->head = pcb;
    } else {
        prev->next = pcb;
        pcb->next = curr;
        if (curr == NULL) {
            rq->tail = pcb;
        }
    }
}

void scheduler_run_aging (ProgramMemory * pm) {
    PCB *current = NULL;

    while (readyQueue.head != NULL || current != NULL) {
        if (current == NULL) {
            current = dequeue_ready_queue (&readyQueue);
        }
        if (current->pc < current->num_lines) {
            char *instruction = pm->lines[current->start + current->pc];
            parseInput (instruction);
            current->pc++;
        }
        PCB *iter = readyQueue.head;
        while (iter != NULL) {
            if (iter->job_length_score > 0)
                iter->job_length_score--;
            iter = iter->next;
        }

        if (current->pc >= current->num_lines) {
            free (current);
            current = NULL;
            continue;
        }

        if (readyQueue.head != NULL
            && readyQueue.head->job_length_score < current->job_length_score) {
            sorted_enqueue_by_score (&readyQueue, current);
            current = NULL;
        }
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
