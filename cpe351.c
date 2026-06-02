#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Process structure - using linked list (NO arrays)
struct Process {
    int id;
    int burst;
    int arrival;
    int priority;
    int waiting;
    int remaining;
    int completed;
    int original_burst;
    int original_priority;
    struct Process* next;
};

// Queue structure using linked list (NO arrays)
struct Queue {
    struct Process* head;
    struct Process* tail;
    int size;
};

// Initialize queue
void init_queue(struct Queue* q) {
    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
}

// Enqueue to end of queue
void enqueue(struct Queue* q, struct Process* p) {
    if (!p) return;
    p->next = NULL;
    if (!q->head) {
        q->head = p;
        q->tail = p;
    } else {
        q->tail->next = p;
        q->tail = p;
    }
    q->size++;
}

// Dequeue from front of queue
struct Process* dequeue(struct Queue* q) {
    if (!q->head) return NULL;
    struct Process* p = q->head;
    q->head = q->head->next;
    if (!q->head) q->tail = NULL;
    q->size--;
    p->next = NULL;
    return p;
}

// Check if queue is empty
int is_empty(struct Queue* q) {
    return q->head == NULL;
}

// Copy a single process
struct Process* copy_process(struct Process* src) {
    struct Process* p = (struct Process*)malloc(sizeof(struct Process));
    p->id = src->id;
    p->burst = src->burst;
    p->arrival = src->arrival;
    p->priority = src->priority;
    p->waiting = src->waiting;
    p->remaining = src->remaining;
    p->completed = src->completed;
    p->original_burst = src->original_burst;
    p->original_priority = src->original_priority;
    p->next = NULL;
    return p;
}

// Copy entire queue
struct Queue* copy_queue(struct Queue* src) {
    struct Queue* dest = (struct Queue*)malloc(sizeof(struct Queue));
    init_queue(dest);
    struct Process* curr = src->head;
    while (curr) {
        enqueue(dest, copy_process(curr));
        curr = curr->next;
    }
    return dest;
}

// Read processes from file
struct Queue* read_processes(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) return NULL;
    
    struct Queue* queue = (struct Queue*)malloc(sizeof(struct Queue));
    init_queue(queue);
    
    char line[256];
    int id = 0;
    
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '\n' || line[0] == '\0') continue;
        
        struct Process* p = (struct Process*)malloc(sizeof(struct Process));
        p->id = id++;
        p->next = NULL;
        
        sscanf(line, "%d:%d:%d", &p->burst, &p->arrival, &p->priority);
        p->original_burst = p->burst;
        p->original_priority = p->priority;
        p->remaining = p->burst;
        p->waiting = 0;
        p->completed = 0;
        
        enqueue(queue, p);
    }
    
    fclose(file);
    return queue;
}

// Function to insert waiting time in order by ID (NO arrays!)
void insert_ordered(struct Queue* ordered, int id, int waiting) {
    struct Process* newp = (struct Process*)malloc(sizeof(struct Process));
    newp->id = id;
    newp->waiting = waiting;
    newp->next = NULL;
    
    if (is_empty(ordered) || ordered->head->id > id) {
        newp->next = ordered->head;
        ordered->head = newp;
        if (!ordered->tail) ordered->tail = newp;
        if (ordered->tail == ordered->head) ordered->tail = newp;
        ordered->size++;
        return;
    }
    
    struct Process* curr = ordered->head;
    while (curr->next && curr->next->id < id) {
        curr = curr->next;
    }
    newp->next = curr->next;
    curr->next = newp;
    if (!newp->next) ordered->tail = newp;
    ordered->size++;
}

