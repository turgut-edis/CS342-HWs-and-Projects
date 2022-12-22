#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include <sys/time.h>
#include <unistd.h>

char *algorithm;

char *burst_dist;
struct timeval start_time;
int quantum;
int burst_len;
int min_burst;
int max_burst;
int IOt1;
int IOt2;
int maxP;
int allP;
int out_mode;
float prob0;
float prob1;
float prob2;
float probG;

struct CPU
{
    int curr_pid;
};

struct IO
{
    int burst;
    int curr_pid;
};

struct PCB
{
    // Next pointer
    struct PCB *next;

    // Data section
    int pid;
    pthread_t thread_id;
    char *state;
    int burst_len;
    int remaining_burst_len; // For RR only
    int num_executed_cpu;
    int time_spent_ready;
    int num_IO1_visited;
    int num_IO2_visited;
    int start, finish;
    int total_exec_time;
    int quantum;
    int last;
};

struct ready_queue
{
    struct PCB *head;
    struct PCB *head1;
    int count; // PCB count created
    int remaining;
    pthread_mutex_t th_ready_queue;
    pthread_mutex_t mutex;
    pthread_cond_t sleep_cond;
    pthread_cond_t th_cond_gen;
    pthread_cond_t th_cond_sch;
};

struct ready_queue *buffer;

/* List Operations */
void ready_init(struct ready_queue **buf)
{
    (*buf)->head = NULL;
    (*buf)->head1 = NULL;
    (*buf)->count = 0;
    (*buf)->remaining = 0;
    pthread_mutex_init(&((*buf)->mutex), NULL);
    pthread_mutex_init(&((*buf)->th_ready_queue), NULL);
    pthread_cond_init(&((*buf)->sleep_cond), NULL);
    pthread_cond_init(&((*buf)->th_cond_gen), NULL);
    pthread_cond_init(&((*buf)->th_cond_sch), NULL);
}

void pcb_addItem(struct ready_queue **q, struct PCB **e)
{

    if ((*q)->count == 0)
    {
        (*q)->head = (*e);
        (*q)->head1 = (*e);
    }
    else
    {
        struct PCB *temp = (*q)->head;
        while (temp->next != NULL)
            temp = temp->next;
        temp->next = (*e);
    }

    (*q)->count++;
    (*q)->remaining++;
}

void pcb_deleteItem(struct ready_queue **q, struct PCB **e)
{

    if ((*q)->count == 0)
    {
        (*e) = NULL;
        return;
    }

    (*e) = (*q)->head;
    if ((*q)->head != NULL)
        (*q)->head = (*q)->head->next;
    (*q)->remaining--;

    return;
}
/* ----------------------------------------------------- */

/* Function used by generator thread */

void add_queue(struct ready_queue **q, struct PCB **e)
{
    pcb_addItem(q, e);
}

/* Function used by scheduler */

/* Function used by scheduler */
void FCFS(struct ready_queue **q)
{
    int arrTime1, arrTime2;
    struct PCB *tmp = (*q)->head;
    struct PCB *tmp2 = NULL;

    while (tmp != NULL)
    {
        arrTime1 = tmp->start;
        if (tmp->next != NULL)
        {
            arrTime2 = tmp->next->start;
            if (arrTime1 - arrTime2 > 0)
            {
                tmp2 = tmp->next;
                tmp->next = tmp2->next;
                tmp2->next = tmp;
                tmp = tmp2;
            }
        }
        tmp = tmp->next;
    }
}

void SJF(struct ready_queue **q)
{

    if ((*q)->head == NULL || (*q)->count == 1)
    {
        return;
    }
    int burst_len1, burst_len2;

    struct PCB *tmp = (*q)->head;
    struct PCB *tmp2 = NULL;
    while (tmp != NULL)
    {
        burst_len1 = tmp->burst_len;
        if (tmp->next != NULL)
        {
            burst_len2 = tmp->next->burst_len;
            if (burst_len1 > burst_len2)
            {
                tmp2 = tmp->next;
                tmp->next = tmp2->next;
                tmp2->next = tmp;
                tmp = tmp2;
            }
        }
        tmp = tmp->next;
    }
}

