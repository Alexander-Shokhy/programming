#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

int main (int argc, char **argv)
{
        int i, j, n, mf;
        int buf;
        char c;
        pid_t pid;
        fd_set set[100];
        int ret_select;
        int fd[100][2], check[100][2];
        n = atoi(argv[2]);


        for (i = 0; i < n; i++)
        {
                if (pipe(fd[i]) < 0)
                {
                        printf("Can\'t create pipe");
                        exit(-1);
                }

                if (pipe(check[i]) < 0)
                {
                        printf("Can\'t create pipe");
                        exit(-1);
                }

                fcntl(fd[i][0], F_SETFL, O_NONBLOCK);
                fcntl(fd[i][1], F_SETFL, O_NONBLOCK);

                fcntl(check[i][0], F_SETFL, O_NONBLOCK);
                fcntl(check[i][1], F_SETFL, O_NONBLOCK);

                FD_ZERO(&set[i]);
                FD_SET(check[i][0], &set[i]);
        }




        for (i = 0; i < n; i++)
        {
                pid = fork();
                if (!pid)
                        break;
        }


        if (pid)//Родитель
        {
                if (n > 1)
                        for (i = 1; i < n; i++)
                        {
                                close(fd[i][0]);
                                close(fd[i][1]);

                                close(check[i][0]);
                                close(check[i][1]);
                        }

                close(fd[0][0]);
                close(check[0][0]);

                //Открываем файл
                if ((mf = open(argv[1], O_RDONLY|O_NONBLOCK)) < 0 )
                {
                        perror("Ошибка при открытии файла");
                        exit(-1);
                }

                //Читаем файл и записываем в pipe
                while ((buf = read(mf, &c, 1)) > 0)
                {

                        write(fd[0][1], &c, 1);
                }

                write(check[0][1], " ", 1);

                close(check[0][1]);

                return 0;
        }

        //Дети
        close(fd[0][1]);

        if (i != n-1)
        {
                close(fd[n-1][0]);
                close(check[n-1][0]);

                for (j = 0; j < n-1; j++)
                 {
                         if (j != i)
                         {
                                 close(fd[j][0]);
                                 close(fd[j+1][1]);

                                 close(check[j][0]);
                                 close(check[j+1][1]);
                         }

                 }
        }
                else
                {
                        for (j = 0; j < n-1; j++)
                        {
                                close(fd[j][0]);
                                close(check[j][0]);
                        }
                        for (j = 0; j < n; j++)
                        {
                                close(fd[j][1]);
                                close(check[j][1]);
                        };
                }

        ret_select = select (check[i][0]+1, &set[i], NULL, NULL, NULL);


        //Считываем из одного pipe в другой
        while((buf = read(fd[i][0], &c, 1)) > 0)
        {

              if (i == n-1)
                      write(STDOUT_FILENO, &c, 1);
              else
              {
                      write(fd[i+1][1], &c, 1);
              }


         }

        close(fd[i][0]);
        close(check[i][0]);

        if (i != n-1)
        {
                close(fd[i+1][1]);
                write(check[i+1][1], " ", 1);
                close(check[i+1][1]);
        }


        return 0;
}
                                                                                                                                                                                                                                                                                               157,1       Внизу