// ALGORITHM 1: FCFS
void fcfs(struct Queue* processes, FILE* out) {
    struct Queue* result = (struct Queue*)malloc(sizeof(struct Queue));
    init_queue(result);
    
    struct Queue* copy = copy_queue(processes);
    
    int current_time = 0;
    int total_waiting = 0;
    int n = processes->size;
    int completed = 0;
    
    fprintf(out, "1");
    printf("1");
    
    while (completed < n) {
        struct Process* selected = NULL;
        struct Process* prev = NULL;
        struct Process* selected_prev = NULL;
        struct Process* curr = copy->head;
        prev = NULL;
        
        while (curr) {
            if (curr->arrival <= current_time && !curr->completed) {
                if (!selected || curr->arrival < selected->arrival) {
                    selected = curr;
                    selected_prev = prev;
                }
            }
            prev = curr;
            curr = curr->next;
        }
        
        if (!selected) {
            current_time++;
            continue;
        }
        
        if (selected_prev) selected_prev->next = selected->next;
        else copy->head = selected->next;
        if (copy->tail == selected) copy->tail = selected_prev;
        
        selected->waiting = current_time - selected->arrival;
        if (selected->waiting < 0) selected->waiting = 0;
        total_waiting += selected->waiting;
        
        current_time += selected->burst;
        selected->completed = 1;
        completed++;
        
        insert_ordered(result, selected->id, selected->waiting);
    }
    
    // Print waiting times in order
    struct Process* curr = result->head;
    while (curr) {
        fprintf(out, ":%d", curr->waiting);
        printf(":%d", curr->waiting);
        curr = curr->next;
    }
    
    float awt = (float)total_waiting / n;
    fprintf(out, ":%.2f\n", awt);
    printf(":%.2f\n", awt);
    
    while (!is_empty(copy)) free(dequeue(copy));
    while (!is_empty(result)) free(dequeue(result));
    free(copy);
    free(result);
}

// ALGORITHM 2: SJF Non-preemptive
void sjf_nonpreemptive(struct Queue* processes, FILE* out) {
    struct Queue* result = (struct Queue*)malloc(sizeof(struct Queue));
    init_queue(result);
    
    struct Queue* copy = copy_queue(processes);
    
    int current_time = 0;
    int total_waiting = 0;
    int n = processes->size;
    int completed = 0;
    
    fprintf(out, "2");
    printf("2");
    
    while (completed < n) {
        struct Process* shortest = NULL;
        struct Process* prev = NULL;
        struct Process* shortest_prev = NULL;
        struct Process* curr = copy->head;
        prev = NULL;
        
        while (curr) {
            if (curr->arrival <= current_time && !curr->completed) {
                if (!shortest || curr->burst < shortest->burst) {
                    shortest = curr;
                    shortest_prev = prev;
                }
            }
            prev = curr;
            curr = curr->next;
        }
        
        if (!shortest) {
            current_time++;
            continue;
        }
        
        if (shortest_prev) shortest_prev->next = shortest->next;
        else copy->head = shortest->next;
        if (copy->tail == shortest) copy->tail = shortest_prev;
        
        shortest->waiting = current_time - shortest->arrival;
        if (shortest->waiting < 0) shortest->waiting = 0;
        total_waiting += shortest->waiting;
        
        current_time += shortest->burst;
        shortest->completed = 1;
        completed++;
        
        insert_ordered(result, shortest->id, shortest->waiting);
    }
    
    struct Process* curr = result->head;
    while (curr) {
        fprintf(out, ":%d", curr->waiting);
        printf(":%d", curr->waiting);
        curr = curr->next;
    }
    
    float awt = (float)total_waiting / n;
    fprintf(out, ":%.2f\n", awt);
    printf(":%.2f\n", awt);
    
    while (!is_empty(copy)) free(dequeue(copy));
    while (!is_empty(result)) free(dequeue(result));
    free(copy);
    free(result);
}

// ALGORITHM 3: SJF Preemptive (SRTF)
void sjf_preemptive(struct Queue* processes, FILE* out) {
    struct Queue* copy = copy_queue(processes);
    struct Queue* result = (struct Queue*)malloc(sizeof(struct Queue));
    init_queue(result);
    
    int current_time = 0;
    int completed = 0;
    int total_waiting = 0;
    int n = processes->size;
    
    fprintf(out, "3");
    printf("3");
    
    while (completed < n) {
        struct Process* shortest = NULL;
        struct Process* curr = copy->head;
        
        while (curr) {
            if (curr->arrival <= current_time && !curr->completed && curr->remaining > 0) {
                if (!shortest || curr->remaining < shortest->remaining) {
                    shortest = curr;
                }
            }
            curr = curr->next;
        }
        
        if (!shortest) {
            current_time++;
            continue;
        }
        
        if (shortest->remaining == shortest->burst) {
            shortest->waiting = current_time - shortest->arrival;
            if (shortest->waiting < 0) shortest->waiting = 0;
        }
        
        shortest->remaining--;
        current_time++;
        
        if (shortest->remaining == 0) {
            shortest->completed = 1;
            completed++;
            total_waiting += shortest->waiting;
            insert_ordered(result, shortest->id, shortest->waiting);
        }
    }
    
    struct Process* curr = result->head;
    while (curr) {
        fprintf(out, ":%d", curr->waiting);
        printf(":%d", curr->waiting);
        curr = curr->next;
    }
    
    float awt = (float)total_waiting / n;
    fprintf(out, ":%.2f\n", awt);
    printf(":%.2f\n", awt);
    
    while (!is_empty(copy)) free(dequeue(copy));
    while (!is_empty(result)) free(dequeue(result));
    free(copy);
    free(result);
}

