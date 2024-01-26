#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "client.h"


int main(int argc, char * argv[]) {

	/* parsing json file */
    FILE *file = fopen("inputs/update_model/map/ball_success.json", "r");

    struct stat st;
	if(stat("inputs/update_model/map/ball_success.json", &st) == -1) {
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
	

	if(parseJson(jsonfile))
		return 1;
	/* parsing json file */


	/* create map */
	map = (int **) malloc (sizeof(int *) * Model.map_width);
	for(int i = 0;i < Model.map_width; i++){
		map[i] =(int *) malloc(sizeof(int) * Model.map_height);
	} 
	/* create map */


	/* update_cell */
	update_cell();
	/* update_cell */

	// regard there is no crash so far

	/* check validation */
	int i, cmd;
    int movement;
	for (i = 0; i < 20; i++) {
		scanf("%d", &cmd);
		if (Model.max_user == 3) {
			if (!(0 <= cmd && cmd <= 11)) {
				continue;
			}

		} else if (Model.max_user == 4) {
			if (!(0 <= cmd && cmd <= 15)) {
				continue;
			}

		}
		else if (Model.max_user == 5) {
			if (!(0 <= cmd && cmd <= 19)) {
				continue;
			}
		}
		
        /* check model_update */    
        int user_idx, span;
        int curr_x, curr_y, target_x, target_y, item_target_x, item_target_y;

        if((movement = check_validation(cmd)) == 0) {
            fprintf(stderr,"invalid movement!\n");
        }
        else{	
            user_idx = cmd/4;
            span = cmd%4;	
            curr_x = target_x = item_target_x = Model.users[user_idx].user_loc.x;
            curr_y = target_y = item_target_y = Model.users[user_idx].user_loc.y;
            switch(span){
                case UP:		
                    target_y = curr_y - 1;
                    item_target_y = target_y - 1;
                    break;	
                case DOWN:
                    target_y = curr_y + 1;
                    item_target_y = target_y + 1;
                    break;	
                case LEFT:
                    target_x = curr_x - 1;
                    item_target_x = target_x - 1;
                    break;	
                case RIGHT:
                    target_x = curr_x + 1;
                    item_target_x = target_x + 1;
                    break;	
            }
        } 

        if(movement < ITEM){ //valid and item-empty
            fprintf(stderr,"move for item %d!!!\n", movement);	
            update_model(movement, item_target_x, item_target_y);	
        }else if(movement > (0-ITEM)){ //valid and item-scoreup
            fprintf(stderr,"move for success %d!!!\n", movement);	
            update_model((0-movement), -1, -1);	
            score_up(user_idx);
            current_num_item--;
        }
        update_model(user_idx+1, target_x,target_y);	
        fprintf(stderr,"move finish!\n");
        /* check model_update */
	}
	/* check validation */

}