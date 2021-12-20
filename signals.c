//sigfillset полностью инициализирует набор set, в котором содержатся все сигналы.
#define byte 128

#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int j = byte, s = 0;
pid_t pid;

// SIGUSR1
void true(int signo)
{
  s += j;
  j /= 2;
  kill(pid, SIGUSR1);
}

// SIGUSR2
void false(int signo)
{
  j /= 2;
  kill(pid, SIGUSR1);
}

// SIGCHLD
void childexit(int signo)
{
  exit(0);
}

// SIGALRM
void parentexit(int signo)
{
  exit(0);
}


void nothing(int signo)
{}

int main(int argc, char ** argv)
{
        unsigned int i, fd;
        char c;
        pid_t ppid;
        sigset_t set;

        if (argc != 2)
        {
                perror("Должно быть 2 аргумента");
                exit(-1);
        }


        //При  SIGUSR1 выполняем true()
        struct sigaction True;
        memset(&True, 0, sizeof(True));
        True.sa_handler = true;
        sigfillset(&True.sa_mask);


        // При SIGUSR2 выполняем false()
        struct sigaction False;
        memset(&False, 0, sizeof(False));
        False.sa_handler = false;
        sigfillset(&False.sa_mask);


        //При SIGCHLD выходим
        struct sigaction Childexit;
        memset(&Childexit, 0, sizeof(Childexit));//Заполняем нулями
        Childexit.sa_handler = childexit;
        sigfillset(&Childexit.sa_mask);



        sigaction(SIGUSR1, &True, NULL);
        sigaction(SIGUSR2, &False, NULL);
        sigaction(SIGCHLD, &Childexit, NULL);

        sigaddset(&set, SIGUSR1);
        sigaddset(&set, SIGUSR2);
        sigaddset(&set, SIGCHLD);



        sigprocmask(SIG_BLOCK, &set, NULL );
        sigemptyset(&set);


        ppid = getpid();
        pid = fork();


        if (!pid)
        {
                sigemptyset(&set);

                // При SIGUSR1 делаем nothing()
                struct sigaction Nothing;
                memset(&Nothing, 0, sizeof(Nothing));
                Nothing.sa_handler = nothing;
                sigfillset(&Nothing.sa_mask);


                // При SIGALRM делаем parentexit()
                struct sigaction Parentexit;
                memset(&Parentexit, 0, sizeof(Parentexit));
                Parentexit.sa_handler = parentexit;
                sigfillset(&Parentexit.sa_mask);



                sigaction(SIGUSR1, &Nothing, NULL);
                sigaction(SIGALRM, &Parentexit, NULL);



                if ((fd = open(argv[1], O_RDONLY)) < 0 )
                {
                        perror("Ошибка при открытии файла");
                        exit(-1);
                }


                while (read(fd, &c, 1) > 0)
                {
                        alarm(1);

                        for (i = byte; i >= 1; i /= 2)
                        {
                                if (i & c)
                                        kill(ppid, SIGUSR1);
                                else
                                        kill(ppid, SIGUSR2);
                                sigsuspend(&set);
                        }
                }
                return 0;
        }

        // Печать в stdout
        while (1)
        {
                if(!j)
                {
                        printf("%c", s);
                        fflush(stdout);

                        s = 0;
                        j = byte;
                }
                sigsuspend(&set);
        };

        return 0;
}
                                                                                                                                                                                                                                                                                               165,1       Внизу
