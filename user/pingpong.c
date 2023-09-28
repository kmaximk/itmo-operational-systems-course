#include "user/user.h"
#define BUFSIZE 512

void read_from_pipe(char *buf, int fd) {
  printf("%d: got ", getpid());
  int ret = read(fd, buf, BUFSIZE - 1);
  while (ret > 0) {
    printf("%s", buf);
    ret = read(fd, buf, BUFSIZE - 1);
  }
  if (ret < 0) {
    fprintf(2, "Error when reading\n");
    exit(1);
  }
  close(fd);
}

int main(int argc, char *argv[]) {
  int fd1[2];
  int fd2[2];
  if (pipe(fd1) < 0 || pipe(fd2) < 0) {
    fprintf(2, "Error when creating pipe\n");
    exit(1);
  }
  int pid = fork();
  char buf[BUFSIZE];
  if (pid == 0) {
    close(fd1[1]);
    close(fd2[0]);
    read_from_pipe(buf, fd1[0]);
    if (write(fd2[1], "pong\n", sizeof("pong\n")) < sizeof("pong\n")) {
      fprintf(2, "Error when writing\n");
      exit(1);
    }
    close(fd2[1]);
    exit(0);
  } else if (pid > 0) {
    close(fd2[1]);
    close(fd1[0]);
    if (write(fd1[1], "ping\n", sizeof("ping\n")) < sizeof("ping\n")) {
      fprintf(2, "Error when writing\n");
      exit(1);
    }
    close(fd1[1]);
    wait(0);
    read_from_pipe(buf, fd2[0]);
    exit(0);
  } else {
    close(fd1[0]);
    close(fd1[1]);
    close(fd2[0]);
    close(fd2[1]);
    fprintf(2, "Fork error\n");
    exit(1);
  }
}