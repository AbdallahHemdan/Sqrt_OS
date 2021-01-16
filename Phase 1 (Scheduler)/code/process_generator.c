#include "headers.h"

void clearResources(int);
int *terminate, msqProcessId;
int shmTerminateId;
int main(int argc, char *argv[])
{
    signal(SIGINT, clearResources);
    // TODO Initialization
    // 1. Read the input files.
    char *line = NULL;
    size_t len = 0;
    int numberOfProcesses = 0;
    FILE *input;
    input = fopen("processes.txt", "r");
    while (getline(&line, &len, input) != -1)
        if (line[0] != '#')
            numberOfProcesses++;
    fclose(input);
    input = fopen("processes.txt", "r");
    Process *processes = (Process *)malloc(numberOfProcesses * sizeof(Process));
    int index = 0;
    while (getline(&line, &len, input) != -1)
    {
        fscanf(input, "%d", &(processes[index].id));
        fscanf(input, "%d", &(processes[index].arrivalTime));
        fscanf(input, "%d", &(processes[index].executionTime));
        fscanf(input, "%d", &(processes[index].priority));
        strcpy(processes[index].text, "not End");
        processes[index].lastProcess = false;
        processes[index].remainingTime = processes[index].executionTime;
        processes[index].arrivalTime++;
        if (index == numberOfProcesses - 1)
            processes[index].lastProcess = true;
        index++;
    }

    // 2. Ask the user for the chosen scheduling algorithm and its parameters, if there are any.
    char algorithmNumber[2];

    char args[2][2];
    //char *args2[1] = NULL;
    printf("please enter:\n1 to choose HPF\n2 to choose SRTN\n3 to choose RR\n");
    scanf("%s", args[0]);
    if (!strcmp(args[0], "3"))
    {
        printf("You choose RR algorithm please enter the quantum\n");
        scanf("%s", args[1]);
    }
    else
        strcpy(args[1], "0");
    /* 3. Initiate and create the scheduler and clock processes.
     *   Note: look at compileAndRun funciton in the header. (just send the file name without .c)
    */
    msqProcessId = initMsgq(msqProcessKey);
    terminate = (int *)initShm(terminateKey, &shmTerminateId);
    *terminate = false;
    int clkID = fork();
    if (clkID == 0)
        compileAndRun("clk", NULL, NULL);

    int schedID = fork();
    if (schedID == 0)
        compileAndRun("scheduler", args[0], args[1]);

    // 4. Use this function after creating the clock process to initialize clock

    // // TODO Generation Main Loop
    // // 5. Create a data structure for processes and provide it with its parameters.
    // // 6. Send the information to the scheduler at the appropriate time.
    index = 0;
    int last = -1;
    initClk();
    while (!(*terminate))
    {
        int x = getClk();
        //printf("current time is %d\n", x);
        while (last != x)
        {
            last = getClk();
            while (processes[index].arrivalTime == x)
            {
                //printf("process generator time %d index %d\n", x, index);
                sendMessage(processes[index], msqProcessId);
                index++;
            }
            Process end;
            strcpy(end.text, "End");
            sendMessage(end, msqProcessId);
        }
    }
    // // 7. Clear clock resources
    msgctl(msqProcessId, IPC_RMID, (struct msqid_ds *)0);
    shmctl(shmTerminateId, IPC_RMID, NULL);
    destroyClk(true);
}

void clearResources(int signum)
{
    msgctl(msqProcessId, IPC_RMID, (struct msqid_ds *)0);
    shmctl(shmTerminateId, IPC_RMID, NULL);
    signal(SIGINT, SIG_DFL);
}