void RR(struct ready_queue **q)
{
    int arrTime1, arrTime2;
    struct PCB *tmp = (*q)->head;
    struct PCB *tmp2 = NULL;

    while (tmp != NULL)
    {
        arrTime1 = tmp->start;
        if (tmp->next != NULL)
        {
            arrTime2 = tmp->next->start;
            if (arrTime1 - arrTime2 > 0)
            {
                tmp2 = tmp->next;
                tmp->next = tmp2->next;
                tmp2->next = tmp;
                tmp = tmp2;
            }
        }
        tmp = tmp->next;
    }
}

void schedule(struct ready_queue **q, struct PCB **e)
{
    // pthread_mutex_lock(&(*q)->th_ready_queue);

    if (strcmp(algorithm, "FCFS") == 0)
    {
        FCFS(q);
        pcb_deleteItem(q, e);
    }
    else if (strcmp(algorithm, "SJF") == 0)
    {
        SJF(q);
        pcb_deleteItem(q, e);
    }
    else
    {
        RR(q);
        pcb_deleteItem(q, e);
    }
    // pthread_mutex_unlock(&((*q)->th_ready_queue));
}
/* -------------------------------------------------------- */

/* Calculate the burst lengths */
int calculateLengths()
{
    
    if (strcmp(burst_dist, "fixed") == 0)
    {
        return burst_len;
    }
    else if (strcmp(burst_dist, "exponential") == 0)
    {

        double lambda = 1.0 / (double)burst_len;
        srand(time(0));
        double u = (double)rand() / (double)RAND_MAX;
        
        int x = (int)(((-1) * log(u)) / lambda);
        
        while (x > max_burst || x < min_burst)
        {
            lambda = 1 / burst_len;
            u = (double)rand() / (double)RAND_MAX;
            x = ((-1) * log(u)) / lambda;
        }
        
        return x;
    }
    else
    {
        int diff = max_burst - min_burst;
        srand(time(0));
        int u = rand() % diff + min_burst;
        return u;
    }
}

double standardize(double num)
{
    double sum = 0.0;
    sum += prob0;
    sum += prob1;
    sum += prob2;
    return (num / sum);
}

void *process_func(void *arg)
{
    struct timeval t_start, t_ready;
    gettimeofday(&t_start, NULL);
    struct PCB *pcb = *((struct PCB **)arg);

    pthread_mutex_lock(&buffer->mutex);
    while (buffer->count < allP)
        pthread_cond_wait(&buffer->sleep_cond, &buffer->mutex);
    strcpy(pcb->state, "RUNNING");
    gettimeofday(&t_ready, NULL);
    pcb->time_spent_ready = ((t_ready.tv_sec - t_start.tv_sec) * 1000) + ( (t_ready.tv_usec - t_start.tv_usec) / 1000 );
    struct timeval tmp;
    if (out_mode == 2)
    {
        struct timeval t1;
        gettimeofday(&t1, NULL);
        int time = ((t1.tv_sec - start_time.tv_sec) * 1000) + ( (t1.tv_usec - start_time.tv_usec) / 1000 );
        printf("%d\t%d\t%s\n", time, pcb->pid, pcb->state);
    }
    if (out_mode == 3)
    {
        printf("Process %d is running in CPU.\n", pcb->pid);
    }
    if (strcmp(algorithm, "RR") != 0)
    {

        usleep(pcb->burst_len * 1000);
        gettimeofday(&tmp, NULL);
        pcb->total_exec_time = ((tmp.tv_sec - t_ready.tv_sec) * 1000) + ( (tmp.tv_usec - t_ready.tv_usec) / 1000 );
        pcb->finish = ((tmp.tv_sec - start_time.tv_sec) * 1000) + ( (tmp.tv_usec - start_time.tv_usec) / 1000 );
        pcb->last = 0;
        pcb->num_executed_cpu++;
        if (out_mode == 3)
        {
            printf("Process %d is finished.\n", pcb->pid);
        }
        pthread_mutex_unlock(&buffer->mutex);
        pthread_exit(NULL);
    }
    else
    {
        if (pcb->remaining_burst_len > quantum)
        {
            usleep(quantum * 1000);
            pcb->last = 1;
            strcpy(pcb->state, "WAITING");
            pthread_mutex_unlock(&buffer->mutex);
            pthread_exit(NULL);
        }
        else
        {
            usleep(pcb->remaining_burst_len * 1000);
            gettimeofday(&tmp, NULL);
            pcb->finish = ((tmp.tv_sec - start_time.tv_sec) * 1000) + ( (tmp.tv_usec - start_time.tv_usec) / 1000 );
            pcb->last = 0;
            if (out_mode == 3)
            {
                printf("Process %d is finished.\n", pcb->pid);
            }
            pthread_mutex_unlock(&buffer->mutex);
            pthread_exit(NULL);
        }
    }
}

