#include "headers.h"

void HPF();
void SRTN();
void RR();

struct curProcess
{
    Process process;
    int runningTime;
};

int msqProcessId, *shmId, *terminate;

int main(int argc, char *argv[])
{
    initClk();
    int algorithm = atoi(argv[0]);
    int quantum = atoi(argv[1]);

    msqProcessId = initMsgq(msqProcessKey);
    shmId = (int *)initShm(shmProcessKey, shmId);
    terminate = (int *)initShm(terminateKey, terminate);
    *terminate = false;
    switch (algorithm)
    {
    case 1:
        HPF();
        break;
    case 2:
        SRTN();
        break;
    case 3:
        RR();
        break;
    }

    // TODO: upon termination release the clock resources.
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

void HPF()
{
    printf("HPF started\n");
    // 1. declare all needed variables
    bool lastProcess = 0;
    int lastSecond = -1;
    Process running;
    Node *pq;

    // 2. initilization
    initializePQ(&pq);
    running.remainingTime = 0;
    printf("At time x process y started arr w total z remain y wait k\n");
    *shmId = -1;

    /* 3. start working with processes
     *  3.1 check every second if there is new processes (consuming)
     *  3.2 push the new processes if there is any
     *  3.3 contiune working
    */
    while (!(lastProcess && isEmptyPQ(&pq) && *shmId == -1))
    {
        while (lastSecond == getClk())
            ;

        lastSecond = getClk();
        Process receivedProcess;
        while (1) // get the new processes
        {
            bool newProcess = checkNewProcess(&receivedProcess);
            if (!newProcess)
                break;
            else
            {
                printf("push in queue\n");
                printf("clk %d\n", getClk());
                lastProcess = receivedProcess.lastProcess;
                push(&pq, receivedProcess, receivedProcess.priority);
            }
        }
        if (*shmId == 0) // the running process has finished
        {
            printf("At time %d process %d finished arr %d total %d remain 0 wait %d TA %d WTA %d\n", getClk(),
                   running.id, running.arrivalTime, running.executaionTime, (getClk() - running.arrivalTime) - running.executaionTime,
                   getClk() - running.arrivalTime, (getClk() - running.arrivalTime) / running.executaionTime);
            *shmId = -1;
        }

        if (!isEmptyPQ(&pq) && *shmId == -1)
        {
            printf("pop from queue\n");
            running = pop(&pq);
            *shmId = running.executaionTime;
            int processID = fork();
            if (processID == 0)
            {
                printf("At time %d process %d started arr %d total %d remain %d wait %d\n",
                       getClk(), running.id, running.arrivalTime, running.executaionTime,
                       running.executaionTime, getClk() - running.arrivalTime);

                compileAndRun("process", NULL, NULL);
            }
        }
    }
    *terminate = true;
    printf("Nice work\nMade with love ❤\n");
}

void SRTN()
{
    // Same as HPF
}

void RR()
{
    // Same as HPF with queue instead of priority queue
}