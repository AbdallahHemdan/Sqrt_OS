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

typedef short bool;
#define true 1
#define false 0

#define SHKEY 300

#define gcc "/usr/bin/gcc "

// ============================== //
// don't mess with this variables //

int *shmaddr; //
char msqProcessKey = 'M';

struct Process
{
    bool running, lastProcess; // lastProcess in the whole program
    int executaionTime, remainingTime, waitingTime, priority, id;
    char text[5]; // if "End": this is the last process in this second, else receive more;
};
typedef struct Process Process;

// =============================== //

void compileAndRun(char *fileName, char *arg1, char *arg2)
{
    char *compile;
    compile = (char *)malloc((15 + 2 * sizeof(fileName) * sizeof(char)));
    strcpy(compile, "gcc ");
    strcat(compile, fileName);
    strcat(compile, ".c -o ");
    strcat(compile, fileName);

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
    int send_val = msgsnd(msgqId, &process, sizeof(process.text), !IPC_NOWAIT);

    if (send_val == -1)
        perror("Error in sending the process");
}

Process receiveMessage(int msgqId)
{
    Process process;

    int rec_val = msgrcv(msgqId, &process, sizeof(process.text), 0, !IPC_NOWAIT);

    if (rec_val == -1)
        perror("Error in receive");
    else
        printf("Message received: %s\n", process.text);

    return process;
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
    return (q->rear == NULL);
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
    {
        (*head) = temp;
    }
    else if ((*head)->priority > p) // Insert New Node before head
    {
        temp->next = *head;
        (*head) = temp;
    }
    else
    {
        Node *start = (*head);

        while (start->next != NULL && start->next->priority < p)
        {
            start = start->next;
        }

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
