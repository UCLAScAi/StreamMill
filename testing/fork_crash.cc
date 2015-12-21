#include <iostream>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>

int crash_process() {
  int* i = NULL;
  std::cout << i[1000];
  sleep(1);
}

int main() {
  while (1) {
    int pid = fork();
    if (pid == 0) {
      std::cout << " starting crash process... " << std::endl;
      sleep(1);
      crash_process();
      std::cout << " I am a child with pid " << pid << std::endl;
    } else {
      waitpid(pid, NULL, 0);
    }
  }
  return 1;
}
