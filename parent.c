#include "defs.h"

void displaySharedMemory(int countNums, int* shmem) {
  for(int numCounter = 0; numCounter < countNums; numCounter++) {
    printf("%d\n", shmem[numCounter]);
  }
}

int main(int argc, char *argv[]) {
  // Use unbuffered output
  setvbuf(stdout, NULL, _IONBF, 0);

  puts("Parent: starts");

  puts("Parent: validate command line TODO");

  puts("Parent: requests shared memory");
  size_t sizeShmem = sizeof(int) * argc - 1;
  // ftok to generate unique key
  key_t key = ftok("/home/johnc/Developer/projects/c/danny-r/parentchild/shmfile",65);
  int shmid = shmget(key, sizeShmem, IPC_CREAT|0666);

  puts("Parent: receives shared memory");
  if(shmid == -1) {
    puts("Error requesting shared memory ");
    printf(" Value of errno: %d\n ", errno);
    if(errno == EACCES) {
      puts("Permission denied");
    }
    exit(1);
  }

  puts("Parent: attaches shared memory");
  int *shmem = (int*) shmat(shmid,(void*)0,0);
  if (shmem == (int *)(-1)) {
    perror("shmat");
  }

  puts("Parent: fills shared memory");
  for(int numCounter = 0; numCounter < argc - 1; numCounter++) {
    int num = atoi(argv[numCounter + 1]);
    printf("NUM IS %d\n", num);
    shmem[numCounter] = num;
  }

  puts("Parent: displays shared memory");
  displaySharedMemory(argc - 1, shmem);

  pid_t parentPid = getpid();
  int uniqueId = 1;
  for(int pidCounter = 0; pidCounter < argc - 1; pidCounter++) {
    if(getpid() == parentPid) {
      puts("Parent: forks (each) child process");
      int childPid = fork();
      if(childPid != 0) {
        uniqueId++;
      } else {
        printf("Child %d: starts PID %d\n", uniqueId, getpid());
      }
    }
  }

  // All child processes have been forked
  pid_t myPid = getpid();
  if(myPid != parentPid) {
    printf("Child %d: displays shared memory\n", uniqueId);
    displaySharedMemory(argc - 1, shmem);

    printf("Child %d: displays private unique id\n", uniqueId);
    printf("Child %d: displays its corresponding value %d\n", uniqueId, shmem[uniqueId - 1]);
    printf("Child %d: updates shared memory\n", uniqueId);
    int childValue = shmem[uniqueId - 1];
    childValue = childValue * uniqueId;
    shmem[uniqueId - 1] = childValue;

    printf("Child %d: displays shared memory\n", uniqueId);
    displaySharedMemory(argc - 1, shmem);

    int exitCode = 100 + uniqueId;
    printf("Child %d: exits with code %d\n", uniqueId, exitCode);
    exit(exitCode);
  }

  puts("Parent: waits for (each) child");
  pid_t childPid;
  int status;
  while ((childPid = wait(&status)) > 0) {
    printf("Parent: detects (each) child completion for child PID %d\n", childPid);
    printf("Parent: displays (each) child PID & exit code\n");
    printf("Parent: child %d exited with code %d\n", childPid, WEXITSTATUS(status));
  }

  puts("Parent: displays shared memory");
  displaySharedMemory(argc - 1, shmem);

  puts("Parent: detaches shared memory");
  shmdt(shmem);

  puts("Parent: removes shared memory");
  shmctl(shmid,IPC_RMID,NULL);

  puts("Parent: finished");

  // Flush output buffer when using buffered output
  // See https://unix.stackexchange.com/questions/447898/why-does-a-program-with-fork-sometimes-print-its-output-multiple-times
  // fflush(stdout);



/*
char* parent_message = "hello";  // parent process will write this message
char* child_message = "goodbye"; // child process will then write this one

  int pid = fork();

  if (pid == 0) {
    printf("Child read: %s\n", (char*)shmem);
    memcpy(shmem, child_message, sizeof(child_message));
    printf("Child wrote: %s\n", (char*)shmem);

  } else {
    printf("Parent read: %s\n", (char*)shmem);
    sleep(1);
    printf("After 1s, parent read: %s\n", (char*)shmem);
  }
*/
}
