#include <stdio.h> //if you don't use scanf/printf change this include
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <assert.h>

typedef short bool;
#define true 1
#define false 0

#define max(a, b) (((a) > (b)) ? (a) : (b))
#define min(a, b) (((a) < (b)) ? (a) : (b))

#define SHKEY 300

#define gcc "/usr/bin/gcc "

// ============================== //
// don't mess with this variables //
int *shmaddr;
char msqProcessKey = 'M';
char terminateKey = 'K';
char shmProcessKey = 'S';

struct Process
{
    char text[5];              // if "End": this is the last process in this second, else receive more;
    bool running, lastProcess; // lastProcess in the whole program
    int executaionTime, remainingTime, arrivalTime, waitingTime, priority, id, pid, memOffset, processSize;
};
typedef struct Process Process;

struct Message
{
    Process process;
    int mtype;
};
typedef struct Message Message;

// =============================== //

void compileAndRun(char *fileName, char *arg1, char *arg2)
{
    char *compile;
    compile = (char *)malloc((20 + 2 * sizeof(fileName) * sizeof(char)));
    strcpy(compile, "gcc ");
    strcat(compile, fileName);
    strcat(compile, ".c -o ");
    strcat(compile, fileName);
    strcat(compile, " -lm");

    system(compile);
    execl(fileName, arg1, arg2, NULL);
}

int getClk()
{
    return *shmaddr;
}

/*
 * All processes call this function at the beginning to establish communication between them and the clock module.
 * Again, remember that the clock is only emulation!
*/
void initClk()
{
    int shmid = shmget(SHKEY, 4, 0444);
    while ((int)shmid == -1)
    {
        //Make sure that the clock exists
        printf("Wait! The clock not initialized yet!\n");
        sleep(1);
        shmid = shmget(SHKEY, 4, 0444);
    }
    shmaddr = (int *)shmat(shmid, (void *)0, 0);
}

int initMsgq(char key)
{
    key_t msgqKey = ftok("keyfile", key);
    int msgqId = msgget(msgqKey, 0666 | IPC_CREAT);

    if (!~msgqId)
    {
        perror("Error in creating of message queue");
        exit(-1);
    }
    return msgqId;
}

void sendMessage(Process process, int msgqId)
{
    Message message;
    message.process = process;
    int send_val = msgsnd(msgqId, &message, sizeof(message.process), !IPC_NOWAIT);
    if (send_val == -1)
        perror("Error in sending the process");
}

Process receiveMessage(int msgqId)
{
    Process process;
    Message message;
    int rec_val = msgrcv(msgqId, &message, sizeof(message.process), 0, !IPC_NOWAIT);
    process = message.process;

    if (rec_val == -1)
        perror("Error in receive");
    // else if (strcmp(process.text, "End"))
    //     printf("Message received: %s clk %d\n", process.text, getClk());

    return process;
}

void *initShm(char key, int *id)
{
    key_t shmKey = ftok("keyfile", key);
    int shmId = shmget(shmKey, 4096, 0666 | IPC_CREAT);
    *id = shmId;

    if (!~shmId)
    {
        perror("Error in creating of message queue");
        exit(-1);
    }

    void *addr = shmat(shmId, (void *)0, 0);
    return addr;
}

/*
 * All process call this function at the end to release the communication
 * resources between them and the clock module.
 * Again, Remember that the clock is only emulation!
 * Input: terminateAll: a flag to indicate whether that this is the end of simulation.
 *                      It terminates the whole system and releases resources.
*/

void destroyClk(bool terminateAll)
{
    shmdt(shmaddr);
    if (terminateAll)
    {
        killpg(getpgrp(), SIGINT);
    }
}

/*
 * Linked-list implementation
 * we used linked-list to benefit from the dynamic allocation
 * 
 * How to initialize it:
 *      lNode *ls;
 *      insert(&ls, val);
 *      extract(&ls);
 * 
*/

typedef struct lNode lNode;

struct lNode
{
    lNode *next;
    double value;
};

