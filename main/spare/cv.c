#include <stdio.h>
#include <stdlib.h>
#include <time.h>


#define NAME_SIZE 16
#define queue_size 20
#define BUF_SIZE 128
#define MAP_WIDTH 5
#define MAP_HEIGHT 5

//object data structures
typedef struct location{
    int x;
    int y;
} location_t;

typedef struct user{
    char name[NAME_SIZE];
    int score;
    location_t base_loc;
    location_t user_loc;
}user_t;

typedef struct object_data{
    int map_width;
	int map_height;
    int timeout;
    int max_user;
    struct user * users;
    location_t * item_locations;
    location_t * block_locations;
}object_data_t;

enum entity {
	EMPTY = 0,
	BLOCK = -1,
	ITEM = -9, //item will be -10 ~ -110
	USER = 1, //user wil be 1 ~ 3
	BASE = 9, //base will be 10 ~ 30
};

enum spans {
	UP, 
	DOWN, 
	LEFT, 
	RIGHT
};

char user_color[8][20] = {"#faa8a1", "#ffe479", "#dbe87c", "#a19b8b", "#ea9574", "#ffca79", "#c79465", "#e3dbcf"};


int ** map; // cell
object_data_t Model; //model
char msg_info[BUF_SIZE] = "";
char buf[BUF_SIZE] = "";
int sock;
int my_id;
int num_item, num_block;
int current_num_item;

pthread_mutex_t mutx;
pthread_cond_t cond;
int event_arry[queue_size];
int rear = 0;
int front = 0;
int ** user_pos;


int check_validation(int cmd){
	int user_idx = cmd/4;
	int span = cmd%4;	
	
	int curr_x, curr_y, target_x, target_y, item_target_x, item_target_y;
	curr_x = target_x = item_target_x = user_pos[user_idx][0];
	curr_y = target_y = item_target_y = user_pos[user_idx][1];
	switch(span){
		case UP:		
			if((target_y = (curr_y - 1)) < 0) return 0;//out of array
			else if(map[target_y][target_x] == EMPTY) return 1; //empty
			else if(map[target_y][target_x] > BASE) return 1; //base
			else if(map[target_y][target_x] < ITEM){ 
				if((item_target_y = (target_y - 1)) < 0) return 0; //item and non-movabel
				if(map[item_target_y][item_target_x] == EMPTY) return map[target_y][target_x]; //item and movable
				if((map[item_target_y][item_target_x] > BASE) && ((map[item_target_y][item_target_x]) == ((user_idx + 1) * 10))) return (0 - map[target_y][target_x]);
				if(map[item_target_y][item_target_x] > BASE) return map[target_y][target_x]; //item and movable as other's base
				else return 0;	//others (block, user, base)
			}else return 0;	
			break;

		case DOWN:
			if((target_y = (curr_y + 1)) > Model.map_height -1 ) return 0;//out of array
			else if(map[target_y][target_x] == EMPTY) return 1; //empty
			else if(map[target_y][target_x] > BASE) return 1; //base
			else if(map[target_y][target_x] < ITEM){ 
				if((item_target_y = (target_y + 1)) > MAP_HEIGHT - 1) return 0; //item and non-movabel
				if(map[item_target_y][item_target_x] == EMPTY) return map[target_y][target_x]; //item and movable
				if((map[item_target_y][item_target_x] > BASE) && ((map[item_target_y][item_target_x]) == ((user_idx + 1) * 10))) return (0 - map[target_y][target_x]);
				if(map[item_target_y][item_target_x] > BASE) return map[target_y][target_x]; //item and movable as other's base
				else return 0;	//others (block, user, base)
			}else return 0;	
			break;


		case LEFT:
			if((target_x = (curr_x - 1)) < 0) return 0;//out of array
			else if(map[target_y][target_x] == EMPTY) return 1; //empty
			else if(map[target_y][target_x] > BASE) return 1; //base
			else if(map[target_y][target_x] < ITEM){ 
				if((item_target_x = (target_x - 1)) < 0) return 0; //item and non-movabel
				if(map[item_target_y][item_target_x] == EMPTY) return map[target_y][target_x]; //item and movable
				if((map[item_target_y][item_target_x] > BASE) && ((map[item_target_y][item_target_x]) == ((user_idx + 1)  * 10))) return (0 - map[target_y][target_x]);
				if(map[item_target_y][item_target_x] > BASE) return map[target_y][target_x]; //item and movable as other's base

				else return 0;	//others (block, user, base)
			}else return 0;	
			break;

		case RIGHT:
			if((target_x = (curr_x + 1)) > Model.map_width  - 1) return 0;//out of array
			else if(map[target_y][target_x] == EMPTY) return 1; //empty
			else if(map[target_y][target_x] > BASE) return 1; //base
			else if(map[target_y][target_x] < ITEM){ 
				if((item_target_x = (target_x + 1)) > MAP_WIDTH -1) return 0; //item and non-movabel
				if(map[item_target_y][item_target_x] == EMPTY) return map[target_y][target_x]; //item and movable
				if((map[item_target_y][item_target_x] > BASE) && ((map[item_target_y][item_target_x]) == ((user_idx + 1) * 10))) return (0 - map[target_y][target_x]);
				if(map[item_target_y][item_target_x] > BASE) return map[target_y][target_x]; //item and movable as other's base
				else return 0;	//others (block, user, base)
			}else return 0;	
			break;
	}

}



