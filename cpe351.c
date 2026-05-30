#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Structure for a process
struct Process {
    int id;
    int burst;
    int arrival;
    int priority;
    int waiting;
    int remaining;
    struct Process* next;
};

struct Process* create_process(int id, int burst, int arrival, int priority) {
    struct Process* p = (struct Process*)malloc(sizeof(struct Process));
    p->id = id;
    p->burst = burst;
    p->arrival = arrival;
    p->priority = priority;
    p->waiting = 0;
    p->remaining = burst;
    p->next = NULL;
    return p;
}

// ALGORITHM 1: FCFS
void fcfs(struct Process* head, int n, FILE* out) {
    int* wt = (int*)malloc(n * sizeof(int));
    int time = 0, total = 0;
    struct Process* curr = head;
    
    while (curr) {
        if (time < curr->arrival) time = curr->arrival;
        wt[curr->id] = time - curr->arrival;
        total += wt[curr->id];
        time += curr->burst;
        curr = curr->next;
    }
    
    fprintf(out, "1");
    for (int i = 0; i < n; i++) fprintf(out, " %d", wt[i]);
    fprintf(out, " %.2f\n", (float)total/n);
    free(wt);
}

// ALGORITHM 2: SJF Non-Preemptive
void sjf_nonpre(struct Process* head, int n, FILE* out) {
    int* wt = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) wt[i] = 0;
    
    struct Process* list = NULL;
    struct Process* curr = head;
    while (curr) {
        struct Process* copy = create_process(curr->id, curr->burst, curr->arrival, curr->priority);
        copy->next = list;
        list = copy;
        curr = curr->next;
    }
    
    int time = 0, completed = 0, total = 0;
    
    while (completed < n) {
        struct Process* shortest = NULL;
        struct Process* prev = NULL;
        struct Process* sprev = NULL;
        curr = list;
        struct Process* pprev = NULL;
        
        while (curr) {
            if (curr->arrival <= time && curr->burst > 0) {
                if (!shortest || curr->burst < shortest->burst) {
                    shortest = curr;
                    sprev = pprev;
                }
            }
            pprev = curr;
            curr = curr->next;
        }
        
        if (!shortest) { time++; continue; }
        
        if (sprev) sprev->next = shortest->next;
        else list = shortest->next;
        
        wt[shortest->id] = time - shortest->arrival;
        total += wt[shortest->id];
        time += shortest->burst;
        completed++;
        free(shortest);
    }
    
    fprintf(out, "2");
    for (int i = 0; i < n; i++) fprintf(out, " %d", wt[i]);
    fprintf(out, " %.2f\n", (float)total/n);
    free(wt);
}

// ALGORITHM 3: SJF Preemptive (SRTF)
void sjf_pre(struct Process* head, int n, FILE* out) {
    struct Process* copies[10];
    struct Process* curr = head;
    for (int i = 0; i < n; i++) {
        copies[i] = create_process(curr->id, curr->burst, curr->arrival, curr->priority);
        copies[i]->remaining = curr->burst;
        curr = curr->next;
    }
    
    int* wt = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) wt[i] = 0;
    
    int time = 0, completed = 0, total = 0;
    
    while (completed < n) {
        int idx = -1, min_rem = 999999;
        for (int i = 0; i < n; i++) {
            if (copies[i]->arrival <= time && copies[i]->remaining > 0) {
                if (copies[i]->remaining < min_rem) {
                    min_rem = copies[i]->remaining;
                    idx = i;
                }
            }
        }
        
        if (idx == -1) { time++; continue; }
        
        copies[idx]->remaining--;
        time++;
        
        for (int i = 0; i < n; i++) {
            if (i != idx && copies[i]->arrival <= time && copies[i]->remaining > 0) {
                wt[i]++;
                total++;
            }
        }
        
        if (copies[idx]->remaining == 0) {
            wt[idx] = time - copies[idx]->arrival - copies[idx]->burst;
            completed++;
        }
    }
    
    fprintf(out, "3");
    for (int i = 0; i < n; i++) fprintf(out, " %d", wt[i]);
    fprintf(out, " %.2f\n", (float)total/n);
    free(wt);
    for (int i = 0; i < n; i++) free(copies[i]);
}

// ALGORITHM 4: Priority Non-Preemptive
void priority_nonpre(struct Process* head, int n, FILE* out) {
    int* wt = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) wt[i] = 0;
    
    struct Process* list = NULL;
    struct Process* curr = head;
    while (curr) {
        struct Process* copy = create_process(curr->id, curr->burst, curr->arrival, curr->priority);
        copy->next = list;
        list = copy;
        curr = curr->next;
    }
    
    int time = 0, completed = 0, total = 0;
    
    while (completed < n) {
        struct Process* highest = NULL;
        struct Process* prev = NULL;
        struct Process* hprev = NULL;
        curr = list;
        struct Process* pprev = NULL;
        
        while (curr) {
            if (curr->arrival <= time && curr->burst > 0) {
                if (!highest || curr->priority < highest->priority) {
                    highest = curr;
                    hprev = pprev;
                }
            }
            pprev = curr;
            curr = curr->next;
        }
        
        if (!highest) { time++; continue; }
        
        if (hprev) hprev->next = highest->next;
        else list = highest->next;
        
        wt[highest->id] = time - highest->arrival;
        total += wt[highest->id];
        time += highest->burst;
        completed++;
        free(highest);
    }
    
    fprintf(out, "4");
    for (int i = 0; i < n; i++) fprintf(out, " %d", wt[i]);
    fprintf(out, " %.2f\n", (float)total/n);
    free(wt);
}

