#include <stdio.h>
#include <stdlib.h>
#include <string.h> // memset
#include "shell.h" // MAX_USER_INPUT
#include "shellmemory.h"
#include "pcb.h"
#include "queue.h"

int pcb_has_next_instruction(struct PCB *pcb) {
    // have next if pc < line_count.
    // Sanity check: count = 0  ==> never have next. Good!
    return pcb->pc < pcb->line_count;
}

size_t pcb_next_instruction(struct PCB *pcb) {
    size_t virtual_addr = pcb->pc;
    size_t page_num = virtual_addr / FRAME_SIZE;
    size_t offset = virtual_addr % FRAME_SIZE;
    size_t frame_num;
    size_t physical_addr;

    if (pcb->page_table[page_num] == (size_t)(-1)) {
        
        frame_num = find_free_frame();
        
        if (frame_num == (size_t)(-1)) {
            frame_num = select_victim_frame();
            printf("Page fault! Victim page contents:\n\n");
            
            for (size_t i = 0; i < FRAME_SIZE; i++) {
                size_t line_idx = frame_num * FRAME_SIZE + i;
                if (line_idx < MEM_SIZE && linememory[line_idx].allocated) {
                    printf("%s", linememory[line_idx].line);
                }
            }
            printf("\nEnd of victim page contents.\n");

            invalidate_frame_users(frame_num);
            
            for (size_t i = 0; i < FRAME_SIZE; i++) {
                free_line(frame_num * FRAME_SIZE + i);
            }
        }

        load_page(pcb, page_num, frame_num);
    }
    
    frame_num = pcb->page_table[page_num];
    
    last_used[frame_num] = time_counter++;

    physical_addr = frame_num * FRAME_SIZE + offset;
    pcb->pc++;
    
    return physical_addr;
}


struct PCB *create_process(const char *filename, struct queue *q, int process_exists) {
    FILE *script = fopen(filename, "rt");
    if (!script) {
        perror("failed to open file for create_process");
        return NULL;
    }
    
    struct PCB *pcb = malloc(sizeof(struct PCB));
    static pid fresh_pid = 1;
    pcb->pid = fresh_pid++;
    pcb->name = strdup(filename);
    pcb->next = NULL;
    pcb->pc = 0;
    pcb->backing_file = strdup(filename);
    
    if (process_exists) {
        struct PCB *existing_pcb = find_existing_process(q, filename);
        
        pcb->page_count = existing_pcb->page_count;
        pcb->page_table = malloc(pcb->page_count * sizeof(size_t));
        
        for (size_t i = 0; i < pcb->page_count; i++) {
            pcb->page_table[i] = existing_pcb->page_table[i];
        }

        pcb->line_base = existing_pcb->line_base;
        pcb->line_count = existing_pcb->line_count;
    } else {
        char linebuf[MAX_USER_INPUT];
        size_t line_count = 0;
        
        while (!feof(script)) {
            if (fgets(linebuf, MAX_USER_INPUT, script) != NULL) {
                line_count++;
            }
        }
        
        pcb->page_count = (line_count + FRAME_SIZE - 1) / FRAME_SIZE;
        pcb->page_table = malloc(pcb->page_count * sizeof(size_t));

        for (size_t i = 0; i < pcb->page_count; i++) {
            pcb->page_table[i] = (size_t)(-1);
        }
        
        rewind(script);
        
        size_t pages_to_load = (pcb->page_count < 2) ? pcb->page_count : 2;
        
        for (size_t page = 0; page < pages_to_load; page++) {
            size_t frame = find_free_frame();
            if (frame == (size_t)(-1)) {
                free_pcb(pcb);
                fclose(script);
                return NULL;
            }
            
            pcb->page_table[page] = frame;
            last_used[frame] = time_counter++;
            register_frame_user(frame, pcb, page);

            for (size_t i = 0; i < FRAME_SIZE; i++) {
                if (feof(script)) break;
                
                memset(linebuf, 0, sizeof(linebuf));
                if (fgets(linebuf, MAX_USER_INPUT, script) != NULL) {
                    size_t line_index = frame * FRAME_SIZE + i;
                    allocate_line_at(line_index, linebuf);
                }
            }
        }
        
        pcb->line_base = 0;
        pcb->line_count = line_count;
    }
    
    pcb->duration = pcb->line_count;
    fclose(script);
    return pcb;
}

struct PCB *find_existing_process(struct queue *q, const char *filename) {
    struct PCB *current_pcb = q->head;
    while (current_pcb) {
        if (strcmp(current_pcb->name, filename) == 0) return current_pcb;
        current_pcb = current_pcb->next;
    }
    return NULL;
}

