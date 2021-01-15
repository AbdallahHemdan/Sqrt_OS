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

// struct shmid_ds buff_status;
// int createShmem(int key)
// {
//     int shmid = shmget(key, sizeof(struct Buffer), IPC_CREAT | 0666);

//     if (shmid == -1)
//     {
//         perror("Error in creating the shared memory @ Producer:(\n");
//         exit(-1);
//     }

//     buf = shmat(shmid, (void *)0, 0);
//     if (buf == (struct Buffer*)-1)
//     {
//         perror("Error in attach @ Producer :(\n");
//         exit(-1);
//     }
//     /*critecal section*/
//     down(mutex_sem_id);
//     int status = shmctl(shmid, IPC_STAT, &buff_status);
//     while (status == -1)
//     {
//         printf("Failed to ge status, try again\n");
//         status = shmctl(shmid, IPC_STAT, &buff_status);
//     }
//     /*if this process is the first to attach then make it initialize the memory*/
//     if(buff_status.shm_nattch == 1)
//     {
//         printf("Initialize the memory\n");
//         buf->next_add=0;
//         buf->next_rem=0;
//     }
//     up(mutex_sem_id);
//     /*critecal section*/
//     return shmid;
// }

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

void *initShm(char key, int *id)
{
    key_t shmKey = ftok("keyfile", key);
    int shmId = shmget(shmKey, sizeof(int) * 10, IPC_CREAT | 0666);

    if (!~shmId)
    {
        printf("doesn't exist shared memory\n");
    }
    // else
    // {
    //     shmId = shmget(shmKey, 4096, 0666 | IPC_CREAT);
    // }
    *id = shmId;

    void *addr = shmat(shmId, (void *)0, 0);
    return addr;
}

int main()
{
    int id;
    int *a = initShm('a', &id);
    a[2] = 5;
    int *a2 = initShm('a', &id);
    printf("%d %d\n", a[2], a2[2]);
    return 0;
}
