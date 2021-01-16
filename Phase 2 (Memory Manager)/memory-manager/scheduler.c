#include "headers.h"
#include <math.h>
void HPF();
void SRTN();
void RR();

int shmProcessRemainingTimeId, shmTerminateId, msqProcessId, *shmId, *terminate;
double sumWTA = 0, sumWait = 0, cpu, std = 0, sumExecution = 0, finishTime;
int idle = 0, count = 0;
bool lastProcess = 0;
void clearResources(int);
lNode *processesWTA;
FILE *schedularFile, *statisticsFile, *memoryFile;
struct buddy *buddy = NULL;

queue *q;
Node *pq;
bool queueType; // 1. 0 = priority queue, 1 = queue
queue *waitingList;

int main(int argc, char *argv[])
{
    schedularFile = fopen("schedular.log", "w");
    memoryFile = fopen("memory.log", "w");
    statisticsFile = fopen("schedular.perf", "w");
    signal(SIGINT, clearResources);
    initClk();
    int algorithm = atoi(argv[0]);
    int quantum = atoi(argv[1]);

    msqProcessId = initMsgq(msqProcessKey);
    shmId = (int *)initShm(shmProcessKey, &shmProcessRemainingTimeId);
    terminate = (int *)initShm(terminateKey, &shmTerminateId);
    *terminate = false;

    waitingList = malloc(sizeof(queue));

    fprintf(memoryFile, "#At time x allocated y bytes for process z from i to j\n");
    fprintf(schedularFile, "#At time x process y started arr w total z remain y wait k\n");

    /* Create new instance from buddy algorithm */
    buddy = buddy_new(64);

    switch (algorithm)
    {
    case 1:
        HPF();
        break;
    case 2:
        SRTN();
        break;
    case 3:
        RR(quantum);
        break;
    }
    *terminate = true;

    double AVGWTA = sumWTA / count;
    cpu = sumExecution * 100.0 / (finishTime - 1);

    while (processesWTA != NULL)
    {
        double WTAC = extract(&processesWTA);
        std += (WTAC - AVGWTA) * (WTAC - AVGWTA);
    }
    std = sqrt(std);
    fprintf(statisticsFile, "CPU utilization = %.0f%% \nAvg WTA = %.2f \nAvg Waiting = %.2f \nStd WTA = %.2f\n", cpu, AVGWTA, sumWait / count, std);

    fclose(memoryFile);
    fclose(schedularFile);
    fclose(statisticsFile);
    printf("Nice work Made with love ❤\n");
    shmctl(shmProcessRemainingTimeId, IPC_RMID, NULL);
    destroyClk(true);
}

void pushWaitingToReady(Node **pq, queue **q, bool type, Process current)
{
    if (!type)
        push(pq, current, current.priority);
    else
        enqueue(*q, current);
}

void removeFromReady(Node **pq, queue **q, bool type)
{
    if (!type && !isEmptyPQ(pq))
    {
        printf("POP from PQ\n");
        pop(pq);
    }
    else if (type && !isEmpty(*q))
    {
        printf("POP from Q\n");
        dequeue(*q);
    }
}

bool checkNewProcess(Process *newProcess)
{
    /* 1. check the message queue for new processes
     *  1.1 if True: update newProcess parameter and return true
     *  1.2 else: return false;
     */
    *newProcess = receiveMessage(msqProcessId);
    return strcmp((*newProcess).text, "End");
}

