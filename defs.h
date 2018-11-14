#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // Needed for getpid() and fork()
#include <errno.h>
#include <sys/wait.h>
#include <sys/shm.h>

#define TRUE 1
#define FALSE 0
#define START_UNIQUE_ID 10