void *gen_func(void *arg)
{
    gettimeofday(&start_time, NULL);
    pthread_mutex_lock(&buffer->th_ready_queue);
    while (buffer->count < 0)
        pthread_cond_wait(&buffer->th_cond_gen, &buffer->th_ready_queue);

    if (maxP >= 10)
    {
        for (int i = 0; i < 10; i++)
        {
            struct PCB *pcb = (struct PCB *)malloc(sizeof(struct PCB));
            pcb->burst_len = calculateLengths();
            pcb->last = 1;
            pcb->num_executed_cpu = 0;
            pcb->num_IO1_visited = 0;
            pcb->num_IO2_visited = 0;
            pcb->pid = i + 1;
            pcb->remaining_burst_len = burst_len;
            pcb->state = malloc(10);
            strcpy(pcb->state, "READY");
            pcb->time_spent_ready = 0;
            struct timeval t;
            gettimeofday(&t, NULL);
            pcb->start = ( (t.tv_sec - start_time.tv_sec) * 1000 ) + ( (t.tv_usec - start_time.tv_usec) / 1000 );
            pcb->total_exec_time = 0;
            add_queue(&buffer, &pcb);
            pthread_create(&pcb->thread_id, NULL, process_func, &pcb);
            if (out_mode == 3)
            {
                printf("New process %d created.\n", pcb->pid);
                printf("PCB is added to ready queue");
            }
        }
        while (buffer->count < maxP)
        {
            usleep(5000);
            srand((unsigned)time(0));
            double a = (double)rand() / (double)RAND_MAX;
            if (a <= probG)
            {
                struct PCB *pcb = (struct PCB *)malloc(sizeof(struct PCB));
                pcb->burst_len = calculateLengths();
                pcb->last = 1;
                pcb->num_executed_cpu = 0;
                pcb->num_IO1_visited = 0;
                pcb->num_executed_cpu = 0;
                pcb->num_IO2_visited = 0;
                pcb->pid = buffer->count + 1;
                pcb->remaining_burst_len = burst_len;
                pcb->state = malloc(10);
                strcpy(pcb->state, "READY");
                pcb->time_spent_ready = 0;
                struct timeval t;
                gettimeofday(&t, NULL);
                pcb->start = ( (t.tv_sec - start_time.tv_sec) * 1000 ) + ( (t.tv_usec - start_time.tv_usec) / 1000 );
                pcb->total_exec_time = 0;

                add_queue(&buffer, &pcb);
                pthread_create(&pcb->thread_id, NULL, process_func, &pcb);
                if (out_mode == 3)
                {
                    printf("New process %d created.\n", pcb->pid);
                    printf("PCB is added to ready queue");
                }
            }
        }
        while (buffer->count < allP)
        {
            usleep(5000);
            struct PCB *pcb = (struct PCB *)malloc(sizeof(struct PCB));
            pcb->burst_len = calculateLengths();
            pcb->last = 1;
            pcb->num_executed_cpu = 0;
            pcb->num_IO1_visited = 0;
            pcb->num_executed_cpu = 0;
            pcb->num_IO2_visited = 0;
            pcb->pid = buffer->count + 1;
            pcb->remaining_burst_len = burst_len;
            pcb->state = malloc(10);
            strcpy(pcb->state, "READY");
            pcb->time_spent_ready = 0;
            struct timeval t;
            gettimeofday(&t, NULL);
            pcb->start = ( (t.tv_sec - start_time.tv_sec) * 1000 ) + ( (t.tv_usec - start_time.tv_usec) / 1000 );
            pcb->total_exec_time = 0;

            add_queue(&buffer, &pcb);
            pthread_create(&pcb->thread_id, NULL, process_func, &pcb);
            if (out_mode == 3)
            {
                printf("New process %d created.\n", pcb->pid);
                printf("PCB is added to ready queue");
            }
        }
    }
    else
    {
        for (int i = 0; i < maxP; i++)
        {
            struct PCB *pcb = (struct PCB *)malloc(sizeof(struct PCB));
            pcb->burst_len = calculateLengths();
            pcb->last = 1;
            pcb->num_executed_cpu = 0;
            pcb->num_IO1_visited = 0;
            pcb->num_executed_cpu = 0;
            pcb->num_IO2_visited = 0;
            pcb->pid = i + 1;
            pcb->remaining_burst_len = burst_len;
            pcb->state = malloc(10);
            strcpy(pcb->state, "READY");
            pcb->time_spent_ready = 0;
            struct timeval t;
            gettimeofday(&t, NULL);
            pcb->start = ( (t.tv_sec - start_time.tv_sec) * 1000 ) + ( (t.tv_usec - start_time.tv_usec) / 1000 );
            pcb->total_exec_time = 0;

            add_queue(&buffer, &pcb);
            pthread_create(&pcb->thread_id, NULL, process_func, &pcb);
            if (out_mode == 3)
            {
                printf("New process %d created.\n", pcb->pid);
                printf("PCB is added to ready queue");
            }
        }

        while (buffer->count < allP)
        {
            usleep(5000);
            struct PCB *pcb = (struct PCB *)malloc(sizeof(struct PCB));
            pcb->burst_len = calculateLengths();
            pcb->last = 1;
            pcb->num_executed_cpu = 0;
            pcb->num_IO1_visited = 0;
            pcb->num_executed_cpu = 0;
            pcb->num_IO2_visited = 0;
            pcb->pid = buffer->count + 1;
            pcb->remaining_burst_len = burst_len;
            pcb->state = malloc(10);
            strcpy(pcb->state, "READY");
            pcb->time_spent_ready = 0;
            struct timeval t;
            gettimeofday(&t, NULL);
            pcb->start = ( (t.tv_sec - start_time.tv_sec) * 1000 ) + ( (t.tv_usec - start_time.tv_usec) / 1000 );
            pcb->total_exec_time = 0;

            add_queue(&buffer, &pcb);
            pthread_create(&pcb->thread_id, NULL, process_func, &pcb);
            if (out_mode == 3)
            {
                printf("New process %d created.\n", pcb->pid);
                printf("PCB is added to ready queue");
            }
        }
    }

    pthread_cond_signal(&buffer->th_cond_sch);

    pthread_mutex_unlock(&buffer->th_ready_queue);

    pthread_exit(NULL);
}