void startProcess(Process *running)
{
    (*running).pid = fork();

    if ((*running).pid == 0) // start a new process
    {
        compileAndRun("process", NULL, NULL);
    }

    fprintf(schedularFile, "At time %d process %d started arr %d total %d remain %d wait %d\n",
            getClk(), (*running).id, (*running).arrivalTime, (*running).executaionTime,
            (*running).executaionTime, getClk() - (*running).arrivalTime);

    /* Start of allocating memory for the process */
    (*running).memOffset = buddy_alloc(buddy, (*running).processSize);

    int endOffset = ceil(log2((*running).processSize) * 1.0);
    endOffset = pow(2, endOffset) - 1;

    if ((*running).memOffset != -1)
    {
        printf("\n\nAllocating: %d,with size = %d\n\n", (*running).id, (*running).processSize);
        fprintf(memoryFile, "At time %d allocated %d bytes for process %d from %d to %d\n",
                getClk(), (*running).processSize, (*running).id, (*running).memOffset, (*running).memOffset + endOffset);
    }
    else
    {
        printf("Add to waiting LIST !!!!!!!!\n");
        enqueue(waitingList, *running);
        // pop from ready queue;

        if (!queueType)
        {
            removeFromReady(&pq, NULL, queueType);
        }
        else
        {
            removeFromReady(NULL, &q, queueType);
        }

        fprintf(memoryFile, "At time %d, there is no memory space for allocation\n", getClk());
    }
    /* Start of allocating memory for the process */
}

void continueProcess(Process running)
{
    kill(running.pid, SIGCONT);
    fprintf(schedularFile, "At time %d process %d resumed arr %d total %d remain %d wait %d\n",
            getClk(), running.id, running.arrivalTime, running.executaionTime,
            running.remainingTime, getClk() - running.arrivalTime - (running.executaionTime - running.remainingTime));
}

void stopProcess(Process running)
{
    *shmId = -1;

    kill(running.pid, SIGSTOP);
    fprintf(schedularFile, "At time %d process %d stopped arr %d total %d remain %d wait %d\n",
            getClk(), running.id, running.arrivalTime, running.executaionTime,
            running.remainingTime, getClk() - running.arrivalTime - (running.executaionTime - running.remainingTime));
}

void finishProcess(Process running)
{
    finishTime = getClk();
    double WTA = (getClk() - running.arrivalTime) * 1.0 / running.executaionTime;
    int wait = (getClk() - running.arrivalTime) - running.executaionTime;
    fprintf(schedularFile, "At time %d process %d finished arr %d total %d remain 0 wait %d TA %d WTA %.2f\n", getClk(),
            running.id, running.arrivalTime, running.executaionTime,
            wait, getClk() - running.arrivalTime, WTA);

    /*  Start Free memory for the current process */;
    buddy_dump(buddy);

    buddy_free(buddy, running.memOffset);
    buddy_dump(buddy);

    int endOffset = ceil(log2(running.processSize) * 1.0);
    endOffset = pow(2, endOffset) - 1;

    printf("\n\nFreeing: %d,with size = %d\n\n", running.id, running.processSize);
    fprintf(memoryFile, "At time %d freed %d bytes from process %d from %d to %d\n",
            getClk(), running.processSize, running.id, running.memOffset, running.memOffset + endOffset);
    /*  End Free memory for the current process */

    /* Start Processing of waitling list and add it to current queue */
    while (!isEmpty(waitingList))
    {
        Process current = dequeue(waitingList); 
        if (!queueType)
        {
            pushWaitingToReady(&pq, NULL, queueType, current);
        }
        else
        {
            pushWaitingToReady(NULL, &q, queueType, current);
        }
    }

    /* End Processing of waitling list and add it to current queue */
    insert(&processesWTA, WTA);
    sumWTA += WTA;
    sumWait += wait;
    sumExecution += running.executaionTime;
    count++;
    *shmId = -1;
}

void pushReadyQueue(Node **pq, queue **q, bool type)
{
    Process receivedProcess;

    // while there are new proecesses in the current second, push them to the ready queue
    while (1)
    {
        bool newProcess = checkNewProcess(&receivedProcess);
        if (!newProcess)
            break;
        else
        {
            lastProcess = receivedProcess.lastProcess;

            //    Make it a function    //

            // check if there is a memory for receivedProcess
            // TRUE: PASS (Push and Allocate)
            // FALSE: Add to the waiting list and return

            if (!type)
                push(pq, receivedProcess, receivedProcess.priority);
            else
                enqueue(*q, receivedProcess);

            ////////////////////////////////
        }
    }
}

