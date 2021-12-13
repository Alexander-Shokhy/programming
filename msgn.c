//Программа создаёт n детей, каждый печатает номер, в каком порядке он порождён.
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>

/* Redefines the message structure */
typedef struct mymsgbuf
{
    long mtype;
    int num;
} mess_t;

int main(int argc, char **argv)
{
    int qid;//IPC дескриптор для очереди сообщений
    //key_t msgkey;//ключ
    pid_t pid;
    mess_t buf;
    int length;
    int i, n;
    length = sizeof(buf.num);

    if (argc != 2)
    {
        fprintf (stderr, "Должно быть 2 аргумента: путь к фалу и количество потомков\n");
        return(-1);
    }


    //msgkey = ftok(".",'m');
    //qid = msgget(msgkey, IPC_CREAT | 0660);
    qid = msgget(IPC_PRIVATE, IPC_CREAT | 0660);

        n = atoi(argv[1]);



        //создали n детей
        for (i = 0; i < n; i++)
        {
                pid = fork();
                if (!pid)
                {
                    buf.mtype = i+1;
                    break;
                }
        }

        //Каждый дочерний процесс ждёт сообщение и выводит свой номер
        if (!pid)
        {
            msgrcv(qid, &buf, length, i+1, 0);
            printf("%d ", i+1);
            fflush(stdout);
            msgsnd(qid, &buf, length, 0);
            return 0;
        }

        //Посылаем дочерним процессам сообщение
        for (i = 0; i < n; i++)
        {
            buf.mtype = i+1;
            buf.num = 1;
            msgsnd(qid, &buf, length, 0);
            msgrcv(qid, &buf, length, i+1, 0);
        }
        msgctl(qid, IPC_RMID, 0);
        printf("\n");

        return 0;
}
