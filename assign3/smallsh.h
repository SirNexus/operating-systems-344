#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

void catchSIGINT(int);
void catchSIGTSTP(int);
int getLine(char **);
void expandCommand(char **);
int tokenizeCommand(char *, char **);
bool builtInCommands(char **, int *, int, int *, int);
void forkProcess(char **, int *, int, int *, int *, bool);
void executeCommand(char **, int *, int, bool);
void redirectIO(char **, bool);
void checkBackground(int *, int *, int *);
void getStatus(int *);
void removeElem(int *, int *, int *);

// Built-in functions
void exitProgram();
int changeDirectory(char *, int);
void printStatus(int);

bool foregroundOnly = false;