void HPF()
{
    printf("HPF started\n");
    queueType = 0;
    // 1. declare all needed variables

    int lastSecond = -1;
    Process running;

    // 2. initilization
    initializePQ(&pq);
    running.remainingTime = 0;

    *shmId = -1;

    /* 3. start working with processes
     *  3.1 check every second if there is new processes (consuming)
     *  3.2 push the new processes if there is any
     *  3.3 contiune working
    */
    while (!(lastProcess && isEmptyPQ(&pq) && *shmId == -1))
    {
        pushReadyQueue(&pq, NULL, 0);

        if (*shmId == 0) // the running process has finished
            finishProcess(running);

        if (!isEmptyPQ(&pq) && *shmId == -1)
        {
            running = pop(&pq);
            *shmId = running.executaionTime + 1;
            startProcess(&running);
            printf("After calling: %d\n", running.memOffset);
        }
    }
}

void SRTN()
{
    printf("SRTN started\n");

    // 1. declare all needed variables
    Node *pq;
    Process running;
    int lastSecond = -1;

    // 2. initilization
    *shmId = -1;
    initializePQ(&pq);
    running.remainingTime = 0;

    /* 3. start working with processes
     *  3.1 check every second if there is new processes (consuming)
     *  3.2 push the new processes if there is any
     *  3.3 contiune working
    */
    while (!(lastProcess && isEmptyPQ(&pq) && *shmId == -1))
    {
        pushReadyQueue(&pq, NULL, 0);

        if (*shmId == 0)
        { // the running process has finished
            printf("SRTN finished: %d\n", running.processSize);
            finishProcess(running);
        }

        if (!isEmptyPQ(&pq))
        {
            Process top = pop(&pq);

            if (top.remainingTime < *shmId)
            {
                running.remainingTime = *shmId;
                push(&pq, running, running.remainingTime);
                stopProcess(running);
            }

            if (*shmId == -1)
            {
                *shmId = top.remainingTime + 1;
                running = top;

                if (top.remainingTime < top.executaionTime) //Cont
                    continueProcess(running);
                else
                    startProcess(&running);
            }
            else // push it again into the pq
            {
                push(&pq, top, top.remainingTime);
            }
        }

        while (lastSecond == getClk())
            ;

        lastSecond = getClk();
    }
}

void RR(int quantum)
{
    printf("RR started\n");
    queueType = 1;

    // 1. declare all needed variables
    Process running;
    int lastSecond = -1, quantumCnt = 0;

    // 2. initilization
    *shmId = -1;
    q = malloc(sizeof(queue));
    initialize(q);
    running.remainingTime = 0;

    /* 3. start working with processes
     *  3.1 check every second if there is new processes (consuming)
     *  3.2 push the new processes if there is any
     *  3.3 contiune working
    */
    while (!(lastProcess && isEmpty(q) && *shmId == -1))
    {
        quantumCnt--;
        pushReadyQueue(NULL, &q, 1);

        if (*shmId == 0) // the running process has finished
        {
            finishProcess(running);
            quantumCnt = quantum;
        }

        if (!isEmpty(q) && (*shmId <= 0 || quantumCnt == 0))
        {
            quantumCnt = quantum;

            // stop current running process
            if (*shmId != -1)
            {
                running.remainingTime = *shmId;
                enqueue(q, running);
                stopProcess(running);
            }

            Process top = dequeue(q);

            *shmId = top.remainingTime + 1;
            running = top;

            if (top.remainingTime < top.executaionTime) //cont
                continueProcess(running);
            else
                startProcess(&running);
        }

        while (lastSecond == getClk())
            ;
        lastSecond = getClk();
    }
}

void clearResources(int signum)
{
    shmctl(shmProcessRemainingTimeId, IPC_RMID, NULL);
    signal(SIGINT, SIG_DFL);
}
