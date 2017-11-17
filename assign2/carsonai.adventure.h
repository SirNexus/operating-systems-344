#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

void initializeRooms(struct Room *);
void initializeConnections(struct Room *);
void getRoomInfo(struct Room *, char *);
void fillRoom(struct Room *, char *);
void getLine(char *, FILE *);
void getDirectory(char *);
void playGame(struct Room *);
void getRoomName(struct Room *);
char * printAndGetInfo(struct Room);
int getRoomNumber(char *, struct Room, struct Room *);
void initializePath(char ***, int *);
void * writeTime(void *);
void readTime();
void addToPath(struct Room, char **, int *);
bool checkPath(int, int);
void increaseMaxPath(char ***, int, int *);
void printPath(char **, int);
void printRoomInfo(struct Room *);
