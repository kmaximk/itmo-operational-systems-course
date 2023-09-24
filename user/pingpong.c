//
// Created by Максим on 24.09.2023.
//
// #include "kernel/pipe.h"
#include "user/user.h"
int main(int argc, char *argv[]) {
  // printf("%d", pid);
  int fd1[2];
  int fd2[2];
  int res = pipe(fd1);
  pipe(fd2);
  printf("%d %d %d", fd1[0], fd1[1], res);
  int pid = fork();
  char buf[512];
  char buff[10];
  //int status;
  if (pid == 0) {
    close(fd1[1]);
    int cnt = 0;
    while (read(fd1[0], buff, 1) > 0) {
      //printf("%s", buff);
      memcpy(buf + cnt, buff, sizeof (char));
      //printf("%s\n", buf);
      cnt++;
    }
    close(fd1[0]);
    printf("child id %d %d: got %s",getpid(), pid, buf);
    write(fd2[1], "pong\n", 5);
    close(fd2[1]);
    close(fd2[0]);
    exit(0);
  } else {
    write(fd1[1], "ping\n", 5);
    //write(fd[1], "\n", 1);
    close(fd1[1]);
    close(fd2[1]);
    //wait(0);
    int cnt = 0;
    while (read(fd2[0], buff, 1) > 0) {
      //printf("%s", buff);
      memcpy(buf + cnt, buff, sizeof (char));
      //printf("%s\n", buf);
      cnt++;
    }
    printf("parent id %d %d: got %s",getpid(), pid, buf);
    close(fd2[0]);
    close(fd1[0]);
    exit(0);
  }
  printf("fafaf");
}