// ALGORITHM 5: Priority Preemptive
void priority_pre(struct Process* head, int n, FILE* out) {
    struct Process* copies[10];
    struct Process* curr = head;
    for (int i = 0; i < n; i++) {
        copies[i] = create_process(curr->id, curr->burst, curr->arrival, curr->priority);
        copies[i]->remaining = curr->burst;
        curr = curr->next;
    }
    
    int* wt = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) wt[i] = 0;
    
    int time = 0, completed = 0, total = 0;
    
    while (completed < n) {
        int idx = -1, high_pri = 999999;
        for (int i = 0; i < n; i++) {
            if (copies[i]->arrival <= time && copies[i]->remaining > 0) {
                if (copies[i]->priority < high_pri) {
                    high_pri = copies[i]->priority;
                    idx = i;
                }
            }
        }
        
        if (idx == -1) { time++; continue; }
        
        copies[idx]->remaining--;
        time++;
        
        for (int i = 0; i < n; i++) {
            if (i != idx && copies[i]->arrival <= time && copies[i]->remaining > 0) {
                wt[i]++;
                total++;
            }
        }
        
        if (copies[idx]->remaining == 0) {
            wt[idx] = time - copies[idx]->arrival - copies[idx]->burst;
            completed++;
        }
    }
    
    fprintf(out, "5");
    for (int i = 0; i < n; i++) fprintf(out, " %d", wt[i]);
    fprintf(out, " %.2f\n", (float)total/n);
    free(wt);
    for (int i = 0; i < n; i++) free(copies[i]);
}

// ALGORITHM 6: Round Robin
void round_robin(struct Process* head, int n, FILE* out, int quantum) {
    struct Process* copies[10];
    struct Process* curr = head;
    for (int i = 0; i < n; i++) {
        copies[i] = create_process(curr->id, curr->burst, curr->arrival, curr->priority);
        copies[i]->remaining = curr->burst;
        curr = curr->next;
    }
    
    int* wt = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) wt[i] = 0;
    
    int queue[100], front = 0, rear = 0;
    int in_queue[10] = {0};
    int time = 0, completed = 0, total = 0;
    
    while (completed < n) {
        for (int i = 0; i < n; i++) {
            if (copies[i]->arrival <= time && copies[i]->remaining > 0 && !in_queue[i]) {
                queue[rear++] = i;
                in_queue[i] = 1;
            }
        }
        
        if (front == rear) { time++; continue; }
        
        int idx = queue[front++];
        in_queue[idx] = 0;
        
        int run = (copies[idx]->remaining < quantum) ? copies[idx]->remaining : quantum;
        
        for (int i = front; i < rear; i++) {
            wt[queue[i]] += run;
            total += run;
        }
        
        time += run;
        copies[idx]->remaining -= run;
        
        for (int i = 0; i < n; i++) {
            if (copies[i]->arrival <= time && copies[i]->remaining > 0 && !in_queue[i]) {
                queue[rear++] = i;
                in_queue[i] = 1;
            }
        }
        
        if (copies[idx]->remaining > 0) {
            queue[rear++] = idx;
            in_queue[idx] = 1;
        } else {
            wt[idx] = time - copies[idx]->arrival - copies[idx]->burst;
            completed++;
        }
    }
    
    fprintf(out, "6");
    for (int i = 0; i < n; i++) fprintf(out, " %d", wt[i]);
    fprintf(out, " %.2f\n", (float)total/n);
    free(wt);
    for (int i = 0; i < n; i++) free(copies[i]);
}

int main() {
    printf("========================================\n");
    printf("CPU SCHEDULER SIMULATOR\n");
    printf("========================================\n\n");
    
    // Create processes (simulating input file)
    struct Process* head = NULL;
    struct Process* tail = NULL;
    
    // Input data: burst:arrival:priority
    // You can change these numbers
    int data[][3] = {
        {10, 0, 0},
        {5, 0, 0},
        {8, 1, 1},
        {4, 2, 2},
        {3, 3, 1}
    };
    int n = 5;
    
    for (int i = 0; i < n; i++) {
        struct Process* p = create_process(i, data[i][0], data[i][1], data[i][2]);
        if (!head) head = p;
        else tail->next = p;
        tail = p;
    }
    
    printf("Processes created:\n");
    struct Process* curr = head;
    while (curr) {
        printf("P%d: Burst=%d, Arrival=%d, Priority=%d\n", 
               curr->id, curr->burst, curr->arrival, curr->priority);
        curr = curr->next;
    }
    
    // Create output
    char output[5000] = "";
    FILE* out = fmemopen(output, sizeof(output), "w");
    
    // Header
    fprintf(out, "CPU Algorithm");
    for (int i = 0; i < n; i++) fprintf(out, " WT%d", i+1);
    fprintf(out, " AWT\n");
    
    // Run all algorithms
    fcfs(head, n, out);
    sjf_nonpre(head, n, out);
    sjf_pre(head, n, out);
    priority_nonpre(head, n, out);
    priority_pre(head, n, out);
    round_robin(head, n, out, 2);
    
    fclose(out);
    
    printf("\n========================================\n");
    printf("OUTPUT (Copy this to your output file)\n");
    printf("========================================\n\n");
    printf("%s", output);
    
    // Cleanup
    curr = head;
    while (curr) {
        struct Process* temp = curr;
        curr = curr->next;
        free(temp);
    }
    
    printf("\n========================================\n");
    printf("SIMULATION COMPLETE\n");
    printf("========================================\n");
    
    return 0;
}