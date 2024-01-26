#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

int parseJson(char * jsonfile);


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
	

	if(parseJson(jsonfile))
		return 1;


}