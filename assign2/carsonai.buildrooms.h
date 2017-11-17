#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

void initializeRooms(struct Room *, char [10][128]);
void randomizeRooms(char [10][128]);
void initializeConnections(struct Room *);
bool IsGraphFull(struct Room *);
void AddRandomConnection(); 
struct Room * GetRandomRoom(struct Room *);
bool CanAddConnectionFrom(struct Room *);
bool ConnectionAlreadyExists(struct Room *, struct Room *);
void ConnectRoom(struct Room *, struct Room *);
bool IsSameRoom(struct Room *, struct Room *); 
void printRoomNames(char [10][128]);
void printRoomInfo(struct Room *);
void makeDir();
void makeFiles(struct Room *);
void createAndFill(char *, struct Room);