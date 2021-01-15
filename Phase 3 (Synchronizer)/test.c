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

union Semun
{
    int val;               /* value for SETVAL */
    struct semid_ds *buf;  /* buffer for IPC_STAT & IPC_SET */
    ushort *array;         /* array for GETALL & SETALL */
    struct seminfo *__buf; /* buffer for IPC_INFO */
    void *__pad;
};

void down(int sem, int sum_num)
{
    struct sembuf p_op;

    p_op.sem_num = sum_num;
    p_op.sem_op = -1;
    p_op.sem_flg = !IPC_NOWAIT;

    if (semop(sem, &p_op, 1) == -1)
    {
        perror("Error in down()");
        exit(-1);
    }
}

void up(int sem, int sem_num)
{
    struct sembuf v_op;

    v_op.sem_num = sem_num;
    v_op.sem_op = 1;
    v_op.sem_flg = !IPC_NOWAIT;

    if (semop(sem, &v_op, 1) == -1)
    {
        perror("Error in up()");
        exit(-1);
    }
}

union Semun semun;

int initSem(char keyNum, int size, int initial_value)
{
    int semKey = ftok("keyfile", keyNum);
    int semId = semget(semKey, size, 0666);

    if (semId == -1)
    {
        perror("Error in create sem");
        exit(-1);
    }

    semun.val = initial_value; /* initial value of the semaphore, Binary semaphore */
    for (int i = 0; i < size; i++)
    {
        if (semctl(semId, i, SETVAL, semun) == -1)
        {
            perror("Error in semctl");
            exit(-1);
        }
    }

    return semId;
}

int main()
{
    int semId = initSem('t', 1, 0);

    up(semId, 0);
    up(semId, 0);
    up(semId, 0);
    up(semId, 0);
    up(semId, 0);
    printf("ds");
    semctl(semId, 0, IPC_RMID);
    return 0;
}