void *sche_func(void *arg)
{
    pthread_mutex_lock(&buffer->mutex);
    pthread_mutex_lock(&buffer->th_ready_queue);

    while (buffer->count < allP)
        pthread_cond_wait(&buffer->th_cond_sch, &buffer->th_ready_queue);

    struct PCB *pcb = NULL;
    schedule(&buffer, &pcb);

    pthread_cond_broadcast(&buffer->sleep_cond);
    pthread_mutex_unlock(&buffer->mutex);
    pthread_mutex_unlock(&buffer->th_ready_queue);
    pthread_cond_broadcast(&buffer->sleep_cond);

    while (pcb != NULL)
    {
        
        if (out_mode == 3)
        {
            printf("Process %d is selected for CPU.\n", pcb->pid);
        }
        pthread_join(pcb->thread_id, NULL);
        if (strcmp(algorithm, "RR") == 0)
        {
            if (pcb->last == 1)
            {
                pcb->remaining_burst_len -= quantum;
                add_queue(&buffer, &pcb);
                buffer->count--;
                printf("%d", buffer->count);
                pthread_create(&pcb->thread_id, NULL, process_func, &pcb);
            }
        }
        schedule(&buffer, &pcb);
    }

    pthread_mutex_unlock(&buffer->th_ready_queue);
    pthread_mutex_unlock(&buffer->mutex);
    pthread_exit(NULL);
}
int main(int argc, char *argv[])
{
    algorithm = malloc(10);
    burst_dist = malloc(25);
    pthread_t generator, scheduler;
    int ret;
    strcpy(algorithm, argv[1]);
    if (strcmp(algorithm, "RR") != 0)
    {
        quantum = 0;
    }
    else
    {
        quantum = atoi(argv[2]);
    }
    IOt1 = atoi(argv[3]);
    if (IOt1 < 30 || IOt1 > 100)
    {
        printf("The IO1 time should be in range 30 and 100\n");
        exit(1);
    }
    IOt2 = atoi(argv[4]);
    if (IOt2 < 100 || IOt2 > 300)
    {
        printf("The IO2 time should be in range 100 and 300\n");
        exit(1);
    }
    strcpy(burst_dist, argv[5]);
    if (strcmp(burst_dist, "fixed") != 0 && strcmp(burst_dist, "exponential") != 0 && strcmp(burst_dist, "uniform") != 0)
    {
        printf("The burst distribution is not supported. Try: fixed, exponential or uniform");
        exit(1);
    }
    burst_len = atoi(argv[6]);
    min_burst = atoi(argv[7]);
    max_burst = atoi(argv[8]);
    prob0 = atof(argv[9]);
    prob1 = atof(argv[10]);
    prob2 = atof(argv[11]);
    prob0 = standardize(atof(argv[9]));
    prob1 = standardize(atof(argv[10]));
    prob2 = standardize(atof(argv[11]));
    probG = atof(argv[12]);
    maxP = atoi(argv[13]);
    if (maxP > 50 || maxP < 1)
    {
        printf("The max process should be in range 1 and 50");
        exit(1);
    }
    allP = atoi(argv[14]);
    if (allP > 1000 || allP < 1)
    {
        printf("The all process should be in range 1 and 1000");
        exit(1);
    }
    out_mode = atoi(argv[15]);
    if (out_mode != 1 && out_mode != 2 && out_mode != 3)
    {
        printf("Invalid out mode");
        exit(1);
    }

    buffer = (struct ready_queue *)malloc(sizeof(struct ready_queue));
    ready_init(&(buffer));
    pthread_mutex_init(&buffer->th_ready_queue, NULL);
    pthread_mutex_init(&buffer->mutex, NULL);
    pthread_cond_init(&buffer->th_cond_gen, NULL);
    pthread_cond_init(&buffer->th_cond_sch, NULL);
    pthread_cond_init(&buffer->sleep_cond, NULL);

    ret = pthread_create(&generator, NULL, gen_func, NULL);
    if (ret != 0)
    {
        perror("thread create failed");
        exit(1);
    }

    ret = pthread_create(&scheduler, NULL, sche_func, NULL);
    if (ret != 0)
    {
        perror("thread create failed");
        exit(1);
    }

    pthread_join(generator, NULL);
    pthread_join(scheduler, NULL);
    printf("\n");
    printf("pid\tarv\tdept\tcpu\twaitr\tturna\tn-bursts\tn-d1\tn-d2\n");
    struct PCB *tmp = buffer->head1;
    
    while (tmp != NULL)
    {
        printf("%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n", tmp->pid, tmp->start, tmp->finish, tmp->total_exec_time, tmp->time_spent_ready, (tmp->finish - tmp->start), tmp->num_executed_cpu, tmp->num_IO1_visited, tmp->num_IO2_visited);
        tmp = tmp->next;
        free(buffer->head1);
        buffer->head1 = tmp;
    }

    free(buffer);

    pthread_mutex_destroy(&buffer->mutex);
    pthread_mutex_destroy(&buffer->th_ready_queue);
    pthread_cond_destroy(&buffer->th_cond_gen);
    pthread_cond_destroy(&buffer->th_cond_sch);
    pthread_cond_destroy(&buffer->sleep_cond);

    return 0;
}
