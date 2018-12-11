This C program uses shared memory and fork() to spawn child processes, each with a unique ID, which is used arbitrarily to perform an arithmetic operation on a value in shared memory that corresponds to the child process.

Usage: ./parent 0-9 [0-9]...

One child process is forked for each parameter and the specified value is stored in an array in shared memory. This value is read, changed, and written by a corresponding child process.

This program demonstrates how to:
  * fork a child process
  * initialize, attach, detach, and remove shared memory
  * wait for child processes to finish and examine their exit codes