struct PCB *create_process_from_FILE(FILE *script, const char *filename, struct queue *q, int process_exists) {
    // We can open the file, so we'll be making a process.
    struct PCB *pcb = malloc(sizeof(struct PCB));

    // The PID is the only weird part. They need to be distinct,
    // so let's use a static counter.
    static pid fresh_pid = 1;
    pcb->pid = fresh_pid++;

    // Update the pcb name according to the filename we received.
    pcb->name = strdup(filename);
    // next should be NULL, according to doc comment.
    pcb->next = NULL;

    // pc is always initially 0.
    pcb->pc = 0;

    // create initial values for base and count, in case we fail to read
    // any lines from the file. That way we'll end up with an empty process
    // that terminates as soon as it is scheduled -- reasonable behavior for
    // an empty script.
    // If we don't do this, and the loop below never runs, base would be
    // unitialized, which would be catastrophic.
    pcb->line_count = 0;
    pcb->line_base = 0;

    // We're told to assume lines of files are limited to 100 characters.
    // That's all well and good, but for implementing # we need to read
    // actual user input, and _that_ is limited to 1000 characters.
    // It's unclear if we should assume it's also limited to 100 for this
    // purpose. If you did assume that, that's OK! We didn't.
    char linebuf[MAX_USER_INPUT];

    if (process_exists) {
        struct PCB *existing_pcb = find_existing_process(q, filename);
        pcb->line_base = existing_pcb->line_base;
        pcb->line_count = existing_pcb->line_count;
    } else {
        while (fgets(linebuf, MAX_USER_INPUT, script) != NULL) {
            size_t index = allocate_line(linebuf);
            if (index == (size_t)(-1)) {
                free_pcb(pcb);
                fclose(script);
                return NULL;
            }
            if (pcb->line_count == 0) {
                pcb->line_base = index;
            }
            pcb->line_count++;
        }
    }

    // We're done with the file, don't forget to close it!
    fclose(script);

    // duration should initially match line_count.
    pcb->duration = pcb->line_count;

    return pcb;
}

void free_pcb(struct PCB *pcb) {
    for (size_t i = 0; i < pcb->page_count; i++) {
        size_t frame = pcb->page_table[i];
        if (frame != (size_t)(-1)) {
            for (size_t j = 0; j < FRAME_SIZE; j++) {
                free_line(frame * FRAME_SIZE + j);
            }
            free_frame(frame);
        }
    }
    
    free(pcb->page_table);
    
    if (strcmp("", pcb->name)) {
        free(pcb->name);
    }
    free(pcb);
}

void register_frame_user(size_t frame, struct PCB *pcb, size_t page) {
    int n = num_users_per_frame[frame];

    if (n == 0) {
        frame_users[frame] = malloc(sizeof(struct frame_user));
    } else {
        frame_users[frame] = realloc(frame_users[frame], 
                                     (n+1) * sizeof(struct frame_user));
    }

    frame_users[frame][n].pcb = pcb;
    frame_users[frame][n].virtual_page = page;
    num_users_per_frame[frame]++;
}

void invalidate_frame_users(size_t frame) {
    for (int i = 0; i < num_users_per_frame[frame]; i++) {
        struct PCB *pcb = frame_users[frame][i].pcb;
        size_t page = frame_users[frame][i].virtual_page;
        
        pcb->page_table[page] = (size_t)(-1);
    }
    
    free(frame_users[frame]);
    frame_users[frame] = NULL;
    num_users_per_frame[frame] = 0;
}

void load_page(struct PCB *pcb, size_t page_num, size_t frame) {
    FILE *file = fopen(pcb->backing_file, "rt");
    if (!file) {
        return;
    }

    for (size_t i = 0; i < page_num * FRAME_SIZE; i++) {
        char buf[MAX_USER_INPUT];
        if (!fgets(buf, MAX_USER_INPUT, file)) {
            break;
        }
    }

    for (size_t i = 0; i < FRAME_SIZE; i++) {
        char buf[MAX_USER_INPUT];
        if (fgets(buf, MAX_USER_INPUT, file)) {
            size_t line_index = frame * FRAME_SIZE + i;
            allocate_line_at(line_index, buf);
        } else {
            allocate_line_at(frame * FRAME_SIZE + i, "");
        }
    }

    frame_allocation_table[frame] = 1;
    pcb->page_table[page_num] = frame;
    register_frame_user(frame, pcb, page_num);
    
    fclose(file);
}