void insert(lNode **head, double v)
{
    lNode *temp = (lNode *)malloc(sizeof(lNode));
    temp->value = v;
    temp->next = *head;
    *head = temp;
}

double extract(lNode **head)
{
    lNode *temp = *head;
    double val = (*head)->value;
    (*head) = (*head)->next;
    free(temp);
    return val;
}
/*
 * Queue implementation using linked-list 
 * we used linked-list to make it easy to add a process and remove it
 * 
 * How to initilize it:
 *      queue *q;
 *      q = malloc(sizeof(queue));
 *      initialize(q);
 *      enqueue(q, val);
 *      dequeue(q);
 * 
*/

typedef struct node node;
typedef struct queue queue;

struct node
{
    Process data;
    struct node *next;
};

struct queue
{
    int count;
    node *front;
    node *rear;
};

void initialize(queue *q)
{
    q->count = 0;
    q->front = NULL;
    q->rear = NULL;
}

bool isEmpty(queue *q)
{
    return !(q->count);
}

void enqueue(queue *q, Process value)
{
    node *tmp;
    tmp = malloc(sizeof(node));
    tmp->data = value;
    tmp->next = NULL;

    if (!isEmpty(q))
    {
        q->rear->next = tmp;
        q->rear = tmp;
    }
    else
    {
        q->front = q->rear = tmp;
    }
    q->count++;
}

Process dequeue(queue *q)
{
    node *tmp;
    Process n = q->front->data;
    tmp = q->front;
    q->front = q->front->next;
    q->count--;
    free(tmp);
    return (n);
}

/*
 * Priority queue using linked-list 
 * 
 * Node* pq;
 * initilize(&pq);
 * push(&pq, 7, 0); 
 * pop(&pq); 
*/

typedef struct Node
{
    Process data;
    int priority; // lower value -> higher priority

    struct Node *next;
} Node;

Node *newNode(Process d, int p)
{
    Node *temp = (Node *)malloc(sizeof(Node));
    temp->data = d;
    temp->priority = p;
    temp->next = NULL;

    return temp;
}

void initializePQ(Node **head)
{
    (*head) = NULL;
}

void push(Node **head, Process d, int p)
{
    Node *temp = newNode(d, p);
    if ((*head) == NULL) // Insert in empty queue
        (*head) = temp;
    else if ((*head)->priority > p) // Insert New Node before head
    {
        temp->next = *head;
        (*head) = temp;
    }
    else
    {
        Node *start = (*head);

        while (start->next != NULL && start->next->priority < p)
            start = start->next;
        temp->next = start->next;
        start->next = temp;
    }
}

Process pop(Node **head)
{
    Node *temp = *head;
    Process ret = (*head)->data;
    (*head) = (*head)->next;
    free(temp);
    return ret;
}

bool isEmptyPQ(Node **head)
{
    return (*head) == NULL;
}

/* ------------ Start of memory management using buddy algorithm -------------*/

struct buddy
{
    size_t size;
    size_t longest[1];
};

static inline int left_child(int index)
{
    return ((index << 1) + 1);
}

static inline int right_child(int index)
{
    return ((index << 1) + 2);
}

static inline int parent(int index)
{
    return (((index + 1) >> 1) - 1);
}

static inline bool is_power_of_2(int index)
{
    return !(index & (index - 1));
}

static void *b_malloc(size_t size)
{
    void *tmp = NULL;

    tmp = malloc(size);
    if (tmp == NULL)
    {
        fprintf(stderr, "my_malloc: not enough memory, quit\n");
        exit(EXIT_FAILURE);
    }

    return tmp;
}

static void b_free(void *addr)
{
    free(addr);
}

static inline unsigned next_power_of_2(unsigned size)
{
    size -= 1;
    size |= (size >> 1);
    size |= (size >> 2);
    size |= (size >> 4);
    size |= (size >> 8);
    size |= (size >> 16);
    return size + 1;
}

struct buddy *buddy_new(unsigned num_of_fragments)
{
    struct buddy *self = NULL;
    size_t node_size;

    int i;

