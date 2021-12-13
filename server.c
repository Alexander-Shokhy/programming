#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>

#define FIFO "server.fifo"
#define pageSIZE 4096
#define sizestrPID 11
#define start_numb 100

typedef struct arg_struct {
    char text[pageSIZE+ 1];
} arg_struct;

pthread_t *thread;
arg_struct *args;

void exit_server (int signo) {
    printf("Server finished working\n");
    free(thread);
    free(args);
    unlink(FIFO);
    exit(EXIT_SUCCESS);
}

void *functhread (void *arg) {
    int fd_file, fd_client, pid_client, read_count;
    char FILE_name[pageSIZE- sizestrPID - 2] = {0};
    char fifo_client[sizestrPID] = {0};
    char buf[pageSIZE] = {0};
    arg_struct * c = (arg_struct *) arg;
    sscanf(c->text, "%d %s", &pid_client, FILE_name);
    sprintf(fifo_client, "%d.fifo", pid_client);
    if ((fd_file = open(FILE_name, O_RDONLY)) < 0) {
        perror("Open File");
        kill(pid_client, SIGUSR1);
    } else {
        if ((fd_client = open(fifo_client, O_WRONLY)) < 0) {
            perror("Open fifo_client");
            kill(pid_client, SIGUSR1);
        } else {
            while ((read_count = read(fd_file, buf, pageSIZE)) > 0) {
                if ((write(fd_client, buf, read_count)) != read_count) {
                    perror("Write client");
                    kill(pid_client, SIGUSR1);
                    break;
                }
            }
            printf("Done\n");
        }
        close(fd_client);
    }
    close(fd_file);

    return NULL;
}

int main() {
    int i, fd_server;
    char buf[pageSIZE];
    signal(SIGTSTP, exit_server);
    signal(SIGINT, exit_server);
    if (mkfifo(FIFO, 0666) < 0) {
        perror("FIFO");
        exit(errno);
    }
    if ((fd_server = open(FIFO, O_RDONLY)) < 0) {
        perror("Open ServerFifo\n");
        exit(errno);
    }
    i = 0;
    while(1) {
        if (read(fd_server, buf, pageSIZE) > 0) {
            if ((i % start_numb) == 0) {
                if ((thread = (pthread_t *) realloc (thread, (i + start_numb) * sizeof(pthread_t))) == NULL) {
                    perror("Can't add threads");
                    free(thread);
                    free(args);
                    exit(errno);
                }
                if ((args = (arg_struct *) realloc (args, (i + start_numb) * sizeof(arg_struct))) == NULL) {
                    perror("Can't add args");
                    free(thread);
                    free(args);
                    exit(errno);
                }
            }
            strcpy(args[i].text, buf);
            if (pthread_create(&thread[i], NULL, functhread,(void *) &args[i]) > 0) {
                perror("Create Thread");
                exit(errno);
            }
            i++;
        }
    }
    unlink(FIFO);
    exit(EXIT_SUCCESS);
}
