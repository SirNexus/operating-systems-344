#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
// Name of rooms:
#define ROOM_1 "1"
#define ROOM_2 "2"
#define ROOM_3 "3"
#define ROOM_4 "4"
#define ROOM_5 "5"
#define ROOM_6 "6"
#define ROOM_7 "7"
#define ROOM_8 "8"
#define ROOM_9 "9"
#define ROOM_10 "10"
#define ROOM_RANDOMIZE 50
// Room connections > 3
#define MIN_CONN 3

struct Room {
    char name[256];
    int max_conn;
    int num_conn;
    struct Room *connections;
};

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

int main() {
    int i;
    char names[10][128] = { ROOM_1, ROOM_2, ROOM_3, ROOM_4, ROOM_5, ROOM_6, ROOM_7, ROOM_8, ROOM_9, ROOM_10 };

    // copyNames(names);
    // printRoomNames(names);
    time_t t;
    // Initialize random number generator
    srand((unsigned) time(&t));

    // Initialize Rooms array
    struct Room *rooms = (struct Room *) malloc(7 * sizeof(struct Room));
    initializeRooms(rooms, names);

    // Create all connections in graph
    while (IsGraphFull(rooms) == false){
      AddRandomConnection();
    //   printRoomInfo(rooms);
    }

    makeDir();
    makeFiles(rooms);

    return 0;
}

// Iterate through rooms array and assign each num_conn 
// to random number between MIN_CONN and MAX_CONN
void initializeRooms(struct Room *rooms, char names[10][128]) {
    randomizeRooms(names);
    int i;
    for (i=0; i<7; i++){
        initializeConnections(&rooms[i]);
        strcpy(rooms[i].name, names[i]);
    }
    // printRoomNames(names);
    // printRoomInfo(rooms);

}
// A function to generate a random permutation of arr[]
void randomizeRooms (char names[10][128]){
    int i, j;
    char temp[128];

    // Start from the last element and swap one by one. We don't
    // need to run for the first element that's why i > 0
    for (i = 0; i < ROOM_RANDOMIZE; i++){
        // Pick a random index from 0 to i
        j = rand() % (10);
        // printf ("Random Number: %i\n", j);
 
        // Swap arr[i] with the element at random index
        strcpy(temp, names[i % 10]);
        strcpy(names[i % 10], names[j]);
        strcpy(names[j], temp);
        // printRoomNames(names);
    }
}

// Initialize connections array within each room
void initializeConnections(struct Room *room){
    room->connections = (struct Room *) malloc(6 * sizeof(struct Room));
}

// Returns true if all rooms have 3 to 6 outbound connections, false otherwise
bool IsGraphFull(struct Room * rooms) {
  int i;
  for (i=0; i<7; i++){
      if (rooms[i].num_conn < MIN_CONN){
          return false;
      }
  }
  return true;
}

// Adds a random, valid outbound connection from a Room to another Room
void AddRandomConnection(struct Room *rooms) {
  struct Room *A;
  struct Room *B;

  while(true){
    A = GetRandomRoom(rooms);

    if (CanAddConnectionFrom(A) == true)
      break;
  }

  do
  {
    B = GetRandomRoom(rooms);
  }
  while(CanAddConnectionFrom(B) == false || IsSameRoom(A, B) == true || ConnectionAlreadyExists(A, B) == true);

  ConnectRoom(A, B);  // TODO: Add this connection to the real variables, 
  ConnectRoom(B, A);  //  because this A and B will be destroyed when this function terminates
//   free(A);
//   free(B);
}

// Returns a random Room, does NOT validate if connection can be added
struct Room * GetRandomRoom(struct Room *rooms){
    int temp = rand() % 7;
    // printf("%i\n", temp); 
    return &rooms[temp];
}

// Returns true if a connection can be added from Room x (< 6 outbound connections), false otherwise
bool CanAddConnectionFrom(struct Room *x) {
    if (x->num_conn < 6)
        return true;
    return false;
}
// Returns true if a connection from Room x to Room y already exists, false otherwise
bool ConnectionAlreadyExists(struct Room *x, struct Room *y) {
    int i;
    for (i=0; i<x->num_conn; i++){
        if (!strcmp(x->connections[i].name, y->name))
            return true;
    }
    return false;
}

// Connects Rooms x and y together, does not check if this connection is valid
void ConnectRoom(struct Room *x, struct Room *y){
    x->connections[x->num_conn] = *y;
    x->num_conn++;
}

// Returns true if Rooms x and y are the same Room, false otherwise
bool IsSameRoom(struct Room *x, struct Room *y){
    // char name[256];
    // int max_conn;
    // int num_conn;
    // Room *connections;
    // bool connected;
  
    if (!strcmp(x->name, y->name))
      return true;
    else
        return false;
}

// Utility function for printing room information
void printRoomNames(char names[10][128]){
    int i;
    printf("Rooms: ");
    for (i=0; i<10; i++){
        printf("%s ", names[i]);
    }
    printf("\n");

}
// Utility function for printing room info to screen.
void printRoomInfo(struct Room *rooms){
    int i;
    int j;
    for (i=0; i<7; i++){
        printf("Room %i:\n\tName: %s\n\tConnections: ", i+1, rooms[i].name);
        for (j=0; j<rooms[i].num_conn; j++){
            printf("%s ", rooms[i].connections[j].name);
        }
        printf("\n\tnum connections: %i\n", rooms[i].num_conn);
    }
}
// Make directory for room files
void makeDir(struct Room *rooms){
    char dirName[128];
    sprintf(dirName, "carsonai_rooms_%i", getpid());
    mkdir(dirName, 0777);
}

// Make files for each element in array
void makeFiles(struct Room *rooms){
    char tempName[64];

    sprintf(tempName, "%s_room", ROOM_1);
    createAndFill(tempName, rooms[0]);
    sprintf(tempName, "%s_room", ROOM_2);
    createAndFill(tempName, rooms[1]);
    sprintf(tempName, "%s_room", ROOM_3);
    createAndFill(tempName, rooms[2]);
    sprintf(tempName, "%s_room", ROOM_4);
    createAndFill(tempName, rooms[3]);
    sprintf(tempName, "%s_room", ROOM_5);
    createAndFill(tempName, rooms[4]);
    sprintf(tempName, "%s_room", ROOM_6);
    createAndFill(tempName, rooms[5]);
    sprintf(tempName, "%s_room", ROOM_7);
    createAndFill(tempName, rooms[6]);
}

// Function to create room file
void createAndFill(char *tempName, struct Room room){
    FILE *fp;
    int i;
    char temp[64];
    sprintf(temp, "carsonai_rooms_%i/%s", getpid(), tempName);
    fp = fopen(temp, "w");
    fprintf(fp, "%s\n", room.name);
    fprintf(fp, "%i\n", room.num_conn);
    for (i=0; i<room.num_conn; i++){
        fprintf(fp, "%s\n", room.connections[i].name);
    }
}