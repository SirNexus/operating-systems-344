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
#define ROOM_RANDOMIZE 0
// Room connections 3 - 6
#define MAX_CONN 6
#define MIN_CONN 3

struct Room {
    char name[256];
    int max_conn;
    int num_conn;
    struct Room *connections;
};