int main(int argc, char * argv[]) {

	// create map
	int i, j;
    map = (int **) malloc (sizeof(int *) * MAP_WIDTH);
	for(i = 0; i < MAP_WIDTH; i++){
		map[i] = (int *)malloc(sizeof(int) * MAP_HEIGHT);
	} 

	int n_user, c;

	// read how many users are in map
	if (scanf("%d", &n_user) != 1) {
        fprintf(stderr, "Error reading from n_user.\n");
        exit(EXIT_SUCCESS);
    }
	printf("%d\n", n_user);

	// create user_pos
	user_pos = (int **)malloc(n_user * sizeof(int *));
    if (user_pos == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(EXIT_SUCCESS);
    }
	for (i = 0; i < n_user; i++) {
        user_pos[i] = (int *)malloc(2 * sizeof(int));
        if (user_pos[i] == NULL) {
            fprintf(stderr, "Memory allocation failed.\n");
            exit(EXIT_SUCCESS);
        }
    }

	// fill map
	for (i = 0; i < MAP_HEIGHT; i++) {
		for (j = 0; j < MAP_WIDTH; j++) {
			if (scanf("%d", &c) != 1) {
                fprintf(stderr, "Error reading from cell value.\n");
                exit(EXIT_SUCCESS);
            }
			if (c == 0 || c == -1 || c == 10 || c == -10) {
				printf("%d ", c);
				map[i][j] = c;
			}
			else if (1 <= c && c <= n_user) {
				// printf("herhe\n");
				// printf("%d %d\n", i, j);
				printf("%d ", c);
				map[i][j] = c;
				user_pos[c-1][0] = j;	// x
				user_pos[c-1][1] = i;	// y

				// for (i = 0; i < n_user; i++) {
				// 	printf("%d: [%d, %d]\n", i, user_pos[i][0], user_pos[i][1]);
				// }
			}
			else {
				fprintf(stderr, "abnormal input\n");
				printf("%d ", c);
				exit(EXIT_SUCCESS);
			}
		}
		printf("\n");
	}

	// print user_pos
	for (i = 0; i < n_user; i++) {
		printf("%d: [%d, %d]\n", i, user_pos[i][0], user_pos[i][1]);
	}

	// print map
	for (i = 0; i < MAP_HEIGHT; i++) {
		for (j = 0; j < MAP_WIDTH; j++) {
			map[i][j]
		}
	}


	// move user
	for (i = 0; i < n_user * 4; i++) {
		if (check_validation(i) == 0) {
			printf("move failed\n");
		}
		else {
			printf("move succeed\n");
		}
		printf("\n");
	}

}