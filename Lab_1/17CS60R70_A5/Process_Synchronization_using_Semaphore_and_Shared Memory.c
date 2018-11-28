#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <time.h>


#define QUEUE_SIZE 3
#define PRODUCERS 1
#define CONSUMERS 1
#define CYCLES 8
#define MAX_SLEEPTIME 100000 /* in us */
#define TIMEOUT 1

struct _arg_t {
    int num;
};
typedef struct _arg_t arg_t;

struct _fifo_t {
    int val;
    struct _fifo_t *next, *prev;
};
typedef struct _fifo_t fifo_t;

struct _mem_t {
    fifo_t fifo[QUEUE_SIZE];
    fifo_t *head, *tail;
    sem_t fifo_lock;

    sem_t full, empty;
};
typedef struct _mem_t mem_t;

static mem_t *sh_mem;



/**
 * Allocates sh_mem and initialize it in anonymous shared memory.
 * Fifo is implemented as circular buffer.
 */
static int allocAndInit(void)
{
    fifo_t *f;
    size_t i;

    sh_mem = (mem_t *)mmap(0, sizeof(*sh_mem), PROT_READ | PROT_WRITE,
                           MAP_SHARED | MAP_ANON, -1, 0);
    if (!sh_mem){
        perror("Allocation of shared memory failed: ");
        return -1;
    }

    // create circular buffer at beggining of shared memory
    f = sh_mem->fifo;
    for (i = 0; i < QUEUE_SIZE; i++){
        f->next = f + 1;
        f->prev = f - 1;
        f++;
    }
    sh_mem->fifo[0].prev = &sh_mem->fifo[QUEUE_SIZE - 1];
    sh_mem->fifo[0].prev->next = sh_mem->fifo;

    // initialize semaphores
    sem_init(&sh_mem->fifo_lock, 1, 1);
    sem_init(&sh_mem->full, 1, 0);
    sem_init(&sh_mem->empty, 1, QUEUE_SIZE);

    return 0;
}

static void unalloc(void)
{
    if (sh_mem){
        sem_destroy(&sh_mem->fifo_lock);
        sem_destroy(&sh_mem->full);
        sem_destroy(&sh_mem->empty);
        munmap(sh_mem, sizeof(*sh_mem));
    }
}

/**
 * Inserts value into queue.
 * Returns 0 on success, -1 if on error and -2 if timeout was reached when
 * waiting to free space in queue.
 */
static int queueInsert(int val)
{
    fifo_t *f;
    int res;
    struct timespec to;

    // sem_wait(&empty);
    clock_gettime(CLOCK_REALTIME, &to);
    to.tv_sec += TIMEOUT;
    res = sem_timedwait(&sh_mem->empty, &to);
    if (res < 0){
        if (errno == ETIMEDOUT){
            return -2;
        }else{
            return -1;
        }
    }

    sem_wait(&sh_mem->fifo_lock);
    if (!sh_mem->head){
        sh_mem->head = sh_mem->tail = f = sh_mem->fifo;
    }else{
        f = sh_mem->head->prev;
        sh_mem->head = f;
    }
    f->val = val;
    sem_post(&sh_mem->fifo_lock);

    sem_post(&sh_mem->full);

    return 0;
}

/**
 * Removes value from queue.
 * Return 0 on success, -1 on error and -2 if timeout was reached.
 */
static int queueRemove(int *val)
{
    int res;
    struct timespec to;

    //sem_wait(&full);
    clock_gettime(CLOCK_REALTIME, &to);
    to.tv_sec += TIMEOUT;
    res = sem_timedwait(&sh_mem->full, &to);
    if (res < 0){
        if (errno == ETIMEDOUT){
            return -2;
        }else{
            return -1;
        }
    }

    sem_wait(&sh_mem->fifo_lock);
    *val = sh_mem->tail->val;
    if (sh_mem->tail == sh_mem->head){
        sh_mem->tail = sh_mem->head = NULL;
    }else{
        sh_mem->tail = sh_mem->tail->prev;
    }
    sem_post(&sh_mem->fifo_lock);

    sem_post(&sh_mem->empty);

    return 0;
}

static useconds_t sleeptime(void)
{
    float s;
    s  = rand();
    s /= RAND_MAX;
    s *= MAX_SLEEPTIME;
    return s;
}

static int producer(int num)
{
    int val, res;
    size_t i;
    useconds_t us;

    srand(time(NULL) + num);

    fprintf(stdout, "Producer %d started\n", num);

    for (i = 0; i < CYCLES; i++){
        val = rand();
        res = queueInsert(val);
        if (res < 0){
            if (res == -2)
                fprintf(stderr, "Producer[%d]: timedout\n", num);
            break;
        }

        fprintf(stdout, "Producer[%d]: Inserted %d\n", num, val);

        us = sleeptime();
        fprintf(stdout, "Producer[%d]: going to sleep for %d us\n", num, us);
        usleep(us);
    }

    return 0;
}

static int consumer(int num)
{
    int val, res;
    useconds_t us;

    srand(time(NULL) + num);

    fprintf(stdout, "Consumer %d started\n", num);

    while (1){
        res = queueRemove(&val);
        if (res < 0){
            if (res == -2)
                fprintf(stderr, "Consumer[%d]: timedout\n", num);
            break;
        }

        fprintf(stdout, "Consumer[%d]: Removed %d\n", num, val);

        us = sleeptime();
        fprintf(stdout, "Producer[%d]: going to sleep for %d us\n", num, us);
        usleep(us);
    }

    return 0;
}


int main(int argc, char *argv[])
{
    size_t i;
    int pid;

    if (allocAndInit() != 0){
        return -1;
    }

    // create producers
    for (i = 0; i < PRODUCERS; i++){
        pid = fork();
        if (pid < 0){
            perror("Fork failed: ");
        }else if (pid == 0){
            return producer(i);
        }else{
            fprintf(stdout, "Created new process: %d\n", pid);
        }
    }

    // create consumers
    for (i = 0; i < CONSUMERS; i++){
        pid = fork();
        if (pid < 0){
            perror("Fork failed: ");
        }else if (pid == 0){
            return consumer(i);
        }else{
            fprintf(stdout, "Created new process: %d\n", pid);
        }
    }

    // join all subprocesses
    while (wait(NULL) != -1);

    unalloc();

    return 0;
}

