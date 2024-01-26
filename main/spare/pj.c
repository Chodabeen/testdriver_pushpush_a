#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include "../../cJSON.h"

#define NAME_SIZE 16
#define queue_size 20
#define BUF_SIZE 128

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

// Still have to fix... json parsing

int parseJson(char * jsonfile) {

    cJSON* root;
	root = cJSON_Parse(jsonfile);
	if (root == NULL) {
		printf("JSON parsing error: %s\n", cJSON_GetErrorPtr());
        	return 1;
    }
    fprintf(stderr, "parsed succeed\n");
        
	cJSON * timeout = cJSON_GetObjectItem(root, "timeout");
    if (timeout == NULL) {
        // timeout object doesn't exist
        return 1;
    } 
    Model.timeout = timeout->valueint;

	cJSON * max_user = cJSON_GetObjectItem(root, "max_user");
    if (max_user == NULL) {
        // max_user object doesn't exist
        return 1;
    }
    Model.max_user = max_user->valueint;
    
	cJSON * map = cJSON_GetObjectItem(root, "map");
    if (map == NULL) {
        // map object doesn't exist
        return 1;
    }

	cJSON * map_width = cJSON_GetObjectItem(map, "map_width");
    if (map_width == NULL) {
        // map_width object doesn't exist
        return 1;
    }
	Model.map_width = map_width->valueint;	

	cJSON * map_height = cJSON_GetObjectItem(map, "map_height");
    if (map_height == NULL) {
        // map_height object doesn't exist
        return 1;
    }
	Model.map_height = map_height->valueint;

	cJSON * user = cJSON_GetObjectItem(root, "user");
    if (user == NULL) {
        // user object doesn't exist
        return 1;
    }
	Model.users = (struct user *)malloc(sizeof(struct user) * Model.max_user);
	for(int i = 0; i < Model.max_user; i++){
		memset(Model.users[i].name, 0, sizeof(NAME_SIZE));
		Model.users[i].score = 0;
		cJSON* user_array = cJSON_GetArrayItem(user,i);
	    cJSON* base = cJSON_GetObjectItem(user_array,"base"); 
		cJSON* base_x = cJSON_GetArrayItem(base, 0);
		cJSON* base_y = cJSON_GetArrayItem(base, 1);
		cJSON* user_location = cJSON_GetObjectItem(user_array,"location"); 
		cJSON* user_x = cJSON_GetArrayItem(user_location, 0);
		cJSON* user_y = cJSON_GetArrayItem(user_location, 1);
		Model.users[i].user_loc.x = user_x->valueint;
		Model.users[i].user_loc.y = user_y->valueint;
		Model.users[i].base_loc.x = base_x->valueint;
		Model.users[i].base_loc.y = base_y->valueint;
	#ifdef DEBUG
		printf("name: %s\n",Model.users[i].name);
		printf("base x: %d\n",Model.users[i].base_loc.x);
		printf("base y: %d\n",Model.users[i].base_loc.y);
	#endif
	}
	
	cJSON * item = cJSON_GetObjectItem(root, "item_location");
    if (item == NULL) {
        // item object doesn't exist
        return 1;
    }
	num_item = cJSON_GetArraySize(item);
	current_num_item = num_item;
	Model.item_locations = (struct location *)malloc(sizeof(struct location) * num_item); 
	for(int i = 0; i < num_item; i++){
		cJSON* item_array = cJSON_GetArrayItem(item,i);
		cJSON* item_x = cJSON_GetArrayItem(item_array, 0);
		cJSON* item_y = cJSON_GetArrayItem(item_array, 1);
		Model.item_locations[i].x = item_x->valueint;
		Model.item_locations[i].y = item_y->valueint;
	#ifdef DEBUG
		printf("item x: %d\n",Model.item_locations[i].x);
		printf("item y: %d\n",Model.item_locations[i].y);
		#endif
	}	

	cJSON * block = cJSON_GetObjectItem(root, "block_location");
    if (block == NULL) {
        // block object doesn't exist
        return 1;
    }
	num_block = cJSON_GetArraySize(block);
	Model.block_locations = (struct location *)malloc(sizeof(struct location) * num_block); 
	for(int i = 0; i < num_block; i++){
		cJSON* block_array = cJSON_GetArrayItem(block,i);
		cJSON* block_x = cJSON_GetArrayItem(block_array, 0);
		cJSON* block_y = cJSON_GetArrayItem(block_array, 1);
		Model.block_locations[i].x = block_x->valueint;
		Model.block_locations[i].y = block_y->valueint;
	#ifdef DEBUG
		printf("block x: %d\n",Model.block_locations[i].x);
		printf("block y: %d\n",Model.block_locations[i].y);
	#endif
	}	
		
	return 0;
}

int main(int argc, char * argv[]) {

    char filepath[128];

    strcpy(filepath, argv[1]);

    FILE *file = fopen(filepath, "r");

    struct stat st;
	if(stat(filepath, &st) == -1) {
  		fprintf(stderr,"ERROR: stat()\n");
  		return 1;
	}
	int json_size = st.st_size;

    char * jsonfile = (char*)malloc(json_size+1);
	if(jsonfile == NULL) {
		fprintf(stderr,"ERROR: memory allocation\n");
		return 1;
	}

	int read_size = fread(jsonfile, 1, json_size, file);
	if(read_size != json_size) {
		fprintf(stderr, "ERROR: read file\n");
		return 1;
	}

	fclose(file);
	jsonfile[json_size] = '\0';
	
	// cJSON * root = cJSON_Parse(jsonfile);
	// if (root == NULL) {
	// 	printf("JSON parsing failed: %s\n", cJSON_GetErrorPtr());
    //   	return 1;
	// }

    // char * json_serialize = cJSON_Print(root);

	if(parseJson(jsonfile))
		return 1;


}