// ALGORITHM 4: Priority Non-preemptive
void priority_nonpreemptive(struct Queue* processes, FILE* out) {
    struct Queue* result = (struct Queue*)malloc(sizeof(struct Queue));
    init_queue(result);
    
    struct Queue* copy = copy_queue(processes);
    
    int current_time = 0;
    int total_waiting = 0;
    int n = processes->size;
    int completed = 0;
    
    fprintf(out, "4");
    printf("4");
    
    while (completed < n) {
        struct Process* highest = NULL;
        struct Process* prev = NULL;
        struct Process* highest_prev = NULL;
        struct Process* curr = copy->head;
        prev = NULL;
        
        while (curr) {
            if (curr->arrival <= current_time && !curr->completed) {
                if (!highest || curr->priority < highest->priority) {
                    highest = curr;
                    highest_prev = prev;
                }
            }
            prev = curr;
            curr = curr->next;
        }
        
        if (!highest) {
            current_time++;
            continue;
        }
        
        if (highest_prev) highest_prev->next = highest->next;
        else copy->head = highest->next;
        if (copy->tail == highest) copy->tail = highest_prev;
        
        highest->waiting = current_time - highest->arrival;
        if (highest->waiting < 0) highest->waiting = 0;
        total_waiting += highest->waiting;
        
        current_time += highest->burst;
        highest->completed = 1;
        completed++;
        
        insert_ordered(result, highest->id, highest->waiting);
    }
    
    struct Process* curr = result->head;
    while (curr) {
        fprintf(out, ":%d", curr->waiting);
        printf(":%d", curr->waiting);
        curr = curr->next;
    }
    
    float awt = (float)total_waiting / n;
    fprintf(out, ":%.2f\n", awt);
    printf(":%.2f\n", awt);
    
    while (!is_empty(copy)) free(dequeue(copy));
    while (!is_empty(result)) free(dequeue(result));
    free(copy);
    free(result);
}

// ALGORITHM 5: Priority Preemptive
void priority_preemptive(struct Queue* processes, FILE* out) {
    struct Queue* copy = copy_queue(processes);
    struct Queue* result = (struct Queue*)malloc(sizeof(struct Queue));
    init_queue(result);
    
    int current_time = 0;
    int completed = 0;
    int total_waiting = 0;
    int n = processes->size;
    
    fprintf(out, "5");
    printf("5");
    
    while (completed < n) {
        struct Process* highest = NULL;
        struct Process* curr = copy->head;
        
        while (curr) {
            if (curr->arrival <= current_time && !curr->completed && curr->remaining > 0) {
                if (!highest || curr->priority < highest->priority) {
                    highest = curr;
                }
            }
            curr = curr->next;
        }
        
        if (!highest) {
            current_time++;
            continue;
        }
        
        if (highest->remaining == highest->burst) {
            highest->waiting = current_time - highest->arrival;
            if (highest->waiting < 0) highest->waiting = 0;
        }
        
        highest->remaining--;
        current_time++;
        
        if (highest->remaining == 0) {
            highest->completed = 1;
            completed++;
            total_waiting += highest->waiting;
            insert_ordered(result, highest->id, highest->waiting);
        }
    }
    
    struct Process* curr = result->head;
    while (curr) {
        fprintf(out, ":%d", curr->waiting);
        printf(":%d", curr->waiting);
        curr = curr->next;
    }
    
    float awt = (float)total_waiting / n;
    fprintf(out, ":%.2f\n", awt);
    printf(":%.2f\n", awt);
    
    while (!is_empty(copy)) free(dequeue(copy));
    while (!is_empty(result)) free(dequeue(result));
    free(copy);
    free(result);
}

