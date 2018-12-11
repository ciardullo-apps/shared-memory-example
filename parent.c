#include "defs.h"

// The one and only global variable
int uniqueId;

int usage(int argc, char *argv[]) {
  puts("Parent: validate command line");
  if(argc < 2 || argc > 8) {
    printf("Usage: %s n ... (up to 7 digits)", argv[0]);
    return FALSE;
  }
  // See man strtol for how to validate args for numbers
  return TRUE;
}

int requestSharedMemory(int countOfNumbers) {
  puts("Parent: requests shared memory");
  size_t sizeShmem = sizeof(int) * countOfNumbers;
  key_t key = IPC_PRIVATE;  // Accessible by child processes
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

  return shmid;
}

int* receiveSharedMemory(int shmid) {
  puts("Parent: attaches shared memory");
  int *shmem = (int*) shmat(shmid,(void*)0,0);
  if (shmem == (int *)(-1)) {
    perror("shmat");
    exit(2);
  }

  return shmem;
}

void fillSharedMemory(int* shmem, int countOfNumbers, char* argv[]) {
  puts("Parent: fills shared memory");
  for(int numCounter = 0; numCounter < countOfNumbers; numCounter++) {
    int num = atoi(argv[numCounter + 1]);
    shmem[numCounter] = num;
  }
}

void displaySharedMemory(char* callerName, int* shmem, int countOfNumbers) {
  printf("%s: displays shared memory\n", callerName);
  for(int numCounter = 0; numCounter < countOfNumbers; numCounter++) {
    printf("\t %s: read %d\n", callerName, shmem[numCounter]);
  }
}

int performChildWork(int* shmem, int uniqueId, int countOfNumbers) {
  // Set trace message to begin with Child #, where # is the uniqueId
  char childDescription[10];
  sprintf(childDescription, "Child %d", uniqueId);

  displaySharedMemory(childDescription, shmem, countOfNumbers);

  printf("Child %d: displays private unique id\n", uniqueId);
  printf("Child %d: displays its corresponding value %d\n", uniqueId, shmem[uniqueId - START_UNIQUE_ID]);
  printf("Child %d: updates shared memory\n", uniqueId);
  int childValue = shmem[uniqueId - START_UNIQUE_ID];
  childValue = childValue * uniqueId;
  shmem[uniqueId - START_UNIQUE_ID] = childValue;

  displaySharedMemory(childDescription, shmem, countOfNumbers);

  int exitCode = 100 + uniqueId;
  return exitCode;
}

void waitForChildren(int numParams, int pids[numParams]) {
  pid_t childPid;
  int status;
  for(int pidCounter = 0; pidCounter < numParams; pidCounter++) {
    printf("Parent: waits for (each) child pid %d\n", pids[pidCounter]);
    waitpid(pids[pidCounter], &status, 0);
    printf("Parent: detects (each) child completion for child PID %d\n", pids[pidCounter]);
    // printf("Parent: displays (each) child PID & exit code\n");
    printf("Parent: child %d exited with code %d\n", pids[pidCounter], WEXITSTATUS(status));
  }
}

void spawnChildProcesses(int countOfNumbers, pid_t parentPid, int pids[countOfNumbers]) {
  uniqueId = START_UNIQUE_ID;
  for(int pidCounter = 0; pidCounter < countOfNumbers; pidCounter++) {
    if(getpid() == parentPid) {
      puts("Parent: forks (each) child process");
      int childPid = fork();
      if(childPid != 0) {
        pids[pidCounter] = childPid;
        uniqueId++;
      } else {
        printf("Child %d: starts PID %d\n", uniqueId, getpid());
      }
    }
  }
}

void detachSharedMemory(int* shmem) {
  puts("Parent: detaches shared memory");
  shmdt(shmem);
}

void removeSharedMemory(int shmid) {
  puts("Parent: removes shared memory");
  shmctl(shmid,IPC_RMID,NULL);
}

int main(int argc, char *argv[]) {
  // Use unbuffered output
  setvbuf(stdout, NULL, _IONBF, 0);

  // Flush output buffer when using buffered output
  // See https://unix.stackexchange.com/questions/447898/why-does-a-program-with-fork-sometimes-print-its-output-multiple-times
  // fflush(stdout);

  // Trace messages
  puts("Parent: starts");

  if(!usage(argc, argv)) {
    exit(1);
  }

  int numParams = argc - 1;

  int shmid = requestSharedMemory(numParams);

  int* shmem = receiveSharedMemory(shmid);

  fillSharedMemory(shmem, numParams, argv);

  displaySharedMemory("Parent", shmem, numParams);

  // Spawn child processes
  pid_t parentPid = getpid();
  int pids[numParams];
  spawnChildProcesses(numParams, parentPid, pids);

  // All child processes have been forked
  pid_t myPid = getpid();
  if(myPid != parentPid) {
    int exitCode = performChildWork(shmem, uniqueId, numParams);
    printf("Child %d: exits with code %d\n", uniqueId, exitCode);
    exit(exitCode);
  }

  waitForChildren(numParams, pids);

  displaySharedMemory("Parent", shmem, numParams);

  detachSharedMemory(shmem);

  removeSharedMemory(shmid);

  puts("Parent: finished");
}
