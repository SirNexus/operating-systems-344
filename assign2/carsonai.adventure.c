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

struct Room {
    char name[256];
    int max_conn;
    int num_conn;
    struct Room *connections;
};

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
void freeRooms(struct Room *);
void freePath(char **, int);
void printPath(char **, int);
void printRoomInfo(struct Room *);

pthread_mutex_t myMutex = PTHREAD_MUTEX_INITIALIZER;

int main (){

    time_t t;
    // Initialize Rooms array
    struct Room *rooms = (struct Room *) malloc(7 * sizeof(struct Room));

    // Initialize random number generator
    srand((unsigned) time(&t));
    
    initializeRooms(rooms);

    playGame(rooms);
    freeRooms(rooms);
    return 0;
}

// Open all files in directory and store into data structure
void initializeRooms(struct Room *rooms) {
    int i;
    char dirName[64];
    for (i=0; i<7; i++){
        initializeConnections(&rooms[i]);
    }
    getDirectory(dirName);
    // printf("Directory: %s\n", dirName);
    getRoomInfo(rooms, dirName);
    // utility function to print all room info, not necessary in final game
    // printRoomInfo(rooms);

}

// Initialize a connections array of 7 element array "rooms"
void initializeConnections(struct Room *room){
    room->connections = (struct Room *) malloc(6 * sizeof(struct Room));
}

// Get room information from files within directory
void getRoomInfo(struct Room *rooms, char *dirName) {
    char tempName[64];
    int i=0;
    sprintf(tempName, "./%s", dirName);
    DIR *dirp = opendir(tempName);
    struct dirent *dp;
    while ((dp = readdir(dirp)) != NULL) {
        if (!strcmp (dp->d_name, "."))
            continue;
        if (!strcmp (dp->d_name, ".."))    
            continue;
        // printf("i: %i, File: %s\n", i, dp->d_name);
        sprintf(tempName, "%s/%s", dirName, dp->d_name);
        fillRoom(&rooms[i], tempName);
        i++;
    }
}

// fill room with data from file read in
void fillRoom(struct Room *room, char *file) {
    FILE *fp;
    char *temp = (char *) malloc (64 * sizeof(char));
    int i, lines;
    fp = fopen(file, "r");
    if (fp == NULL) {
        printf("%s", file);
        perror("Failed: ");
    }
    // getline(&line, &len, file);
    // getLine is a function for getting line from file
    getLine(temp, fp);
    strcpy(room->name, temp);
    // printf("Name: %s\n", room->name);
    getLine(temp, fp);
    room->num_conn = atoi(temp);
    // printf("Num Conn: %i\nConnections: ", room->num_conn);
    for (i=0; i<room->num_conn; i++){
        getLine(temp, fp);
        strcpy(room->connections[i].name, temp);
        // printf("%s ", room->connections[i].name);
    }
    // printf("\n");

    free(temp);
    fclose(fp);
}

// Function for accurately using getline function
void getLine(char *line, FILE *fp){
    size_t len;
    char *temp = NULL;
    int n;
    n = getline(&temp, &len, fp);
    if (temp[n - 1] == '\n'){
        temp[n - 1] = '\0';
    }
    strcpy(line, temp);
}

// Taken from the readings online, this function gets the latest directory
// for use in the game.
void getDirectory(char *dName){
    int newestDirTime = -1; 
    char targetDirPrefix[32] = "carsonai_rooms_";
    memset(dName, '\0', sizeof(dName));
  
    // Holds the directory we're starting in
    DIR* dirToCheck;
    // Holds the current subdir of the starting dir
    struct dirent *fileInDir; 
    // Holds information we've gained about subdir
    struct stat dirAttributes; 
  
    // Open current directory to check for lastest modified
    dirToCheck = opendir("."); 
  
    // Make sure the current directory could be opened
    if (dirToCheck > 0) 
    {
      while ((fileInDir = readdir(dirToCheck)) != NULL)
      {
        if (strstr(fileInDir->d_name, targetDirPrefix) != NULL)
        {
        //   printf("Found the prefex: %s\n", fileInDir->d_name);
        // Get attributes of the entry
          stat(fileInDir->d_name, &dirAttributes); 
          if ((int)dirAttributes.st_mtime > newestDirTime) // If this time is bigger
          {
            newestDirTime = (int)dirAttributes.st_mtime;
            memset(dName, '\0', sizeof(dName));
            strcpy(dName, fileInDir->d_name);
            // printf("Newer subdir: %s, new time: %d\n",
            // fileInDir->d_name, newestDirTime);
          }
        }
      }
    }
  
    closedir(dirToCheck); // Close the directory we opened
  
    printf("Newest entry found is: %s\n", dName);
}

