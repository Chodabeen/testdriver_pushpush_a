#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "client.h"


int main(int argc, char * argv[]) {

	/* parsing json file */
    FILE *file = fopen("inputs/check/map/ball_success.json", "r");

    struct stat st;
	if(stat("inputs/check/map/ball_success.json", &st) == -1) {
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
		
        /* check move and model_update */    
        if((movement = check_validation(cmd)) == 0) {
            fprintf(stderr,"invalid movement!\n");
        }
        else{	
            move(cmd, movement);
        } 
        /* check move and model_update */
	}
	/* check validation */

}