// ALGORITHM 6: Round Robin
void round_robin(struct Queue* processes, int quantum, FILE* out) {
    struct Queue* ready = (struct Queue*)malloc(sizeof(struct Queue));
    init_queue(ready);
    
    struct Queue* copy = copy_queue(processes);
    struct Queue* result = (struct Queue*)malloc(sizeof(struct Queue));
    init_queue(result);
    
    int current_time = 0;
    int total_waiting = 0;
    int n = processes->size;
    
    fprintf(out, "6");
    printf("6");
    
    while (result->size < n) {
        struct Process* curr = copy->head;
        while (curr) {
            if (curr->arrival <= current_time && !curr->completed && curr->remaining > 0) {
                int in_queue = 0;
                struct Process* r = ready->head;
                while (r) {
                    if (r->id == curr->id) { in_queue = 1; break; }
                    r = r->next;
                }
                if (!in_queue) {
                    enqueue(ready, copy_process(curr));
                }
            }
            curr = curr->next;
        }
        
        if (is_empty(ready)) {
            current_time++;
            continue;
        }
        
        struct Process* current = dequeue(ready);
        
        if (current->remaining == current->burst) {
            current->waiting = current_time - current->arrival;
            if (current->waiting < 0) current->waiting = 0;
            total_waiting += current->waiting;
        }
        
        int run = (current->remaining < quantum) ? current->remaining : quantum;
        current_time += run;
        current->remaining -= run;
        
        if (current->remaining > 0) {
            enqueue(ready, current);
        } else {
            current->completed = 1;
            insert_ordered(result, current->id, current->waiting);
            free(current);
        }
    }
    
    struct Process* curr = result->head;
    while (curr) {
        fprintf(out, ":%d", curr->waiting);
        printf(":%d", curr->waiting);
        curr = curr->next;
    }
    
    float awt = (float)total_waiting / n;
    fprintf(out, ":%.2f\n", awt);
    printf(":%.2f\n", awt);
    
    while (!is_empty(ready)) free(dequeue(ready));
    while (!is_empty(result)) free(dequeue(result));
    while (!is_empty(copy)) free(dequeue(copy));
    free(ready);
    free(copy);
    free(result);
}

// Parse command line arguments
void parse_args(int argc, char* argv[], int* quantum, char* input_file, char* output_file) {
    int opt;
    *quantum = 2;
    input_file[0] = '\0';
    output_file[0] = '\0';
    
    while ((opt = getopt(argc, argv, "t:f:o:")) != -1) {
        switch(opt) {
            case 't': *quantum = atoi(optarg); break;
            case 'f': strcpy(input_file, optarg); break;
            case 'o': strcpy(output_file, optarg); break;
        }
    }
}

// MAIN FUNCTION
int main(int argc, char* argv[]) {
    int quantum;
    char input_file[256];
    char output_file[256];
    
    parse_args(argc, argv, &quantum, input_file, output_file);
    
    if (input_file[0] == '\0' || output_file[0] == '\0') {
        printf("Usage: %s -t <quantum> -f <input> -o <output>\n", argv[0]);
        printf("Example: %s -t 2 -f input.txt -o output.txt\n", argv[0]);
        return 1;
    }
    
    printf("========================================\n");
    printf("CPU SCHEDULER SIMULATOR\n");
    printf("========================================\n");
    printf("Time Quantum: %d\n", quantum);
    printf("Input File: %s\n", input_file);
    printf("Output File: %s\n", output_file);
    printf("=====================A===================\n\n");
    
    struct Queue* processes = read_processes(input_file);
    if (!processes) {
        printf("ERROR: Cannot read %s\n", input_file);
        return 1;
    }
    
    printf("Processes loaded: %d\n", processes->size);
    printf("\n--- RESULTS ---\n");
    printf("========================================\n");
    
    FILE* out = fopen(output_file, "w");
    if (!out) {
        printf("ERROR: Cannot create %s\n", output_file);
        return 1;
    }
    
    // Run all 6 scheduling algorithms
    fcfs(processes, out);
    sjf_nonpreemptive(processes, out);
    sjf_preemptive(processes, out);
    priority_nonpreemptive(processes, out);
    priority_preemptive(processes, out);
    round_robin(processes, quantum, out);
    
    fclose(out);
    
    printf("========================================\n");
    printf("\n✓ Output written to %s\n", output_file);
    
    // Cleanup
    while (!is_empty(processes)) free(dequeue(processes));
    free(processes);
    
    return 0;
}