    if (num_of_fragments < 1 || !is_power_of_2(num_of_fragments))
    {
        return NULL;
    }

    self = (struct buddy *)b_malloc(sizeof(struct buddy) + 2 * num_of_fragments * sizeof(size_t));
    self->size = num_of_fragments;
    node_size = num_of_fragments * 2;

    int iter_end = num_of_fragments * 2 - 1;
    for (i = 0; i < iter_end; i++)
    {
        if (is_power_of_2(i + 1))
        {
            node_size >>= 1;
        }
        self->longest[i] = node_size;
    }

    return self;
}

void buddy_destory(struct buddy *self)
{
    b_free(self);
}

unsigned choose_better_child(struct buddy *self, unsigned index, size_t size)
{
    struct compound
    {
        size_t size;
        unsigned index;
    } children[2];
    children[0].index = left_child(index);
    children[0].size = self->longest[children[0].index];
    children[1].index = right_child(index);
    children[1].size = self->longest[children[1].index];

    int min_idx = (children[0].size <= children[1].size) ? 0 : 1;

    if (size > children[min_idx].size)
    {
        min_idx = 1 - min_idx;
    }

    return children[min_idx].index;
}

int buddy_alloc(struct buddy *self, size_t size)
{
    if (self == NULL || self->size < size)
    {
        return -1;
    }
    size = next_power_of_2(size);

    unsigned index = 0;
    if (self->longest[index] < size)
    {
        return -1;
    }

    unsigned node_size = 0;
    for (node_size = self->size; node_size != size; node_size >>= 1)
    {
        index = choose_better_child(self, index, size);
    }

    self->longest[index] = 0;
    int offset = (index + 1) * node_size - self->size;

    while (index)
    {
        index = parent(index);
        self->longest[index] =
            max(self->longest[left_child(index)],
                self->longest[right_child(index)]);
    }

    return offset;
}

void buddy_free(struct buddy *self, int offset)
{
    if (self == NULL || offset < 0 || offset > self->size)
    {
        return;
    }

    size_t node_size;
    unsigned index;

    node_size = 1;
    index = offset + self->size - 1;

    for (; self->longest[index] != 0; index = parent(index))
    {
        node_size <<= 1;

        if (index == 0)
        {
            break;
        }
    }

    self->longest[index] = node_size;

    while (index)
    {
        index = parent(index);
        node_size <<= 1;

        size_t left_longest = self->longest[left_child(index)];
        size_t right_longest = self->longest[right_child(index)];

        if (left_longest + right_longest == node_size)
        {
            self->longest[index] = node_size;
        }
        else
        {
            self->longest[index] = max(left_longest, right_longest);
        }
    }
}

int buddy_size(struct buddy *self, int offset)
{
    unsigned node_size = 1;
    unsigned index = offset + self->size - 1;

    for (; self->longest[index]; index = parent(index))
    {
        node_size >>= 1;
    }

    return node_size;
}

void buddy_dump(struct buddy *self)
{
    int len = self->size << 1;
    int max_col = self->size << 1;
    int level = 0;
    int i, j;

    char cs[] = {'/', '\\'};
    int idx = 0;
    char c;

    for (i = 0, max_col = len, level = 0; i < len - 1; i++)
    {
        if (is_power_of_2(i + 1))
        {
            max_col >>= 1;
            level++;
            idx = 0;
            printf("\n%d(%.2d): ", level, max_col);
        }

        printf("%*ld", max_col, self->longest[i]);
    }

    for (i = 0, max_col = len, level = 0; i < len - 1; i++)
    {
        if (is_power_of_2(i + 1))
        {
            max_col >>= 1;
            level++;
            idx = 0;
            printf("\n%d(%.2d): ", level, max_col);
        }

        if (self->longest[i] > 0)
        {
            c = '-';
        }
        else
        {
            c = cs[idx];
            idx ^= 0x1;
        }

        for (j = 0; j < max_col; j++)
        {
            printf("%c", c);
        }
    }
    printf("\n");
}
/* ------------ End of memory management using buddy algorithm -------------*/
