#include "headers.h"
#include <math.h>

void HPF();
void SRTN();
void RR();

int shmProcessRemainingTimeId, shmTerminateId, msqProcessId, *shmId, *terminate;
double sumWTA = 0, sumWait = 0, cpu, std = 0, sumExecution = 0, finishTime;
int idle = 0, count = 0, remainingTimeSem;
bool lastProcess = 0;
void clearResources(int);
lNode *processesWTA;
FILE *schedularFile, *statisticsFile;

int main(int argc, char *argv[])
{
    schedularFile = fopen("schedular.log", "w");
    statisticsFile = fopen("schedular.perf", "w");
    signal(SIGINT, clearResources);
    initClk();
    int algorithm = atoi(argv[0]);
    int quantum = atoi(argv[1]);

    remainingTimeSem = initSem('R');
    msqProcessId = initMsgq(msqProcessKey);
    shmId = (int *)initShm(shmProcessKey, &shmProcessRemainingTimeId);
    terminate = (int *)initShm(terminateKey, &shmTerminateId);
    *terminate = false;
    fprintf(schedularFile, "#At time x process y state arr w total z remain y wait k\n");
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

    fclose(schedularFile);
    fclose(statisticsFile);
    printf("Nice work Made with love ❤\n");
    shmctl(shmProcessRemainingTimeId, IPC_RMID, NULL);
    destroyClk(true);
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

int startProcess(Process running)
{
    running.pid = fork();

    if (running.pid == 0) // start a new process
        compileAndRun("process", NULL, NULL);

    fprintf(schedularFile, "At time %d process %d started arr %d total %d remain %d wait %d\n",
            getClk(), running.id, running.arrivalTime, running.executionTime,
            running.executionTime, getClk() - running.arrivalTime);
    return running.pid;
}

void continueProcess(Process running)
{
    kill(running.pid, SIGCONT);
    fprintf(schedularFile, "At time %d process %d resumed arr %d total %d remain %d wait %d\n",
            getClk(), running.id, running.arrivalTime, running.executionTime,
            running.remainingTime, getClk() - running.arrivalTime - (running.executionTime - running.remainingTime));
}

void stopProcess(Process running)
{
    *shmId = -1;

    // up(remainingTimeSem);
    kill(running.pid, SIGSTOP);
    // down(remainingTimeSem);

    fprintf(schedularFile, "At time %d process %d stopped arr %d total %d remain %d wait %d\n",
            getClk(), running.id, running.arrivalTime, running.executionTime,
            running.remainingTime, getClk() - running.arrivalTime - (running.executionTime - running.remainingTime));
}

void finishProcess(Process running)
{
    finishTime = getClk();
    double WTA = (getClk() - running.arrivalTime) * 1.0 / running.executionTime;
    int wait = (getClk() - running.arrivalTime) - running.executionTime;
    fprintf(schedularFile, "At time %d process %d finished arr %d total %d remain 0 wait %d TA %d WTA %.2f\n", getClk(),
            running.id, running.arrivalTime, running.executionTime,
            wait, getClk() - running.arrivalTime, WTA);
    insert(&processesWTA, WTA);
    sumWTA += WTA;
    sumWait += wait;
    sumExecution += running.executionTime;
    count++;
    *shmId = -1;
}

/*
 * Push the new received processes if there is any 
*/
void pushReadyQueue(Node **pq, queue **q, bool type, int prioritize)
{
    Process receivedProcess;
    while (1) // get the new processes
    {
        bool newProcess = checkNewProcess(&receivedProcess);
        if (!newProcess)
            break;
        else
        {
            lastProcess = receivedProcess.lastProcess;
            if (!type)
                push(pq, receivedProcess, prioritize == 1 ? receivedProcess.priority : receivedProcess.remainingTime);
            else
                enqueue(*q, receivedProcess);
        }
    }
}

void nextSecondWaiting(int *lastSecond)
{
    while (*lastSecond == getClk())
        ;
    *lastSecond = getClk();
}

void HPF()
{
    printf("HPF started\n");

    // 1. declare all needed variables

    int lastSecond = -1;
    Process running;
    Node *pq;

    // 2. initialization
    initializePQ(&pq);
    running.remainingTime = 0;

    *shmId = -1;
    /* 3. start working with processes
     *  3.1 check every second if there is new processes (consuming)
     *  3.2 push the new processes if there is any
     *  3.3 continue working
    */
    while (!(lastProcess && isEmptyPQ(&pq) && *shmId == -1))
    {
        pushReadyQueue(&pq, NULL, 0, 1);

        if (*shmId == 0) // the running process has finished
            finishProcess(running);

        if (!isEmptyPQ(&pq) && *shmId == -1)
        {
            running = pop(&pq);
            *shmId = running.executionTime + 1;
            running.pid = startProcess(running);
        }

        nextSecondWaiting(&lastSecond);
    }
}

void SRTN()
{
    printf("SRTN started\n");

    // 1. declare all needed variables
    Node *pq;
    Process running;
    int lastSecond = -1;

    // 2. initialization
    *shmId = -1;
    initializePQ(&pq);
    running.remainingTime = 0;

    /* 3. start working with processes
     *  3.1 check every second if there is new processes (consuming)
     *  3.2 push the new processes if there is any
     *  3.3 continue working
    */
    while (!(lastProcess && isEmptyPQ(&pq) && *shmId == -1))
    {
        pushReadyQueue(&pq, NULL, 0, 0);

        // down(remainingTimeSem);
        if (*shmId == 0) // the running process has finished
            finishProcess(running);

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

                if (top.remainingTime < top.executionTime) //Cont
                    continueProcess(running);
                else
                    running.pid = startProcess(running);
            }
            else // push it again into the pq
            {
                push(&pq, top, top.remainingTime);
            }
        }

        nextSecondWaiting(&lastSecond);
        // up(remainingTimeSem);
    }
}

void RR(int quantum)
{
    printf("RR started\n");

    // 1. declare all needed variables
    Node *pq;
    Process running;
    int lastSecond = -1, quantumCnt = 0;

    // 2. initialization
    *shmId = -1;
    queue *q;
    q = malloc(sizeof(queue));
    initialize(q);
    running.remainingTime = 0;

    /* 3. start working with processes
     *  3.1 check every second if there is new processes (consuming)
     *  3.2 push the new processes if there is any
     *  3.3 continue working
    */
    while (!(lastProcess && isEmpty(q) && *shmId == -1))
    {
        quantumCnt--;
        pushReadyQueue(NULL, &q, 1, 0);

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

            if (top.remainingTime < top.executionTime) //cont
                continueProcess(running);
            else
                running.pid = startProcess(running);
        }

        nextSecondWaiting(&lastSecond);
    }
}

void clearResources(int signum)
{
    shmctl(shmProcessRemainingTimeId, IPC_RMID, NULL);
    semctl(remainingTimeSem, 0, IPC_RMID);
    signal(SIGINT, SIG_DFL);
}
