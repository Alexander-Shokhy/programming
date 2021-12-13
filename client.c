#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/errno.h>

#define FIFO "server.fifo"
#define pageSIZE 4096
#define sizeforPID 11

char FIFO_client[sizeforPID] = {0};

void exit_client (int signo) {
    fprintf(stderr, "Process stopped");
    unlink(FIFO_client);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    int fd, numb;
    char buf[pageSIZE] = {0};
    char FIFO_client_info[pageSIZE] = {0};
    pid_t pid;
    signal(SIGUSR1, exit_client);
    if (argc < 2) {
        fprintf(stderr, "argc");
        exit(EXIT_FAILURE);
    }
    if ((pid = getpid()) < 0) {
        perror("getpid");
        exit(errno);
    }
    sprintf(FIFO_client, "%d.fifo", pid);
    sprintf(FIFO_client_info, "%d %s", pid, argv[1]);
    if (mkfifo(FIFO_client, 0666) < 0) {
        perror("Can't make FIFO");
	    unlink(FIFO_client);
        exit(errno);
    }
    if ((fd = open(FIFO, O_WRONLY)) < 0) {
        perror("Open ServerFifo");
        close(fd);
	    unlink(FIFO_client);
        exit(errno);
    }
    if ((write(fd, FIFO_client_info, pageSIZE)) != pageSIZE) {
        perror("write");
        close(fd);
        unlink(FIFO_client);
        exit(errno);
    }
    close(fd);
    if ((fd = open(FIFO_client, O_RDONLY)) < 0) {
        perror("Open FIFO");
        close(fd);
        unlink(FIFO_client);
        exit(errno);
    }
    while ((numb = read(fd, buf, pageSIZE)) > 0) {
        //sleep(2);
        if ((write(STDOUT_FILENO, buf, numb)) < 0) {
            perror("write");
            unlink(FIFO_client);
            exit(errno);
        }
    }
    if (numb < 0) {
        perror("Read from FIFO");
        close(fd);
        unlink(FIFO_client);
        exit(errno);
    }
    close(fd);
    unlink(FIFO_client);
    exit(EXIT_SUCCESS);
}
