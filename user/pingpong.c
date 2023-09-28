#include "user/user.h"

void read_from_pipe(char *buf, int fd) {
  printf("%d: got ", getpid());
  while (read(fd, buf, 511) > 0) {
    printf("%s", buf);
  }
  close(fd);
}

int main(int argc, char *argv[]) {
  int fd1[2];
  int fd2[2];
  if (pipe(fd1) < 0 || pipe(fd2) < 0) {
    fprintf(2, "Error when creating pipe");
    exit(1);
  }
  int pid = fork();
  char buf[512];
  if (pid == 0) {
    close(fd1[1]);
    close(fd2[0]);
    read_from_pipe(buf, fd1[0]);
    if (write(fd2[1], "pong\n", 5) < 5) {
      fprintf(2, "Error when writing\n");
      exit(1);
    }
    close(fd2[1]);
    exit(0);
  } else if (pid > 0) {
    close(fd2[1]);
    close(fd1[0]);
    if (write(fd1[1], "ping\n", 5) < 5) {
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