// Set start room, keep asking room until user finds the end
void playGame(struct Room *rooms){

    int start_room = rand() % 7;
    int end_room = 0;
    char *next_room = NULL;
    int cur_room_num = 0, temp;
    bool end = false;
    char **path = NULL;
    int path_length = 0;
    int max_path = 0;
    pthread_t thread;

    pthread_mutex_lock(&myMutex);
    initializePath(&path, &max_path);

    do {
        end_room = rand() % 7;
    }while (start_room == end_room);

    cur_room_num = start_room;
    // Choosing to not include first room in path. Per example online.
    // addToPath(rooms[cur_room_num], path, &path_length);
    while (!end){
        next_room = printAndGetInfo(rooms[cur_room_num]);
        // printf("Current Room: %s", cur_room);
        temp = cur_room_num;

        cur_room_num = getRoomNumber(next_room, rooms[cur_room_num], rooms);
        if (!strcmp(next_room, "time")){
            // Main thread should lock variable, then call write time to write to the file
            // printf("In main: creating thread %d\n", 1);
            pthread_mutex_unlock(&myMutex);
            pthread_create(&thread, NULL,
                writeTime, (void *)&end);
            readTime();
            cur_room_num = temp;
        }
        else if (cur_room_num == -1){
            printf("HUH? I DON’T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
            cur_room_num = temp;
        }
        else {
            addToPath(rooms[cur_room_num], path, &path_length);
            // Check if path length is more than half of max path length
            if (checkPath(path_length, max_path)){
                increaseMaxPath(&path, path_length, &max_path);
            }
            if (cur_room_num == end_room){
                printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
                printf("YOU TOOK %i STEPS. YOUR PATH TO VICTORY WAS:\n", path_length);
                // Print path of rooms user took
                printPath(path, path_length);
                freePath(path, max_path);
                end = true;
                // printPath(path, path_length);
            }
        }
    }
}

// Initialize path array for holding room names of path
void initializePath(char ***path, int *max_path){
    int i;
    // Start at holding 5 rooms
    *path = (char **) malloc (5 * sizeof(char *));
    for (i=0; i<5; i++){
        (*path)[i] = (char *) malloc (64 * sizeof(char));
    }
    *max_path = 5;
}

// print info about current room and get input from user
char * printAndGetInfo(struct Room room){
    int i;
    char *name = (char *) malloc (64 * sizeof(char));
    // Current location
    printf("CURRENT LOCATION: %s\n", room.name);
    printf("POSSIBLE CONNECTIONS: ");
    
    for (i=0; i<room.num_conn-1; i++){
        printf("%s, ", room.connections[i].name);
    }
    printf("%s.\n", room.connections[room.num_conn - 1].name);
    // Get input from user
    printf("WHERE TO? >");
    scanf("%s", name);
    printf("\n");
    return name;
}

// Get room number in 7 element array from name passed in from user
int getRoomNumber(char *cur_room, struct Room room, struct Room *rooms){
    int i, j;

    // check passed in room for connection name
    for (i=0; i<room.num_conn; i++){
        // check which element in 7 element array matches name
        if (!strcmp(room.connections[i].name, cur_room))
            for (j=0; j<7; j++){
                if (!strcmp(room.connections[i].name, rooms[j].name))
                    return j;
            }
    }
    return -1;
}

// Function to write the current time in format specified in assignment
// This will be written by the second thread
void * writeTime(void *argument){
    FILE *fp;
    time_t rawtime;
    struct tm * timeinfo;
    char date[64];

    // lock mutex so that it has time to write to file
    pthread_mutex_lock(&myMutex);
    // Open time file for writing
    fp = fopen("currentTime.txt", "w");
  
    // Get current time
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    // Convert time to format specified online
    strftime (date, 80, "%I:%M, %A, %B %d, %Y", timeinfo);
    if (date[0] == '0'){
        memmove(date, date+1, strlen(date));
    }
    // print time to file
    fprintf (fp, "%s", date);
    fclose(fp);

    // unlock mutex so main thread may read file
    pthread_mutex_unlock(&myMutex);
}

// Function for reading time from currentTime.txt file
// This will be read by the main thread
void readTime(){
    FILE *fp;
    time_t rawtime;
    char date[64];
    
    sleep(1);
    // Lock mutex so that the write can't write while
    // this is reading
    pthread_mutex_lock(&myMutex);
    // Open time file for writing
    fp = fopen("currentTime.txt", "r");

    // Read current time
    getLine(date, fp);
    printf (" %s\n\n", date);
    fclose(fp);

}

// Add current room to path
void addToPath(struct Room room, char **path, int *path_length){
    strcpy(path[*path_length], room.name);
    // placeholder print statement to ensure room prints correctly
    // printf("Room Name: %s", path[*path_length]);
    (*path_length)++;
}

// Check path to see if path is greater than or equal to half of the array
// If it is return true, else false
bool checkPath(int path_length, int max_path){
    if ((path_length - 1) * 2 / max_path >= 1){
        return true;
    }
    return false;
}

// Increase max path length if current path is over half
// of the max capacity
void increaseMaxPath(char ***path, int path_length, int *max_path) {
    // placeholder print to show increaseMaxPath
    // printf("This is a placeholder inside increaseMaxPath\n");
    int i;
    char **temp = (char **) malloc ((*max_path) * 2 * sizeof(char *));
    *max_path *= 2;
    for (i=0; i<(*max_path); i++){
        temp[i] = (char *) malloc (64 * sizeof(char));
    }

    // copy over and free path to temp
    for (i=0; i<path_length; i++){
        strcpy(temp[i], (*path)[i]);
        free((*path)[i]);
    }
    free(*path);

    *path = temp;

    // placeholder print to show max_path is getting increased
    // printf("New Max Path: %i\n", *max_path);

}

void freeRooms(struct Room *rooms){
    int i;

    for (i=0; i<7; i++){
        free(rooms[i].connections);
    }
    free(rooms);
}

void freePath(char **path, int max_path){
    int i;
    for (i=0; i<max_path; i++){
        free(path[i]);
    }
    free(path);
}

// Utility function to print existing path and max_path
void printPath(char **path, int path_length){
    int i;
    
    // printf("THIS IS YOUR PATH of length %i: \n", path_length);
    for (i=0; i<path_length; i++){
        printf("%s\n", path[i]);
    }
    // printf("\n");
    // Free path after printing
    
}

// Utility function for printing room information to screen
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